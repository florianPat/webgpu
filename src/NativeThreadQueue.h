#pragma once

#include "NativeThread.h"
#include "Vector.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

class NativeThreadQueue
{
    struct Entry
    {
        uint32_t data;
        Delegate<void(uint32_t, float)> delegate;
    };
    struct MainThreadEntry : public Entry
    {
        uint32_t entryId = 0;
    };

    static constexpr int32_t SIZE = 64;
    Vector<Entry> entries;
    volatile uint32_t entryCount = 0;
    volatile uint32_t nextEntryIndex = 0;
    volatile uint32_t size = 0;
    Vector<MainThreadEntry> mainThreadEntries;
    uint32_t nextMainThreadEntryIndex = 0;
    float flushArg = 0.0f;
    HANDLE semaphore = nullptr;
    HANDLE mainWaitToFinsihSemaphore = nullptr;
    static const int32_t nThreads;
    HeapArray<NativeThread> nativeThreads;
#ifdef DEBUG
    bool startedFlushing = false;
#endif
public:
    NativeThreadQueue();
    void startThreads();
    void endThreads();
    NativeThreadQueue(const NativeThreadQueue& other) = delete;
    NativeThreadQueue& operator=(const NativeThreadQueue& rhs) = delete;
    NativeThreadQueue(NativeThreadQueue&& other) = delete;
    NativeThreadQueue& operator=(NativeThreadQueue&& rhs) = delete;
    void addEntry(Delegate<void(uint32_t, float)>&& delegate, uint32_t param);
    void addMainThreadEntry(Delegate<void(uint32_t, float)>&& delegate, uint32_t param);
    void setNextWriteIndex(uint32_t newWriteIndex);
    void flush(float arg = 0.0f);
    void flushToWithAndReset(uint32_t index, float arg = 0.0f);
    void flushToWith(uint32_t index, float arg = 0.0f);
    void flushFrom(uint32_t index, float arg = 0.0f);
    uint32_t getSize() const;
#ifdef DEBUG
    bool getStartedFlushing() const;
    void resetStartedFlushing();
#endif
private:
    void threadFunc(volatile bool& running, void* arg);
    bool getAndCompleteNextEntry();
    void completeTheEntries();
    void processMainThreadEntries();
};
