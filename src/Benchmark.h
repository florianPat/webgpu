#pragma once

#include "Clock.h"
#include "String.h"

class Benchmark
{
	bool running = false;
	Clock clock;
	ShortString benchmarkName;

	Benchmark() = default;
public:
	void start(const ShortString& benchmarkNameIn);
	void stop();

	static Benchmark getBenchmark();
};