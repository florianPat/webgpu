#include "GameObjectManager.h"
#include "Utils.h"

GameObjectManager::GameObjectManager(uint32_t renderActorSize) : actors(), pipelineIndexes(),
	nativeThreadQueue(Globals::window->getNativeThreadQueue())
{
    //NOTE: This is the RenderOnlyActor
	actors.push_back(Actor(actors.size(), renderActorSize));
}

Actor* GameObjectManager::addActor(uint32_t componentsSize)
{
	assert(!nativeThreadQueue.getStartedFlushing());

	actors.push_back(Actor(actors.size(), componentsSize));
	nativeThreadQueue.addEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float),
	        GameObjectManager, &GameObjectManager::actorUpdateDelegate), actors.size() - 1);

	return &actors.back();
}

Actor* GameObjectManager::addMainThreadActor(uint32_t componentsSize)
{
	assert(!nativeThreadQueue.getStartedFlushing());
	// NOTE: Important because if they get added last, the system will break apart (so just add them first;))
	assert(pipelineIndexes.empty() || (pipelineIndexes.back() == actors.size() + 2));

	mainThreadActors.push_back(Actor(mainThreadActors.size(), componentsSize));
	nativeThreadQueue.addMainThreadEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float),
		GameObjectManager, &GameObjectManager::mainThreadActorUpdateDelegate), mainThreadActors.size() - 1);

	return &mainThreadActors.back();
}

void GameObjectManager::endPipeline()
{
	assert(!nativeThreadQueue.getStartedFlushing());
	assert(pipelineIndexes.empty() || (pipelineIndexes.back() != actors.size() + 2));

	// NOTE: This should be just the size but the first one is reserved for clearing the triggerEventVariableVector 
	// and second one is reserved for adding more eventListerners (see setup of Level)
	// TODO: Think about moving this into the NativeThreadQueue!
	pipelineIndexes.push_back(actors.size() + 2);
}

void GameObjectManager::destroyActor(uint32_t actorId)
{
	//NOTE: Needs to be implemented in multithreading
	InvalidCodePath;
	//destroyActorQueue.push_back(actorId);
}

void GameObjectManager::updateAndDrawActors(float dt)
{
	// NOTE: This is for EventManager::clearTriggerEventsDelegate and EventManager::addListenerEventsDelegate so that it is safely executed before the
	// components
	// TODO: Do a pre-pipeline-phase work queue thing to get rid of this magic number
	nativeThreadQueue.flushToWithAndReset(2, 0.0f);
	for(auto it = pipelineIndexes.begin(); it != pipelineIndexes.end(); ++it)
	{
		nativeThreadQueue.flushToWith(*it, dt);
	}
	nativeThreadQueue.flush();

	for(int32_t i = 0; i < arrayCount(layers); ++i)
	{
		auto& layer = layers[i];

		for(auto it = layer.begin(); it != layer.end(); ++it)
		{
			Actor& actor = actors[it->first];
			actor.getComponent<Component>(it->second)->render();
		}
	}
}

void GameObjectManager::actorUpdateDelegate(uint32_t specificArg, float broadArg)
{
	Actor* actor = &actors[specificArg];

	actor->update(broadArg);
}

void GameObjectManager::mainThreadActorUpdateDelegate(uint32_t specificArg, float broadArg)
{
	Actor* actor = &mainThreadActors[specificArg];

	actor->update(broadArg);
}
