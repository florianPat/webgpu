#pragma once

#include "EventData.h"

struct EventResumeApp : public EventData
{
    inline static int32_t eventId = -1;
    EventResumeApp() : EventData(eventId) {}
};