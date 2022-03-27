#pragma once

#include "Types.h"

class EventData
{
private:
	int32_t& eventId;
public:
	EventData(int32_t& eventId) : eventId(eventId) {};
	int32_t getEventId() const { return eventId; };
};