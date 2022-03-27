#include "Ifstream.h"
#include "Utils.h"
#include "AssetManager.h"

Ifstream::Ifstream(const String & filename)
{
	open(filename);
}

Ifstream::Ifstream(Ifstream && other) noexcept : asset(std::exchange(other.asset, nullptr)), good(std::exchange(other.good, false)),
										fileData(std::exchange(other.fileData, nullptr)), pointerPos(std::exchange(other.pointerPos, 0))
{
}

Ifstream & Ifstream::operator=(Ifstream && rhs) noexcept
{
	close();

	asset = std::exchange(rhs.asset, nullptr);
	good = std::exchange(rhs.good, false);
	fileData = std::exchange(rhs.fileData, nullptr);
	pointerPos = std::exchange(rhs.pointerPos, 0);

	return *this;
}

Ifstream::~Ifstream()
{
	close();
}

Ifstream::operator bool() const
{
	return good;
}

bool Ifstream::operator!() const
{
	return !(operator bool());
}

bool Ifstream::eof()
{
	return (pointerPos == getSize());
}

void Ifstream::getline(String & line)
{
	line.clear();
	char c;
	while (!eof())
	{
		c = get();

		if (c == '\n')
			break;

		line += c;
	}
}

void Ifstream::readTempLine()
{
	for (char c = get(); (!eof()) && (c != '\n'); c = get());
}

uint64_t Ifstream::getSize()
{
	LARGE_INTEGER fileSize;
	BOOL result = GetFileSizeEx(asset, &fileSize);
	assert(result != 0);

	return fileSize.QuadPart;
}

void Ifstream::open(const String & filename)
{
	asset = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

	if (asset != INVALID_HANDLE_VALUE)
	{
		good = true;
	}
}

const void* Ifstream::getFullData()
{
	uint64_t fileSize = getSize();
	//NOTE: Dangerous!
	fileData = makeUnique<char[]>((uint32_t)fileSize);

	DWORD bytesRead = 0;

	seekg(0);
	BOOL result = ReadFile(asset, fileData.get(), (DWORD)fileSize, &bytesRead, nullptr);
		
	assert(result != 0);
	assert(bytesRead == fileSize);

	return fileData.get();
}

void Ifstream::close()
{
	if (asset != nullptr)
	{
		CloseHandle(asset);
		asset = nullptr;
		good = false;
		pointerPos = 0;
		fileData.~UniquePtr();
		fileData = nullptr;
	}
}

void Ifstream::read(void * s, uint32_t n)
{
	DWORD bytesRead = 0;

	BOOL result = ReadFile(asset, s, n, &bytesRead, nullptr);

	assert(result != 0);
	assert(bytesRead == n);

	pointerPos += n;
}

char Ifstream::get()
{
	char c;
	read(&c, 1);
	return c;
}

uint64_t Ifstream::tellg()
{
	return pointerPos;
}

void Ifstream::seekg(uint32_t pos)
{
	LARGE_INTEGER lPos = {};
	lPos.QuadPart = pos;
	BOOL result = SetFilePointerEx(asset, lPos, nullptr, FILE_BEGIN);

	pointerPos = pos;

	assert(result != 0);
}

void Ifstream::seekg(uint32_t off, SeekDir way)
{
	switch (way)
	{
		case SeekDir::beg:
		{
			LARGE_INTEGER lPos = {};
			lPos.QuadPart = off;
			BOOL result = SetFilePointerEx(asset, lPos, nullptr, FILE_BEGIN);

			pointerPos = off;

			assert(result != 0);
			break;
		}
		case SeekDir::cur:
		{
			LARGE_INTEGER lPos = {};
			lPos.QuadPart = off;
			BOOL result = SetFilePointerEx(asset, lPos, nullptr, FILE_CURRENT);

			pointerPos += off;

			assert(result != 0);
			break;
		}
		case SeekDir::end:
		{
			LARGE_INTEGER lPos = {};
			lPos.QuadPart = off;
			BOOL result = SetFilePointerEx(asset, lPos, nullptr, FILE_END);

			pointerPos = getSize() - off;

			assert(result != 0);
			break;
		}
		default:
		{
			InvalidCodePath;
			break;
		}
	}
}
