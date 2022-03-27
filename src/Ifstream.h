#pragma once

#include "String.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include "UniquePtr.h"

class Ifstream final
{
public:
	enum class SeekDir {beg, cur, end};
private:
	HANDLE asset = nullptr;
	bool good = false;
	UniquePtr<char[]> fileData = nullptr;
	uint64_t pointerPos = 0;
public:
	Ifstream(const String& filename);
	Ifstream() = default;
	Ifstream(const Ifstream& other) = delete;
	Ifstream(Ifstream&& other) noexcept;
	Ifstream& operator=(const Ifstream& rhs) = delete;
	Ifstream& operator=(Ifstream&& rhs) noexcept;
	~Ifstream();
	explicit operator bool() const;
	bool operator!() const;
	bool eof();
	void getline(String& line);
	void readTempLine();
	void read(void* s, uint32_t n);
	char get();
	uint64_t tellg();
	void seekg(uint32_t pos);
	void seekg(uint32_t off, SeekDir way);
	uint64_t getSize();
	void open(const String& filename);
	const void* getFullData();
	void close();
};