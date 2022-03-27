#pragma once

#include "Vector2.h"
#include "Rect.h"
#include "Mat4x4.h"
#include "Color.h"
#include "Texture.h"

class Sprite
{
	const Texture* texture = nullptr;
	IntRect rect = IntRect(0, 0, 0, 0);
public:
    //NOTE: Origin in 0 - 1 space
	Vector2f pos = { 0.0f, 0.0f }, org = { 0.0f, 0.0f };
	Vector2f scl = { 1.0f, 1.0f };
	Vector2i size = { 0, 0 };
	float rot = 0.0f;
	Color color = Colors::White;
public:
	Sprite() = default;
	Sprite(const Texture* texture);
	Sprite(const Texture* texture, const IntRect& rect);
	void setTexture(const Texture* texture, bool resetRect = true);
	//NOTE: Texture rect in texture widht/height "space". Not 0 - 1 space!!
	void setTextureRect(const IntRect& rect);
	const Texture* getTexture() const;
	//NOTE: Texture rect in texture widht/height "space". Not 0 - 1 space!!
	const IntRect& getTextureRect() const;
	FloatRect getGlobalBounds() const;
	Vector2f getSize() const;
	float getWidth() const;
	float getHeight() const;
	void setScale(float factor);
	void setScale(float factorX, float factorY);
	const Mat4x4 getTransform() const;
};