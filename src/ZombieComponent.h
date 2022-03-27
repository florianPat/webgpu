#pragma once

#include "Component.h"
#include "MeshGltfModel.h"
#include "Physics.h"
#include "FireTrigger.h"
#include "NetworkPacket.h"
#include "GameObjectManager.h"
#include "Sound.h"

class ZombieComponent : public Component
{
	static constexpr float WALK_SPEED_MIN = 15.0f;
	static constexpr float WALK_SPEED_MAX = 20.0f;
	static constexpr float COLLIDER_SIZE = 7.0f;
	static constexpr float HIT_HEALTH_COST = 100.0f;
	static constexpr float HALF_BOX_SIZE = 60.0f;
	static constexpr uint32_t WAVE_ADD = 2;
	static constexpr float PEER_ZOMBIE_DOWN_POINTS = -300.0f;
	Vector3f initialPosition = Vector3f(0.0f, 15.0f, 0.0f);

	static volatile uint32_t zombieDead;
	static volatile uint32_t zombieCount;

	MeshGltfModel* model = nullptr;
	Texture* texture = nullptr;
	Physics& physics;
	Graphics3D* renderer = nullptr;
	int32_t zombieBodyId = -1;
	int32_t radiusBodyId = -1;
	FireTriggers& fireTriggers;
	uint32_t fireTriggerIndex = 0;
	float hitPoints = 0;
	float health = 100.0f;
	float lookAtY = 0.0f;
	String animationStateName = "models/zombie_walk.glb";
	NetworkPacket& networkPacket;
	bool reset = false;
	bool doNotRender = false;
	GameObjectManager& gom;
	Sound* downSound = nullptr;
	float walkSpeed = WALK_SPEED_MIN;
public:
	static constexpr uint32_t MAX_WAVE = 10;
public:
	ZombieComponent(Physics& physics, FireTriggers& fireTriggers, NetworkPacket& networkPacket, GameObjectManager& gom);
	void update(float dt, Actor* owner) override;
	void render() override;
private:
	void eventHit(EventData* eventData);
	void eventDie(EventData* eventData);
	void eventReset(EventData* eventData);
	void setDieAnimation();
};