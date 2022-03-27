#include "DoenerRoomComponent.h"
#include "Globals.h"
#include "Window.h"
#include "PhysicsLayers.h"

DoenerRoomComponent::DoenerRoomComponent(Physics& physics, const Vector3f& pos) : Component(utils::getGUID()),
physics(physics), renderer(Globals::window->getGfx().getRenderer<Graphics3D>())
{
	doenerTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("images/doener.png");

	Vector3f bodyPos = pos;
	FloatCube floatCube = {};
	floatCube.left = pos.x;
	floatCube.bottom = pos.y;
	floatCube.back = pos.z;
	floatCube.width = boxSize.x;
	floatCube.height = boxSize.y;
	floatCube.depth = boxSize.z;
	Physics::Collider cubeCollider(floatCube);
	Physics::Body cubeBody(std::move(bodyPos), "doener_room", std::move(cubeCollider), { (int32_t)PhysicsLayer::PLANE });
	cubeBodyId = physics.addElementPointer(std::move(cubeBody), (int32_t)PhysicsLayer::PLANE);
}

void DoenerRoomComponent::update(float dt, Actor* owner)
{
	int32_t realIndex = physics.getRealIndex(cubeBodyId);
	Physics::Body* cubeBody = physics.getBodyFromRealIndex(realIndex);
	cubeBody->vel.x = 0.0f;
	cubeBody->vel.y += Physics::GRAVITY * dt;
	cubeBody->vel.z = 0.0f;

	FloatCube& cubeCollider = cubeBody->getCollider().collider.cube;
	cubeCollider.left = cubeBody->pos.x + (cubeBody->vel.x * dt);
	cubeCollider.bottom = cubeBody->pos.y + (cubeBody->vel.y * dt);
	cubeCollider.back = cubeBody->pos.z + (cubeBody->vel.z * dt);
}

void DoenerRoomComponent::render()
{
	int32_t realIndex = physics.getRealIndex(cubeBodyId);
	Physics::Body* cubeBody = physics.getBodyFromRealIndex(realIndex);
	Model model(Model::cube());
	renderer->draw(model, cubeBody->pos + boxSize / 2.0f, { 0.0f, 0.0f, 0.0f }, boxSize / 2.0f, doenerTexture);
}
