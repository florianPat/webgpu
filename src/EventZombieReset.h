#pragma once

#include "EventData.h"

struct EventZombieReset : public EventData
{
	inline static int32_t eventId = -1;
	bool resetWaves = true;

	EventZombieReset(bool resetWavesIn) : EventData(eventId), resetWaves(resetWavesIn) {}
};