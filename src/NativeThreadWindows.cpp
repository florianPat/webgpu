#include "NativeThreadWindows.h"
#include "Utils.h"

NativeThreadWindows::NativeThreadWindows(Delegate<void(volatile bool&, void*)> threadFunc, void* arg) : INativeThread(threadFunc, arg),
    threadParameter(threadFunc, arg)
{
    start();
}

NativeThreadWindows::~NativeThreadWindows()
{
    stop();
}

static DWORD WINAPI startRoutine(LPVOID args)
{
    NativeThread::ThreadParameter* threadParameter = (NativeThread::ThreadParameter*)args;

	threadParameter->threadFunc(threadParameter->running, threadParameter->arg);

	utils::log("pthread_exit");
	return 0;
}

void NativeThreadWindows::start()
{
    utils::log("start thread");

    assert(thread == 0);
	
	DWORD threadId;
	thread = CreateThread(0, 0, startRoutine, this, 0, &threadId);
}

void NativeThreadWindows::stop()
{
    assert(thread != 0);

    threadParameter.running = false;
    //pthread_detach(thread);
    thread = 0;
}
