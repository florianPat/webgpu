#include "PeerComponent.h"
#include "Window.h"
#include "EventManager.h"
#include "EventPeerTransform.h"
#include "EventPeerLeft.h"
#include "PhysicsLayers.h"

PeerComponent::PeerComponent(Physics& physicsIn) : Component(utils::getGUID()), renderer(Globals::window->getGfx().getRenderer<GraphicsVK3D>()), physics(physicsIn)
{
	model = Globals::window->getAssetManager()->getOrAddRes<MeshGltfModel>("models/thebosspeer.glb", Vector<String>{ "models/theboss_reload.glb" });
	weapon = Globals::window->getAssetManager()->getOrAddRes<MeshGltfModel>("models/weapon.glb");

	texture = Globals::window->getAssetManager()->getOrAddRes<Texture>("models/Boss_diffuse.png");
	weaponTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("models/Weapon_texture.png");

	Globals::eventManager->addListener(EventPeerTransfrom::eventId, Delegate<void(EventData*)>::from<PeerComponent, &PeerComponent::eventPeerTransform>(this));
	Globals::eventManager->addListener(EventPeerLeft::eventId, Delegate<void(EventData*)>::from<PeerComponent, &PeerComponent::eventPeerLeft>(this));

	for (uint32_t i = 0; i < 6; ++i)
	{
		FloatCube floatCube(100.0f, 10.0f, 0.0f, 5.0f, 5.0f, 5.0f);
		Physics::Collider cubeCollider(std::move(floatCube));
		Physics::Body cubeBody({ 10.0f, 100.0f, 0.0f }, "player", std::move(cubeCollider), { (int32_t)PhysicsLayer::RADIUS }, false, false);
		cubeBody.usesGravity = false;
		int32_t peerBodyId = physics.addElementPointer(std::move(cubeBody), (int32_t)PhysicsLayer::PLAYER);
		Physics::Body* body = physics.getBodyFromRealIndex(physics.getRealIndex(peerBodyId));
		body->setIsActive(false);
		peerBodies.push_back(peerBodyId);
	}
}

PeerComponent::PeerComponent(Physics& physics, Vector<PeerTransform>&& peerTransformsIn) : PeerComponent(physics)
{
	peerTransforms = std::move(peerTransformsIn);
}

void PeerComponent::update(float dt, Actor* owner)
{
	InstancedInfo newInstanceInfo{ Vector3f(), Vector3f(), Vector3f{0.1f, 0.1f, 0.1f} };
	while (instanceInfos.size() < peerTransforms.size())
	{
		instanceInfos.push_back(newInstanceInfo);
		assert(peerBodies.size() >= instanceInfos.size());
		utils::logF("activate body %d", instanceInfos.size() - 1);
		Physics::Body* body = physics.getBodyFromRealIndex(physics.getRealIndex(peerBodies[instanceInfos.size() - 1]));
		body->setIsActive(true);
	}
	while (instanceInfos.size() > peerTransforms.size())
	{
		assert(peerBodies.size() >= instanceInfos.size());
		utils::logF("deactivate body %d", instanceInfos.size() - 1);
		Physics::Body* body = physics.getBodyFromRealIndex(physics.getRealIndex(peerBodies[instanceInfos.size() - 1]));
		body->setIsActive(false);
		if (instanceInfos.size() == 1)
		{
			instanceInfos.clear();
			peerBodies.clear();
		}
		else
		{
			instanceInfos.pop_back();
			peerBodies.pop_back();
		}
	}

	assert(instanceInfos.size() == peerTransforms.size());
	for (uint32_t i = 0; i < peerTransforms.size(); ++i)
	{
		InstancedInfo& instanceInfo = instanceInfos[i];
		const PeerTransform& peerInfo = peerTransforms[i];

		instanceInfo.pos.x = (float)peerInfo.posX;
		instanceInfo.pos.y = (float)peerInfo.posY;
		instanceInfo.pos.z = (float)peerInfo.posZ;
		instanceInfo.rot.y = (float)peerInfo.rotY;

		Physics::Body* body = physics.getBodyFromRealIndex(physics.getRealIndex(peerBodies[i]));
		FloatCube& collider = body->getCollider().collider.cube;

		collider.left = instanceInfo.pos.x;
		collider.bottom = instanceInfo.pos.y;
		collider.back = instanceInfo.pos.z;
		body->pos = instanceInfo.pos;
		// TODO: rotX
	}
}

void PeerComponent::render()
{
	model->animations.find("models/theboss_reload.glb")->second.updateJoints();

	if (instanceInfos.empty())
	{
		return;
	}
	renderer->drawInstanced(*model, instanceInfos, texture, model->uniformBuffer);

	// TODO: Instanced drawing!
	for (auto it = instanceInfos.begin(); it != instanceInfos.end(); ++it)
	{
		Vector3f from;
		Vector3f to = (Mat4x4::rotateX(-it->rot.x) * Vector3f { 0.0f, 0.0f, 1.0f }).getNormalized();
		to = from + to;
		Mat4x4 xLookAtCameraRotation = Mat4x4::lookAt(from, to);
		Mat4x4 updatedCameraTransform = renderer->camera.persProj * xLookAtCameraRotation * renderer->camera.cameraTransform;
		Vector3f weaponPos = Mat4x4::rotateY(it->rot.y) * Vector3f { -1.2f, 14.1f, 4.0f };
		Mat4x4 modelCameraTransform = updatedCameraTransform * Mat4x4::translate(it->pos + weaponPos) * Mat4x4::rotateY(it->rot.y + PIf / 2.0f)
			* Mat4x4::scale({ 4.0f, 5.0f, 5.0f });
		renderer->draw(*weapon, modelCameraTransform, weaponTexture);
	}
}

void PeerComponent::eventPeerTransform(EventData* eventData)
{
	auto peerEventData = (EventPeerTransfrom*)eventData;
	
	if (peerEventData->playerId == 0 && peerTransforms.empty())
	{
		utils::log("first peer transform added.");
		peerTransforms.push_back(PeerTransform());
	}
	while (peerEventData->playerId > (peerTransforms.size() - 1))
	{
		utils::logF("%d nth peer transform added");
		peerTransforms.push_back(PeerTransform());
	}
	
	peerTransforms[peerEventData->playerId] = peerEventData->peerTransform;
}

void PeerComponent::eventPeerLeft(EventData* eventData)
{
	assert(!peerTransforms.empty());
	if (peerTransforms.size() == 1)
	{
		peerTransforms.clear();
	}
	else
	{
		peerTransforms.pop_back();
	}
}
