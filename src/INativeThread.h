#pragma once

#include "Delegate.h"

struct INativeThread
{
    INativeThread(Delegate<void(volatile bool&, void*)> threadFunc, void* arg) {}
    INativeThread(const INativeThread& other) = delete;
    INativeThread& operator=(const INativeThread& rhs) = delete;
    INativeThread(INativeThread&& other) = delete;
    INativeThread& operator=(INativeThread&& rhs) = delete;
//public:
    //void start();
    //void stop();
};
