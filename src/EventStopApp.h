#pragma once

#include "EventData.h"

struct EventStopApp : public EventData
{
    inline static int32_t eventId = -1;
    EventStopApp() : EventData(eventId) {}
};