#pragma once

#include "Component.h"
#include "MeshGltfModel.h"
#include "Graphics.h"
#include "NetworkPacket.h"
#include "Physics.h"

class PeerComponent : public Component
{
	MeshGltfModel* model = nullptr;
	MeshGltfModel* weapon = nullptr;
	Texture* texture = nullptr;
	Texture* weaponTexture = nullptr;
	Graphics3D* renderer = nullptr;
	Vector<InstancedInfo> instanceInfos;
	Vector<PeerTransform> peerTransforms;
	Vector<int32_t> peerBodies;
	Physics& physics;
public:
	PeerComponent(Physics& physics);
	PeerComponent(Physics& physics, Vector<PeerTransform>&& peerTransforms);
	void update(float dt, Actor* owner) override;
	void render() override;
private:
	void eventPeerTransform(EventData* eventData);
	void eventPeerLeft(EventData* eventData);
};