#include "Audio.h"
#include "Window.h"

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID, LPDIRECTSOUND*, LPUNKNOWN)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

#undef max
#undef min

Audio::Audio(uint32_t samplesPerSecond, uint32_t bufferSize) : samplesPerSecound(samplesPerSecond), bufferSize(bufferSize)
{
}

bool Audio::startSnd(HWND windowHandle)
{
	HMODULE dSoundLibary = LoadLibraryA("dsound.dll");

	if (dSoundLibary)
	{
		direct_sound_create* directSoundCreate = (direct_sound_create*)GetProcAddress(dSoundLibary, "DirectSoundCreate");

		if (directSoundCreate)
		{
			LPDIRECTSOUND directSound;
			if (SUCCEEDED(directSoundCreate(0, &directSound, 0)))
			{
				WAVEFORMATEX waveFormat = {};
				waveFormat.wFormatTag = WAVE_FORMAT_PCM;
				waveFormat.nChannels = 2;
				waveFormat.nSamplesPerSec = samplesPerSecound;
				waveFormat.wBitsPerSample = 16;
				waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
				waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

				if (SUCCEEDED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY)))
				{
					DSBUFFERDESC bufferDescription = {};
					bufferDescription.dwSize = sizeof(bufferDescription);
					bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

					LPDIRECTSOUNDBUFFER primaryBuffer;
					if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
					{
						if (!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
							utils::logBreak("unsucceeded call to setFormat");
					}
					else
						utils::logBreak("unsucceeded call to createSoundBuffer for primaryBuffer!");
				}
				else
					utils::logBreak("unsucceeded call to setCooperativeLevel!");

				DSBUFFERDESC bufferDescription = {};
				bufferDescription.dwSize = sizeof(bufferDescription);
				bufferDescription.dwBufferBytes = bufferSize;
				bufferDescription.lpwfxFormat = &waveFormat;

				if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &secondaryBuffer, 0)))
				{
					secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
				}
				else
					utils::logBreak("unsucceeded call to createSoundBuffer for secondaryBuffer!");
			}
			else
				utils::logBreak("unsucceeded call to DirectSoundCreate!");
		}
		else
			utils::logBreak("DirectSoundCreate could not be called!");
	}
	else
		utils::logBreak("dsound.dll not found!");

    return true;
}

void Audio::stopSnd()
{
}

void Audio::startStream()
{
}

void Audio::stopStream()
{
}

void Audio::start(Sound* sound)
{
    assert(sounds.size() < sounds.capacity());
	sound->playIndex = 0;
    sounds.push_back(sound);
    sound->audioIndex = sounds.size() - 1;
}

void Audio::stop(Sound* sound)
{
    assert(sound->audioIndex != -1 && (uint32_t)sound->audioIndex < sounds.size());
    sounds.erasePop_back(sound->audioIndex);

	uint32_t i = sound->audioIndex;
	for (auto it = sounds.begin() + i; it != sounds.end(); ++it, ++i)
	{
		(*it)->audioIndex = i;
	}

    sound->audioIndex = -1;
}

void Audio::clear()
{
	for (auto it = sounds.begin(); it != sounds.end(); ++it)
	{
		(*it)->audioIndex = -1;
	}
    sounds.clear();
}

void Audio::update()
{
	DWORD writeCursor, playCursor;

	if (SUCCEEDED(secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
	{
		VOID* region0;
		DWORD region0Size;
		VOID* region1;
		DWORD region1Size;

		DWORD byteToLock = sampleIndex * bytesPerSample % bufferSize;
		DWORD bytesToWrite = 0;
		if (byteToLock > playCursor)
		{
			bytesToWrite = bufferSize - byteToLock + playCursor;
		}
		else
		{
			bytesToWrite = playCursor - byteToLock;
		}

		if (bytesToWrite == 0)
		{
			return;
		}

		HRESULT lockResult = secondaryBuffer->Lock(byteToLock, bytesToWrite, &region0, &region0Size, &region1, &region1Size, 0);
		if (SUCCEEDED(lockResult))
		{
			int16_t* sampleOutput = (int16_t*)region0;
			int region0SampleCount = region0Size / bytesPerSample;
			for (int i = 0; i < region0SampleCount; ++i, ++sampleIndex)
			{
				int16_t sampleValue = 0;
				for (auto it = sounds.begin(); it != sounds.end(); ++it)
				{
					Sound* a = *it;
					// if (a->playDur != Audio::PlayDur::LOOPED)
					if (a->playIndex > (a->nSamples / a->nChannels))
					{
						if (i == 0)
						{
							deleteList.push_back(it);
						}
						continue;
					}

					sampleValue += (int16_t)((float)a->samples[0][a->playIndex++ % a->nSamples]);
				}
				if (sampleValue > std::numeric_limits<int16_t>::max())
					sampleValue = std::numeric_limits<int16_t>::max();
				else if (sampleValue < std::numeric_limits<int16_t>::min())
					sampleValue = std::numeric_limits<int16_t>::min();

				*sampleOutput++ = sampleValue;
				*sampleOutput++ = sampleValue;
			}

			sampleOutput = (int16_t*)region1;
			int region1SampleCount = region1Size / bytesPerSample;
			for (int i = 0; i < region1SampleCount; ++i, ++sampleIndex)
			{
				int16_t sampleValue = 0;
				for (auto it = sounds.begin(); it != sounds.end(); ++it)
				{
					Sound* a = *it;

					// if (a->playDur != Audio::PlayDur::LOOPED)
					if (a->playIndex > (a->nSamples / a->nChannels))
					{
						continue;
					}

					sampleValue += a->samples[0][a->playIndex++ % a->nSamples];
				}
				if (sampleValue > std::numeric_limits<int16_t>::max())
					sampleValue = std::numeric_limits<int16_t>::max();
				else if (sampleValue < std::numeric_limits<int16_t>::min())
					sampleValue = std::numeric_limits<int16_t>::min();

				*sampleOutput++ = sampleValue;
				*sampleOutput++ = sampleValue;
			}

			if (!SUCCEEDED(secondaryBuffer->Unlock(region0, region0Size, region1, region1Size)))
				utils::logBreak("unlock failed!");
		}
		else
		{
			utils::logBreak("lock failed!");
		}
	}
	else
		utils::logBreak("getCurrentPos failed!");

	if (!deleteList.empty())
	{
		for (auto it = deleteList.begin(); it != deleteList.end(); ++it)
		{
			(*(*it))->audioIndex = -1;
			sounds.erase(*it);
		}

		deleteList.clear();

		uint32_t i = 0;
		for (auto it = sounds.begin(); it != sounds.end(); ++it, ++i)
		{
			(*it)->audioIndex = i;
		}
	}
}
