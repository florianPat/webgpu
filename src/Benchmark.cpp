#include "Benchmark.h"
#include "Utils.h"

void Benchmark::start(const ShortString & benchmarkNameIn)
{
	assert(!running);
	running = true;
	benchmarkName = benchmarkNameIn;
	clock.restart();
}

void Benchmark::stop()
{
	float time = clock.getTime().asSeconds();
	assert(running);
	running = false;
	utils::logF("%s: has taken: %f", benchmarkName.c_str(), time);
}

Benchmark Benchmark::getBenchmark()
{
	return Benchmark();
}
