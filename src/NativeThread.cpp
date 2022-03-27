#include "NativeThread.h"
#include "Utils.h"

NativeThread::NativeThread(Delegate<void(volatile bool&, void*)> threadFunc, void* arg) : threadParameter{ threadFunc, arg }
{
    start();
}

NativeThread::~NativeThread()
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

void NativeThread::start()
{
    utils::log("start thread");

    assert(thread == 0);
	
	DWORD threadId;
	thread = CreateThread(0, 0, startRoutine, this, 0, &threadId);
}

void NativeThread::stop()
{
    assert(thread != 0);

    threadParameter.running = false;
    //pthread_detach(thread);
    thread = 0;
}
