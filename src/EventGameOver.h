#pragma once

#include "EventData.h"

struct EventGameOver : public EventData
{
	inline static int32_t eventId = -1;

	EventGameOver() : EventData(eventId) {}
};