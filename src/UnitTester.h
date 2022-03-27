#pragma once

#include "Utils.h"

#define unitAssert(exp) do {if(!((exp) && true)) {utils::logF("Test \"%s\" failed in line: %i, in file: %s", #exp, __LINE__, __FILE__); return false;} } while(false);

struct UnitTester
{
	template <typename Functor>
	static void test(Functor functor)
	{
		if (!functor())
			utils::logBreak("Functor failed!!!");
	}
};