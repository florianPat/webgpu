#pragma once

#include <pthread.h>
#include <unistd.h>
#include "INativeThread.h"
#include "Types.h"

class NativeThreadLinux : public INativeThread
{
    enum class ThreadAffinityMask
    {
        LITTLE = 0x0F,
        BIG = 0xF0
    };
    static constexpr int32_t PAUSED_THREAD = -1;
public:
    struct ThreadParameter
    {
        Delegate<void(volatile bool&, void*)> threadFunc;
        void* arg;
        volatile bool running = true;
    };
protected:
    ThreadParameter threadParameter;
    pthread_t thread = 0;
public:
    NativeThreadLinux(Delegate<void(volatile bool&, void*)> threadFunc, void* arg);
    ~NativeThreadLinux();

    void start();
    void stop();
private:
    void setThreadAffinityMask(int32_t tid, ThreadAffinityMask mask);
};
