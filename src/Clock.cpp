#include "Clock.h"
#include "stdlib.h"
#include "Utils.h"

LARGE_INTEGER Clock::performanceCounterFrequency = LARGE_INTEGER{ 0 };

Clock::Clock()
{
	if (performanceCounterFrequency.QuadPart == 0)
	{
		QueryPerformanceFrequency(&performanceCounterFrequency);
	}

	restart();
}

void Clock::restart()
{
	elapsed = 0;
	firstTime = now();
	lastTime = firstTime;
}

Time Clock::getTime()
{
	update();

	return Time{ elapsed };
}

Time Clock::getElapsedTimeTotal()
{
	update();

	return Time{ elapsedTotal };
}

uint64_t Clock::now()
{
	LARGE_INTEGER timeVal;
	QueryPerformanceCounter(&timeVal);

	timeVal.QuadPart *= 1000000000ll;
	return timeVal.QuadPart / performanceCounterFrequency.QuadPart;
}

void Clock::update()
{
	int64_t currentTime = now();
	elapsed = currentTime - lastTime;
	elapsedTotal = currentTime - firstTime;
	lastTime = currentTime;
}
