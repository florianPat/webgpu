#pragma once

#include "EventData.h"
#include "Utils.h"

struct EventLevelReload : public EventData
{
	inline static int32_t eventId = -1;
	EventLevelReload() : EventData(eventId) {}
};