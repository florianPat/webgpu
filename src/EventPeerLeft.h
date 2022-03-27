#pragma once

#include "EventData.h"

struct EventPeerLeft : public EventData
{
	inline static int32_t eventId = -1;

	EventPeerLeft() : EventData(eventId) {}
};