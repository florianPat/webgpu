#include "Sprite.h"
#include "Utils.h"
#include "Mat4x4.h"

Sprite::Sprite(const Texture* texture) : texture(texture), rect(0, 0, texture->getWidth(), texture->getHeight()),
	size(texture->getWidth(), texture->getHeight())
{
}

Sprite::Sprite(const Texture* texture, const IntRect & rect) : texture(texture), rect(rect), size(rect.width, rect.height)
{
}

void Sprite::setTexture(const Texture* textureIn, bool resetRect)
{
	assert(textureIn != nullptr);
	assert(*textureIn);
	texture = textureIn;
	if (resetRect)
	{
		rect = { 0, 0, (int32_t)texture->getWidth(), (int32_t)texture->getHeight() };
	}
	size = { (int32_t)texture->getWidth(), (int32_t)texture->getHeight() };
}

void Sprite::setTextureRect(const IntRect & rectIn)
{
	assert(rectIn.left >= 0);
	assert(rectIn.bottom >= 0);
	assert(rectIn.getRight() > 0 && (uint32_t)rectIn.getRight() <= texture->getWidth());
	assert(rectIn.getTop() > 0 && (uint32_t)rectIn.getTop() <= texture->getHeight());
	rect = rectIn;
	size = { rect.width, rect.height };
}

const Texture * Sprite::getTexture() const
{
	return texture;
}

const IntRect & Sprite::getTextureRect() const
{
	return rect;
}

FloatRect Sprite::getGlobalBounds() const
{
	float halfWidth = (rect.getRight() - rect.left) / 2.0f;
	float halfHeight = (rect.getTop() - rect.bottom) / 2.0f;

	auto transform = getTransform();

	Vector2f leftTop = transform * Vector2f{ -halfWidth, -halfHeight };
	Vector2f rightBottom = transform * Vector2f { halfWidth, halfHeight };

	FloatRect result = FloatRect(leftTop.x, leftTop.y, (rightBottom.x - leftTop.x), (rightBottom.y - leftTop.y));

	return result;
}

Vector2f Sprite::getSize() const
{
	return Vector2f{ getWidth(), getHeight() };
}

float Sprite::getWidth() const
{
	return size.x * scl.x;
}

float Sprite::getHeight() const
{
	return size.y * scl.y;
}

void Sprite::setScale(float factor)
{
	scl = { factor, factor };
}

void Sprite::setScale(float factorX, float factorY)
{
	scl = { factorX, factorY };
}

const Mat4x4 Sprite::getTransform() const
{
	Mat4x4 result = Mat4x4::translate(pos) * Mat4x4::rotate(rot) * Mat4x4::translate(-(org * Vector2f{ getWidth(), getHeight() })) *
		Mat4x4::scale({ getWidth(), getHeight() });

	return result;
}