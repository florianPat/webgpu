#include "NativeThreadLinux.h"
#include "Utils.h"
#include <syscall.h>

NativeThreadLinux::NativeThreadLinux(Delegate<void(volatile bool&, void*)> threadFunc, void* arg) : INativeThread(threadFunc, arg),
    threadParameter{threadFunc, arg}
{
    start();
}

NativeThreadLinux::~NativeThreadLinux()
{
    stop();
}

static void* startRoutine(void* args)
{
    NativeThreadLinux::ThreadParameter* threadParameter = (NativeThreadLinux::ThreadParameter*)args;

    threadParameter->threadFunc(threadParameter->running, threadParameter->arg);

    utils::log("pthread_exit");
    pthread_exit(nullptr);
}

void NativeThreadLinux::start()
{
    utils::log("start thread");

    assert(thread == 0);

    pthread_attr_t attributes;
    int32_t result = pthread_attr_init(&attributes);
    assert(result == 0);
    result = pthread_create(&thread, &attributes, startRoutine, &threadParameter);
    assert(result == 0);
    result = pthread_attr_destroy(&attributes);
    assert(result == 0);

    setThreadAffinityMask(thread, ThreadAffinityMask::LITTLE);
}

void NativeThreadLinux::stop()
{
    assert(thread != 0);

    threadParameter.running = false;
    pthread_detach(thread);
    thread = 0;
}

void NativeThreadLinux::setThreadAffinityMask(int32_t tid, NativeThreadLinux::ThreadAffinityMask mask)
{
    int32_t syscallResult = syscall(__NR_sched_setaffinity, tid, sizeof(mask), mask);
     assert(syscallResult);
}
