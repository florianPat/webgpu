#pragma once

#include "EventData.h"
#include "Vector.h"
#include "Delegate.h"
#include "Globals.h"
#include "VariableVector.h"
#include <intrin.h>
#include "NativeThreadQueue.h"

class EventManager
{
	static constexpr uint32_t DATA_HOLDER_SIZE = 265;
    struct EventListenerMapEntry
    {
    	int32_t eventType;
		Delegate<void(EventData*)> function;
    };
    struct EventTypeAndHolderVector
	{
    	int32_t& eventType;
		VariableVector<EventData> eventDataHolder;
	};
public:
	struct DelegateFunctionRef
	{
		const uint32_t threadTypeId;
		const uint32_t delegateId;
	};
private:
	Vector<Vector<EventListenerMapEntry>> eventListenerMap;
	Vector<EventTypeAndHolderVector> eventTypeVector;
	Vector<DelegateFunctionRef> eventDeleterMap;
	NativeThreadQueue& nativeThreadQueue;
	volatile uint32_t mutex = 0;
	uint32_t eventListenerMapInsertIndex = 0;
#ifdef DEBUG
	// NOTE: Thats not the 100% correct, but hopefully its good enough...
	volatile bool startedWorkingOnTheVectors = false;
#endif
private:
	void triggerEventDelegate(uint32_t specificArg, float broadArg);
	void clearTriggerEventsDelegate(uint32_t specificArg, float broadArg);
	void addListenerEventsDelegate(uint32_t specificArg, float broadArg);
public:
	EventManager();
	~EventManager();
	DelegateFunctionRef addListener(int32_t& eventType, Delegate<void(EventData*)>&& function);
	DelegateFunctionRef addListenerForSameThreadType(int32_t& eventType, Delegate<void(EventData*)>&& function, uint32_t threadTypeId);
	template <typename T, typename... Args>
	void TriggerEvent(Args&&... args);
	void addClearTriggerAndAddListenerEventsJob();
};

template <typename T, typename... Args>
inline void EventManager::TriggerEvent(Args&&... args)
{
	if(T::eventId != -1)
	{
#ifdef DEBUG
		assert(!startedWorkingOnTheVectors);
#endif
		assert((uint32_t)T::eventId < eventTypeVector.size());
		VariableVector<EventData>& eventDataHolder = eventTypeVector[T::eventId].eventDataHolder;

		uint32_t readValue = 1;
		do {
			readValue = InterlockedCompareExchange(&mutex, 1, 0);
		} while(readValue == 0);

		eventDataHolder.push_back<T>(std::forward<Args>(args)...);

		_WriteBarrier();
		_mm_sfence();

        mutex = 0;
	}
	else
	{
		utils::log("Event is not registered with any functions yet!");
	}
}