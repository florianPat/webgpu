#pragma once

#include "EventData.h"

struct EventDestroyApp : public EventData
{
    inline static int32_t eventId = -1;
    EventDestroyApp() : EventData(eventId) {}
};