#include "ThreeDLevel.h"
#include "EventLevelReload.h"
#include "Delegate.h"
#include "ModelComponent.h"
#include "PhysicsUpdateComponent.h"
#include "PlayerComponent.h"
#include "ZombieComponent.h"
#include "PeerComponent.h"
#include "NetworkUpdateComponent.h"
#include "Globals.h"
#include "DoenerRoomComponent.h"

void ThreeDLevel::eventLevelReloadHandler(EventData * eventData)
{
	newLevel = makeUnique<ThreeDLevel>("", "");
	endLevel = true;
}

ThreeDLevel::ThreeDLevel(const String& lobbyName, const String& username) : Level(), fireTriggers(), lobbyName(lobbyName), username(username)
{
}

void ThreeDLevel::init()
{
	gom.addActor(sizeof(ModelComponent) + (sizeof(uint32_t) * 1));
	gom.addRenderComponent<ModelComponent>(0, physics);
	Actor* actor = gom.addActor(sizeof(PlayerComponent) + (sizeof(uint32_t) * 1));
	gom.addComponent<PlayerComponent>(0, *actor, physics, fireTriggers, networkPacket);
	actor = gom.addActor((sizeof(ZombieComponent) * ZombieComponent::MAX_WAVE) + (sizeof(uint32_t) * ZombieComponent::MAX_WAVE));
	gom.addComponent<ZombieComponent>(0, *actor, physics, fireTriggers, networkPacket, gom);
	actor = gom.addActor(sizeof(PeerComponent) + (sizeof(uint32_t) * 1));
	gom.addComponent<PeerComponent>(0, *actor, physics);
	actor = gom.addActor(sizeof(DoenerRoomComponent) + (sizeof(uint32_t) * 1));
	gom.addComponent<DoenerRoomComponent>(0, *actor, physics, Vector3f{ 0.0f, 70.0f, ModelComponent::PLANE_SIZE / 2.0f - 60.0f });
	gom.endPipeline();
	actor = gom.addMainThreadActor(sizeof(NetworkUpdateComponent) + (sizeof(uint32_t) * 1));
	gom.addUpdateComponent<NetworkUpdateComponent>(*actor, lobbyName.c_str(), username, networkPacket);
	actor = gom.addActor(sizeof(PhysicsUpdateComponent) + (sizeof(uint32_t) * 1));
	gom.addUpdateComponent<PhysicsUpdateComponent>(*actor, physics);
	gom.endPipeline();

	eventManager.addListener(EventLevelReload::eventId, Delegate<void(EventData*)>::from<ThreeDLevel,
		&ThreeDLevel::eventLevelReloadHandler>(this));

	Globals::window->hideAndClipCursor();
}
