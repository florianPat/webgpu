#pragma once

#include "String.h"

struct AssetLoader
{
	void(*reloadFromFile)(int8_t* thiz, const String& filename);
	uint64_t(*getSize)(int8_t* thiz);
	void(*destruct)(int8_t* thiz);
	bool isGpu;
public:
	template <typename T, bool isGpuIn>
	static constexpr AssetLoader initLoader()
	{
		AssetLoader result;

		if constexpr (isGpuIn)
		    result.reloadFromFile = [](int8_t* thiz, const String& filename) { ((T*)thiz)->reloadFromFile(filename); };

		result.getSize = [](int8_t* thiz) {return ((T*)thiz)->getSize(); };
		result.destruct = [](int8_t* thiz) {((T*)thiz)->~T(); };
		result.isGpu = isGpuIn;
		return result;
	}
};