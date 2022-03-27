#include "Sound.h"
#include "Ifstream.h"
#include "Utils.h"
#include "Globals.h"
#include "UniquePtr.h"
#include "Window.h"

uint64_t Sound::getSize() const
{
	return (nSamples * sizeof(int16_t) + sizeof(Sound));
}

const Vector<int16_t>* Sound::getBuffer() const
{
	return samples.data();
}

Sound::operator bool() const
{
	return (nSamples != 0);
}

Sound::Sound(const String& filename)
{
	Ifstream file;
	file.open(filename);

	if (!file)
	{
		utils::logBreak("Could not open file!");
	}

	uint32_t fileSize = (uint32_t)file.getSize();

	UniquePtr<int8_t[]> fileContents = makeUnique<int8_t[]>(fileSize);
	file.read(fileContents.get(), fileSize);

	FileHeader* fileHeader = (FileHeader*)fileContents.get();

	assert(fileHeader->riffId == (uint32_t)ChunkId::RIFF);
	assert(fileHeader->waveId == (uint32_t)ChunkId::WAVE);

	int16_t* sampleData = nullptr;
	uint32_t sampleDataSize = 0;

	for (RiffIt it = RiffIt(fileHeader + 1, (fileHeader + 1) + fileHeader->size - 4); it && (!sampleData || !nChannels); ++it)
	{
		switch (it.getType())
		{
			case (uint32_t)ChunkId::FMT:
			{
				Fmt* fmt = (Fmt*)it.getChunkData();
				nChannels = fmt->nChannles;

				assert(fmt->wFormatTag == 1); //NOTE: Only PCM music!
				assert(fmt->nSamplesPerSecond == SAMPLE_RATE);
				assert(fmt->wBitsPerSample == 16);
				assert(fmt->nBlockAlign == (sizeof(int16_t)*fmt->nChannles));
				break;
			}
			case (uint32_t)ChunkId::DATA:
			{
				sampleData = (int16_t*)it.getChunkData();
				sampleDataSize = it.getChunkDataSize();
				break;
			}
		}
	}

	assert(nChannels && sampleData && sampleDataSize);

	nSamples = sampleDataSize / (sizeof(int16_t));
	samples.reserve((uint32_t)nSamples);

	assert(nChannels == 1 || nChannels == 2);

	if (nChannels == 1)
	{
		samples.push_back(Vector<int16_t>(nSamples));

		for (int i = 0; i < nSamples; ++i)
		{
			samples[0].push_back(sampleData[i]);
		}
	}
	else if (nChannels == 2)
	{
		samples.push_back(Vector<int16_t>(nSamples / 2));
		samples.push_back(Vector<int16_t>(nSamples / 2));

		for (int i = 0; i < nSamples;)
		{
			samples[0].push_back(sampleData[i++]);
			samples[1].push_back(sampleData[i++]);
		}
	}
	else
		utils::logBreak("Invalid channel count!");
}

Sound::~Sound()
{
	if(audioIndex != -1)
	{
		Globals::window->getAudio().stop(this);
	}
}

void Sound::resume()
{
	resumed = true;
}

void Sound::pause()
{
	resumed = false;
}

bool Sound::isFinished() const
{
	return (audioIndex == -1);
}

Sound::RiffIt::RiffIt(void * at, void* stop) : at(reinterpret_cast<uint8_t*>(at)), stop(reinterpret_cast<uint8_t*>(stop))
{
}

Sound::RiffIt::operator bool() const
{
	return this->at < this->stop;
}

Sound::RiffIt& Sound::RiffIt::operator++()
{
	Chunk* chunk = (Chunk*)at;
	uint32_t size = chunk->size;
	if (size % 2 != 0)
		++size;
	at += sizeof(Chunk) + size;
	return *this;
}

uint32_t Sound::RiffIt::getChunkDataSize() const
{
	Chunk* chunk = (Chunk*)at;
	return chunk->size;
}

void * Sound::RiffIt::getChunkData() const
{
	return at + sizeof(Chunk);
}

uint32_t Sound::RiffIt::getType() const
{
	Chunk* chunk = (Chunk*)at;
	return chunk->id;
}
