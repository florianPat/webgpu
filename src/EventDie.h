#pragma once

#include "EventData.h"

struct EventDie : public EventData
{
	inline static int32_t eventId = -1;

	EventDie() : EventData(eventId) {}
};