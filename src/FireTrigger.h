#pragma once

#include "Vector.h"

struct FireTrigger
{
	int32_t healthEventIndex = -1;
	int32_t dieEventIndex = -1;
	int32_t bodyIndex = -1;
};

typedef Vector<FireTrigger> FireTriggers;