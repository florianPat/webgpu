#include "RectangleShape.h"

Mat4x4 RectangleShape::getTransform() const
{
	Mat4x4 result = Mat4x4::identity();

	result *= Mat4x4::translate(pos);

	result *= Mat4x4::rotate(rotation);

	result *= Mat4x4::translate(-origin);

	result *= Mat4x4::scale({ getWidth(), getHeight() });

	return result;
}

float RectangleShape::getWidth() const
{
	return size.x;
}

float RectangleShape::getHeight() const
{
	return size.y;
}

void RectangleShape::setToFloatRect(const FloatRect& rect)
{
	pos = { rect.left, rect.bottom };
	size = { rect.width, rect.height };
}

RectangleShape::RectangleShape(const Vector2f& pos, const Vector2f& size, Color color) : pos(pos),
	size(size), fillColor(color)
{
}
