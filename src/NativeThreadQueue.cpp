#include "NativeThreadQueue.h"
#include "Utils.h"
#include <intrin.h>

const int32_t NativeThreadQueue::nThreads = 4;

NativeThreadQueue::NativeThreadQueue() : entries(SIZE), nativeThreads(nThreads)
{
}

void NativeThreadQueue::startThreads()
{
	semaphore = CreateSemaphoreExA(0, 0, nThreads, 0, 0, SEMAPHORE_ALL_ACCESS);
    mainWaitToFinsihSemaphore = CreateSemaphoreExA(0, 0, nThreads, 0, 0, SEMAPHORE_ALL_ACCESS);

    utils::logF("nThreads: %i", nThreads);
    for(uint32_t i = 0; i < nThreads; ++i)
    {
        nativeThreads.emplace_back(GET_DELEGATE_WITH_PARAM_FORM(void(volatile bool&, void*),
            NativeThreadQueue, &NativeThreadQueue::threadFunc), nullptr);
    }

    for (uint32_t i = 0; i < nThreads; ++i)
    {
        WaitForSingleObjectEx(mainWaitToFinsihSemaphore, INFINITE, FALSE);
    }
}

void NativeThreadQueue::endThreads()
{
    nativeThreads.clear();

	ReleaseSemaphore(semaphore, nThreads, 0);
    for (uint32_t i = 0; i < nThreads; ++i)
    {
        WaitForSingleObjectEx(mainWaitToFinsihSemaphore, INFINITE, FALSE);
    }
    CloseHandle(semaphore);
    semaphore = nullptr;
    CloseHandle(mainWaitToFinsihSemaphore);
    mainWaitToFinsihSemaphore = nullptr;
}

void NativeThreadQueue::threadFunc(volatile bool& running, void* arg)
{
    while(running)
    {
        uint32_t originalNextEntryIndex = nextEntryIndex;
        if(originalNextEntryIndex < size)
        {
            uint32_t readData = InterlockedCompareExchange(&nextEntryIndex, originalNextEntryIndex + 1, originalNextEntryIndex);
            if(readData == originalNextEntryIndex)
            {
                Entry entry = entries[originalNextEntryIndex];
                entry.delegate(entry.data, flushArg);
            }
        }
        else
        {
            ReleaseSemaphore(mainWaitToFinsihSemaphore, 1, 0);
			WaitForSingleObjectEx(semaphore, INFINITE, FALSE);
        }
    }

    ReleaseSemaphore(mainWaitToFinsihSemaphore, 1, 0);
}

void NativeThreadQueue::addEntry(Delegate<void(uint32_t, float)>&& delegate, uint32_t param)
{
    assert(entryCount < SIZE);

    entries[entryCount].delegate = delegate;
    entries[entryCount].data = param;

	_WriteBarrier();
	_mm_sfence();

    ++entryCount;
}

void NativeThreadQueue::addMainThreadEntry(Delegate<void(uint32_t, float)>&& delegate, uint32_t param)
{
    mainThreadEntries.push_back(MainThreadEntry{ Entry{param, delegate}, entryCount });
}

void NativeThreadQueue::flushToWithAndReset(uint32_t index, float arg)
{
#ifdef DEBUG
    if(!startedFlushing)
        startedFlushing = true;
#endif

    assert(index <= entryCount);
    flushArg = arg;

	_WriteBarrier();
	_mm_sfence();

    nextEntryIndex = 0;
    nextMainThreadEntryIndex = 0;
    size = index;

	_WriteBarrier();
	_mm_sfence();

    completeTheEntries();
}

void NativeThreadQueue::flushToWith(uint32_t index, float arg)
{
#ifdef DEBUG
    if(!startedFlushing)
        startedFlushing = true;
#endif

    assert(index <= entryCount);
    flushArg = arg;

	_WriteBarrier();
	_mm_sfence();

    size = index;

	_WriteBarrier();
	_mm_sfence();

    completeTheEntries();
}

void NativeThreadQueue::flush(float arg)
{
#ifdef DEBUG
    if(!startedFlushing)
        startedFlushing = true;
#endif

    flushArg = arg;

	_WriteBarrier();
	_mm_sfence();

    size = entryCount;

	_WriteBarrier();
	_mm_sfence();

    completeTheEntries();
}

void NativeThreadQueue::flushFrom(uint32_t index, float arg)
{
#ifdef DEBUG
    if(!startedFlushing)
        startedFlushing = true;
#endif

    assert(index <= entryCount);
    flushArg = arg;

	_WriteBarrier();
	_mm_sfence();

    nextEntryIndex = index;
    size = entryCount;

	_WriteBarrier();
	_mm_sfence();

    completeTheEntries();
}

void NativeThreadQueue::setNextWriteIndex(uint32_t newWriteIndex)
{
    assert((size < newWriteIndex) || (size == nextEntryIndex));

    entryCount = newWriteIndex;
}

#ifdef DEBUG
bool NativeThreadQueue::getStartedFlushing() const
{
    return startedFlushing;
}

void NativeThreadQueue::resetStartedFlushing()
{
    startedFlushing = false;
}
#endif

bool NativeThreadQueue::getAndCompleteNextEntry()
{
    bool existsMoreWork = false;

    uint32_t originalNextEntryIndex = nextEntryIndex;
    if(originalNextEntryIndex < size)
    {
        uint32_t readData = InterlockedCompareExchange(&nextEntryIndex, originalNextEntryIndex + 1, originalNextEntryIndex);
        if(readData == originalNextEntryIndex)
        {
            Entry entry = entries[originalNextEntryIndex];
            entry.delegate(entry.data, flushArg);
        }
        existsMoreWork = true;
    }

    return existsMoreWork;
}

void NativeThreadQueue::completeTheEntries()
{
    uint32_t nEntriesToDo = size - nextEntryIndex;
    nEntriesToDo = (nEntriesToDo > nThreads) ? nThreads : nEntriesToDo;
    ReleaseSemaphore(semaphore, nEntriesToDo, 0);
    processMainThreadEntries();
    while (getAndCompleteNextEntry());
    for (uint32_t i = 0; i < nEntriesToDo; ++i)
    {
        WaitForSingleObjectEx(mainWaitToFinsihSemaphore, INFINITE, FALSE);
    }
}

void NativeThreadQueue::processMainThreadEntries()
{
    // TODO: This falls apart if for a given "pipeline", so there are only mainThreadEntries added...
    for (; nextMainThreadEntryIndex < mainThreadEntries.size(); ++nextMainThreadEntryIndex)
    {
        auto& entry = mainThreadEntries[nextMainThreadEntryIndex];
        if (entry.entryId < size)
        {
            entry.delegate(entry.data, flushArg);
        }
        else
        {
            break;
        }
    }
}

uint32_t NativeThreadQueue::getSize() const
{
    return entryCount;
}
