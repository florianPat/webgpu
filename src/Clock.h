#pragma once

#include "Types.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

struct Time
{
	uint64_t nanosecondss;

	inline uint64_t asNanoseconds()
	{
		return nanosecondss;
	}

	inline uint64_t asMicroseconds()
	{
		return nanosecondss / 1000;
	}

	inline uint32_t asMilliseconds()
	{
		return (uint32_t)(nanosecondss / 1000000.0);
	}

	inline float asSeconds()
	{
		return (float)(nanosecondss / 1000000000.0);
	}

	static Time seconds(float seconds)
	{
		return Time { (uint64_t)(seconds * 1000000000.0) };
	}

	static Time milliseconds(int32_t milliseconds)
	{
		return Time { (uint64_t)(milliseconds * 1000000.0) };
	}

	static Time microseconds(uint64_t microseconds)
	{
		return Time{ microseconds * 1000 };
	}

	static Time nanoseconds(uint64_t nanoseconds)
	{
		return Time{ nanoseconds };
	}
};

class Clock
{
public:
	Clock();

	void restart();
	Time getTime();

	Time getElapsedTimeTotal();
	static uint64_t now();
private:
	void update();
private:
	uint64_t firstTime = 0;
	uint64_t lastTime = 0;
	uint64_t elapsed = 0;
	uint64_t elapsedTotal = 0;
	static LARGE_INTEGER performanceCounterFrequency;
};