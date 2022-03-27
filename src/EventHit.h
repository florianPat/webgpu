#pragma once

#include "EventData.h"

struct EventHit : public EventData
{
	inline static int32_t eventId = -1;

	EventHit() : EventData(eventId) {}
};