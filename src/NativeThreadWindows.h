#pragma once

#include "INativeThread.h"
#include "Types.h"
#include "WindowsWindow.h"

struct NativeThreadWindows : public INativeThread
{
    struct ThreadParameter
    {
        Delegate<void(volatile bool&, void*)> threadFunc;
        void* arg;
        volatile bool running = true;
    };
protected:
    ThreadParameter threadParameter;
    HANDLE thread = 0;
public:
    NativeThreadWindows(Delegate<void(volatile bool&, void*)> threadFunc, void* arg);
    ~NativeThreadWindows();

    void start();
    void stop();
};
