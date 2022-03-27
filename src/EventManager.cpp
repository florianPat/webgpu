#include "EventManager.h"
#include "Utils.h"
#include "Window.h"

EventManager::EventManager() : eventListenerMap(), nativeThreadQueue(Globals::window->getNativeThreadQueue())
{
}

EventManager::DelegateFunctionRef EventManager::addListener(int32_t& eventType, Delegate<void(EventData*)>&& function)
{
#ifdef DEBUG
    assert(!startedWorkingOnTheVectors);
#endif

    eventListenerMap.push_back(Vector<EventListenerMapEntry>());
    return addListenerForSameThreadType(eventType, std::move(function), eventListenerMap.size() - 1);
}

EventManager::DelegateFunctionRef EventManager::addListenerForSameThreadType(int32_t& eventType,
		Delegate<void(EventData*)>&& function, uint32_t threadTypeId)
{
#ifdef DEBUG
    assert(!startedWorkingOnTheVectors);
#endif

    if (eventType == -1)
    {
        eventType = eventTypeVector.size();
        eventTypeVector.push_back(EventTypeAndHolderVector{ eventType, VariableVector<EventData>(DATA_HOLDER_SIZE) });
    }
    Vector<EventListenerMapEntry>* eventListenerList = &eventListenerMap[threadTypeId];

    eventListenerList->push_back(EventListenerMapEntry{ eventType, function });

	return DelegateFunctionRef{ threadTypeId, eventListenerList->size() - 1 };
}

void EventManager::addClearTriggerAndAddListenerEventsJob()
{
    nativeThreadQueue.addEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float), EventManager,
            &EventManager::clearTriggerEventsDelegate), 0);
    nativeThreadQueue.addEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float), EventManager,
        &EventManager::addListenerEventsDelegate), 0);
}

void EventManager::addListenerEventsDelegate(uint32_t specificArg, float broadArg)
{
#ifdef DEBUG
    startedWorkingOnTheVectors = true;
#endif

    for(; eventListenerMapInsertIndex < eventListenerMap.size(); ++eventListenerMapInsertIndex)
    {
        nativeThreadQueue.addEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float), EventManager,
            &EventManager::triggerEventDelegate), eventListenerMapInsertIndex);
    }

#ifdef DEBUG
    startedWorkingOnTheVectors = false;
#endif
}

void EventManager::triggerEventDelegate(uint32_t specificArg, float broadArg)
{
#ifdef DEBUG
    startedWorkingOnTheVectors = true;
#endif

    Vector<EventListenerMapEntry>* eventListenerList = &eventListenerMap[specificArg];

    for(auto it = eventListenerList->begin(); it != eventListenerList->end(); ++it)
    {
        VariableVector<EventData>& dataHolderOffset = eventTypeVector[it->eventType].eventDataHolder;
        for(auto offsetIt = dataHolderOffset.begin(); offsetIt != dataHolderOffset.end();)
        {
            uint32_t eventDataSize = *((uint32_t*)offsetIt);
            offsetIt += sizeof(uint32_t);

            it->function((EventData*)(offsetIt));

            offsetIt += eventDataSize;
        }
    }

#ifdef DEBUG
    startedWorkingOnTheVectors = false;
#endif
}

void EventManager::clearTriggerEventsDelegate(uint32_t specificArg, float broadArg)
{
#ifdef DEBUG
    startedWorkingOnTheVectors = true;
#endif

    for(auto it = eventTypeVector.begin(); it != eventTypeVector.end(); ++it)
    {
        it->eventDataHolder.clear();
    }

#ifdef DEBUG
    startedWorkingOnTheVectors = false;
#endif
}

EventManager::~EventManager()
{
    for(auto it = eventTypeVector.begin(); it != eventTypeVector.end(); ++it)
    {
        it->eventType = -1;
    }

    eventListenerMap.~Vector();
    eventDeleterMap.~Vector();
    eventTypeVector.~Vector();
}

//void EventManager::removeListener(const DelegateFunctionRef& delegateFunctionRef)
//{
//	eventDeleterMap.push_back(delegateFunctionRef);
//}
//
//void EventManager::removeListeners()
//{
//	if (!eventDeleterMap.empty())
//	{
//		for (auto it = eventDeleterMap.begin(); it != eventDeleterMap.end(); ++it)
//		{
//			int32_t eventType = it->eventType;
//			uint32_t delegateFunctionId = it->delegateId;
//
//			assert(eventType < eventListenerMap.size());
//			auto foundTuple = eventListenerMap.at(eventType);
//			auto findIt = foundTuple.delegateFunctions;
//			//TODO: Optimize this!
//			for (auto it = findIt.begin(); it != findIt.end(); ++it)
//			{
//				if (delegateFunctionId == it->index)
//				{
//					findIt.erasePop_back(it);
//					break;
//				}
//			}
//			if (findIt.empty())
//			{
//			    int32_t eventType = foundTuple.eventType;
//                foundTuple.eventType = -1;
//				eventListenerMap.erasePop_back(eventType);
//				eventListenerMap.at(eventType).eventType = eventType;
//			}
//		}
//		eventDeleterMap.clear();
//	}
//}
