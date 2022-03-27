#pragma once

#include "Component.h"
#include "MeshGltfModel.h"
#include "Physics.h"

class DoenerRoomComponent : public Component
{
	Texture* doenerTexture = nullptr;
	Physics& physics;
	Graphics3D* renderer = nullptr;
	uint32_t cubeBodyId;
	Vector3f boxSize = { 70.0f, 45.0f, 30.0f };
public:
	DoenerRoomComponent(Physics& physics, const Vector3f& pos);
	void update(float dt, Actor* owner) override;
	void render() override;
};