#include "Actor.h"
#include "GameObjectManager.h"

Actor::Actor(uint32_t id, uint32_t componentsSize)
	:	id(id), components(componentsSize)
{
}

void Actor::update(float dt)
{
	for (auto it = components.begin(); it != components.end();)
	{
		uint32_t compSize = *((uint32_t*) it);
		it += sizeof(uint32_t);

		((Component*)(it))->update(dt, this);

		it += compSize;
	}
}

uint32_t Actor::getComponentIndex(uint32_t componentId) const
{
	int32_t result = -1;

	for(auto it = components.begin(); it != components.end();)
	{
		uint32_t compSize = *((uint32_t*) it);
		it += sizeof(uint32_t);

		if(((Component*)(it))->getId() == componentId)
			result = (int32_t)(it - components.begin());

		it += compSize;
	}

	assert(result != -1);

	return (uint64_t)result;
}
