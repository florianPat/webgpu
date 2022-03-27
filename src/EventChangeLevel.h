#pragma once

#include "EventData.h"
#include "Level.h"

struct EventChangeLevel : public EventData
{
    inline static int32_t eventId = -1;
    UniquePtr<Level> newLevel = nullptr;

    EventChangeLevel(UniquePtr<Level> newLevel) : EventData(eventId), newLevel(std::move(newLevel)) {}
};