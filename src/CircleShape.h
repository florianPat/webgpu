#pragma once

#include "Vector2.h"
#include "Color.h"
#include "Mat4x4.h"

struct CircleShape
{
	Vector2f pos = { 0.0f, 0.0f };
	float radius = 1.0f;
	Color fillColor = Colors::White;
public:
	CircleShape() = default;
	Mat4x4 getTransform() const;
};