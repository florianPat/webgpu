#pragma once

#include "Vector2.h"
#include "Color.h"
#include "Mat4x4.h"
#include "Rect.h"

struct RectangleShape
{
	Vector2f pos = { 0.0f, 0.0f };
	Vector2f size = { 1.0f, 1.0f };
	Color fillColor = Colors::White;
	Vector2f origin = { 0.0f, 0.0f };
	float rotation = 0.0f;
public:
	RectangleShape() = default;
	RectangleShape(const Vector2f& pos, const Vector2f& size, Color color = Colors::White);
	void setToFloatRect(const FloatRect& rect);
	Mat4x4 getTransform() const;
	float getWidth() const;
	float getHeight() const;
};