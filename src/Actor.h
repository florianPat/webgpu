#pragma once

#include "Component.h"
#include "Window.h"
#include "Physics.h"
#include "VariableVector.h"

class Actor
{
    //Needs to happen because it has to change the id if an Actor gets deleted!
    friend class GameObjectManager;

	uint32_t id;
	VariableVector<Component> components;
public:
	Actor(uint32_t id, uint32_t componentsSize);
	template <typename T, typename... Args>
	const T* addComponent(Args&&... args);
	uint32_t getComponentIndex(uint32_t componentId) const;

	template <typename T> T* getComponent(uint32_t componentIndex);

	uint32_t getId() const { return id; }
	void update(float dt);
};

template<typename T>
inline T* Actor::getComponent(uint32_t componentIndex)
{
	assert(componentIndex < components.getOffsetToEnd());
	Component* componentPtr = (Component*) (components.begin() + componentIndex);

	//NOTE: Only used to really verify that it is ok what I am doing. RTTI should be switched off in release mode
	//assert(typeid((*componentPtr)) == typeid(T));
	assert(dynamic_cast<T*>(componentPtr) != nullptr);

	return (T*) componentPtr;
}

template<typename T, typename... Args>
inline const T* Actor::addComponent(Args&&... args)
{
	uint32_t lastOffsetToEnd = components.getOffsetToEnd();

	components.push_back<T>(std::forward<Args>(args)...);

	return (T*)(components.begin() + lastOffsetToEnd + 4);
}