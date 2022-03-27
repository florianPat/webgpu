#pragma once

#include "Actor.h"
#include "NativeThreadQueue.h"

class GameObjectManager
{
	static constexpr uint32_t DEFAULT_COMPONENT_SIZE = 512;
	Vector<Actor> actors;
	Vector<Actor> mainThreadActors;
	Vector<std::pair<int32_t, int32_t>> layers[4];
	Vector<uint32_t> pipelineIndexes;
	NativeThreadQueue& nativeThreadQueue;
public:
	GameObjectManager(uint32_t renderActorSize = DEFAULT_COMPONENT_SIZE);
	Actor* addActor(uint32_t componentsSize);
	Actor* addMainThreadActor(uint32_t componentsSize);
	void endPipeline();
	template <typename T, typename... Args>
	void addUpdateComponent(Actor& actor, Args&&... args);
	template <typename T, typename... Args>
	void addComponent(uint32_t renderLayer, Actor& actor, Args&&... args);
	template <typename T, typename... Args>
	void addRenderComponent(uint32_t renderLayer, Args&&... args);
	void destroyActor(uint32_t actorId);
	void updateAndDrawActors(float dt);
private:
    void actorUpdateDelegate(uint32_t specificArg, float broadArg);
	void mainThreadActorUpdateDelegate(uint32_t specificArg, float broadArg);
};

template<typename T, typename... Args>
inline void GameObjectManager::addUpdateComponent(Actor& actor, Args&&... args)
{
	assert(!nativeThreadQueue.getStartedFlushing());

	actor.addComponent<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
inline void GameObjectManager::addComponent(uint32_t renderLayer, Actor& actor, Args&&... args)
{
	// TODO: This works because every thread goes through the components sequentially.
	//assert(!nativeThreadQueue.getStartedFlushing());

	const T* component = actor.addComponent<T>(std::forward<Args>(args)...);
	
	layers[renderLayer].push_back(std::make_pair(actor.getId(), actor.getComponentIndex(component->getId())));
}

template<typename T, typename... Args>
inline void GameObjectManager::addRenderComponent(uint32_t renderLayer, Args&&... args)
{
	assert(!nativeThreadQueue.getStartedFlushing());

	Actor& actor = actors.front();
    const T* component = actor.addComponent<T>(std::forward<Args>(args)...);

	layers[renderLayer].push_back(std::make_pair(0, actor.getComponentIndex(component->getId())));
}