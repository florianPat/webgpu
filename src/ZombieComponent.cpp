#include "ZombieComponent.h"
#include "Window.h"
#include "EventDie.h"
#include "EventZombieReset.h"
#include "PhysicsLayers.h"
#include "EventHit.h"
#include "Actor.h"
#include <random>
#include "ModelComponent.h"

volatile uint32_t ZombieComponent::zombieDead = 0;
volatile uint32_t ZombieComponent::zombieCount = 1;

ZombieComponent::ZombieComponent(Physics& physicsIn, FireTriggers& fireTriggersIn, NetworkPacket& networkPacketIn, GameObjectManager& gomIn) : Component(utils::getGUID()),
	physics(physicsIn), renderer(Globals::window->getGfx().getRenderer<Graphics3D>()), fireTriggers(fireTriggersIn), networkPacket(networkPacketIn), gom(gomIn)
{
	model = Globals::window->getAssetManager()->getOrAddRes<MeshGltfModel>("models/zombie.glb", Vector<String>{"models/zombie_walk.glb", "models/zombie_die.glb"});
	model->animations.find("models/zombie_die.glb")->second.setPlayMode(Animation::PlayMode::NORMAL);
	texture = Globals::window->getAssetManager()->getOrAddRes<Texture>("models/Zombie_diffuse.png");
	downSound = Globals::window->getAssetManager()->getOrAddRes<Sound>("sound/zombie_down.wav");

	static float initialPositionOffset = 0.0f;
	static std::mt19937 rng;
	std::uniform_real_distribution dist(-ModelComponent::PLANE_SIZE / 2.0f, ModelComponent::PLANE_SIZE / 2.0f);
	if (initialPositionOffset == 0.0f)
	{
		initialPosition.x = sinf(initialPositionOffset) * 70.0f + (initialPositionOffset * 2.0f);
		initialPosition.z = cosf(initialPositionOffset) * 70.0f + (initialPositionOffset * 3.0f);
	}
	else
	{
		initialPosition.x = dist(rng);
		initialPosition.z = dist(rng);
	}
	initialPositionOffset = 1.0f;

	dist = std::uniform_real_distribution(WALK_SPEED_MIN, WALK_SPEED_MAX);
	walkSpeed = dist(rng);

	FloatCube floatCube(initialPosition.x, initialPosition.y, initialPosition.z, COLLIDER_SIZE, 16.0f, COLLIDER_SIZE);
	Physics::Collider cubeCollider(std::move(floatCube));
	Vector3f pos = initialPosition;
	String name("zombie");
	String append(1, (char)(fireTriggers.size() + '0'));
	name.append(append);
	Physics::Body zombieBody(std::move(pos), name, std::move(cubeCollider), { (int32_t)PhysicsLayer::PLANE, (int32_t)PhysicsLayer::GUN });
	zombieBodyId = physics.addElementPointer(std::move(zombieBody), (int32_t)PhysicsLayer::ZOMBIE);

	FloatCube radiusCube(initialPosition.x - HALF_BOX_SIZE, 0.0f, initialPosition.z - HALF_BOX_SIZE, HALF_BOX_SIZE * 2.0f, 32.0f, HALF_BOX_SIZE * 2.0f);
	Physics::Collider radiusCollider(std::move(radiusCube));
	name = "radius";
	name.append(append);
	Physics::Body radiusBody({ 10.0f, 100.0f, 0.0f }, name, std::move(radiusCollider), {}, true, false);
	radiusBodyId = physics.addElementPointer(std::move(radiusBody), (int32_t)PhysicsLayer::RADIUS);
	
	fireTriggerIndex = fireTriggers.size();
	// NOTE/TODO: This can load to a data race? Because the size gets incremented before the value is written!
	fireTriggers.emplace_back();
	Globals::eventManager->addListener(fireTriggers[fireTriggerIndex].healthEventIndex, Delegate<void(EventData*)>::from<ZombieComponent, &ZombieComponent::eventHit>(this));
	Globals::eventManager->addListener(fireTriggers[fireTriggerIndex].dieEventIndex, Delegate<void(EventData*)>::from<ZombieComponent, &ZombieComponent::eventDie>(this));
	Globals::eventManager->addListener(EventZombieReset::eventId, Delegate<void(EventData*)>::from<ZombieComponent, &ZombieComponent::eventReset>(this));
}

void ZombieComponent::update(float dt, Actor* owner)
{
	int32_t realIndex = physics.getRealIndex(zombieBodyId);
	fireTriggers[fireTriggerIndex].bodyIndex = realIndex;
	Physics::Body* zombieBody = physics.getBodyFromRealIndex(realIndex);
	zombieBody->vel.x = 0.0f;
	zombieBody->vel.y += Physics::GRAVITY * dt;
	zombieBody->vel.z = 0.0f;

	realIndex = physics.getRealIndex(radiusBodyId);
	Physics::Body* radiusBody = physics.getBodyFromRealIndex(realIndex);

	if (radiusBody->getIsTriggerd())
	{
		Physics::Body* playerBody = physics.getBodyFromRealIndex(radiusBody->getTriggerInformation().index);
		const Vector3f& playerPos = playerBody->pos;
		Vector3f target = playerPos - zombieBody->pos;
		target.y = 0.0f;

		if (target.z <= 4.0f && target.z >= -1.0f && target.x <= 2.0f && target.x >= -2.0f)
		{
			for (auto it = fireTriggers.begin(); it != fireTriggers.end(); ++it)
			{
				if (it->bodyIndex == radiusBody->getTriggerInformation().index)
				{
					EventHit::eventId = it->healthEventIndex;
					Globals::eventManager->TriggerEvent<EventHit>();
					break;
				}
			}
		}

		target.normalize();
		lookAtY = atan2f(target.z, target.x) - PIf / 2.0f;
		target *= walkSpeed;
		zombieBody->vel.x = target.x;
		zombieBody->vel.z = target.z;
	}

	Mat4x4 rotationMatrix = Mat4x4::rotate({ 0.0f, lookAtY, 0.0f });
	Vector3f rotationOffset = rotationMatrix * Vector3f(COLLIDER_SIZE, 0.0f, COLLIDER_SIZE);
	Vector3f translationOffset = { -2.0f, 0.0f, -2.0f };

	if (rotationOffset.x > 0.0f && rotationOffset.x < (COLLIDER_SIZE / 2.0f))
	{
		rotationOffset.x = COLLIDER_SIZE;
	}
	else if (rotationOffset.x < 0.0f)
	{
		translationOffset.x = 2.0f;
		if (rotationOffset.x > -(COLLIDER_SIZE / 2.0f))
		{
			rotationOffset.x = -COLLIDER_SIZE;
		}
	}
	if (rotationOffset.z > 0.0f && rotationOffset.z < (COLLIDER_SIZE / 2.0f))
	{
		rotationOffset.z = COLLIDER_SIZE;
	}
	else if (rotationOffset.z < 0.0f)
	{
		translationOffset.z = 2.0f;
		if (rotationOffset.z > -(COLLIDER_SIZE / 2.0f))
		{
			rotationOffset.z = -COLLIDER_SIZE;
		}
	}

	FloatCube& zombieCollider = zombieBody->getCollider().collider.cube;
	zombieCollider.left = zombieBody->pos.x + (zombieBody->vel.x * dt) + translationOffset.x;
	zombieCollider.bottom = zombieBody->pos.y + (zombieBody->vel.y * dt);
	zombieCollider.back = zombieBody->pos.z + (zombieBody->vel.z * dt) + translationOffset.z;
	zombieCollider.width = rotationOffset.x;
	zombieCollider.depth = rotationOffset.z;

	FloatCube& radiusCollider = radiusBody->getCollider().collider.cube;
	radiusCollider.left = zombieCollider.left - HALF_BOX_SIZE;
	radiusCollider.back = zombieCollider.back - HALF_BOX_SIZE;

	hitPoints = HIT_HEALTH_COST * dt;

	if (health <= 0.0f && animationStateName != "models/zombie_die.glb")
	{
		// Globals::window->getAudio().start(downSound);
		setDieAnimation();
		zombieBody->setIsActive(false);
		radiusBody->setIsActive(false);
		assert(fireTriggers[fireTriggerIndex].dieEventIndex < 256);
		if (health != PEER_ZOMBIE_DOWN_POINTS)
		{
			networkPacket.zombie = (uint8_t)fireTriggers[fireTriggerIndex].dieEventIndex;
		}
		++zombieDead;
	}
	if ((!doNotRender) && animationStateName == "models/zombie_die.glb" && model->animations.find(animationStateName)->second.isAnimationFinished())
	{
		doNotRender = true;
	}

	if (reset)
	{
		health = 100.0f;
		animationStateName = "models/zombie_walk.glb";
		zombieBody->vel = { 0.0f, 0.0f, 0.0f };
		zombieBody->pos = initialPosition;
		reset = false;
		doNotRender = false;
		zombieBody->setIsActive(true);
		radiusBody->setIsActive(true);
	}

	if (zombieDead == zombieCount)
	{
		zombieDead = 0;
		if (zombieCount + WAVE_ADD <= MAX_WAVE)
		{
			zombieCount += WAVE_ADD;
			for (uint32_t i = 0; i < WAVE_ADD; ++i)
			{
				gom.addComponent<ZombieComponent>(0, *owner, physics, fireTriggers, networkPacket, gom);
			}
		}
		Globals::eventManager->TriggerEvent<EventZombieReset>(false);
	}
}

void ZombieComponent::render()
{
	if (doNotRender)
	{
		return;
	}

	int32_t realIndex = physics.getRealIndex(zombieBodyId);
	Physics::Body* zombieBody = physics.getBodyFromRealIndex(realIndex);
	SkeletalAnimation& walkAnimation = model->animations.find(animationStateName)->second;
	walkAnimation.baseTranslation.z = 0.1f;
	walkAnimation.updateJoints();
	renderer->draw(*model, zombieBody->pos, { 0.0f, lookAtY, 0.0f }, { 0.1f, 0.1f, 0.1f }, texture, model->uniformBuffer);
}

void ZombieComponent::eventHit(EventData* eventData)
{
	health -= hitPoints;
}

void ZombieComponent::eventDie(EventData* eventData)
{
	health = PEER_ZOMBIE_DOWN_POINTS;
}

void ZombieComponent::eventReset(EventData* eventData)
{
	reset = true;
}

void ZombieComponent::setDieAnimation()
{
	animationStateName = "models/zombie_die.glb";
	model->animations.find(animationStateName)->second.restart();
}
