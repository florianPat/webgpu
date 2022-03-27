#pragma once

#include "Component.h"
#include "Physics.h"

class PhysicsUpdateComponent : public Component
{
    Physics& physics;
public:
    PhysicsUpdateComponent(Physics& physics) : Component(utils::getGUID()), physics(physics)
    {
    }

    void update(float dt, Actor* owner) override
    {
        physics.update(dt);
    }
};