#pragma once

#include "Vector2.h"

template <typename T>
class Rect
{
public:
	inline Rect() = default;
	inline Rect(T left, T bottom, T width, T height) : bottom(bottom), left(left), width(width), height(height) {}
	//Rect(const Rect& rect) : bottom(rect.bottom), left(rect.left), width(rect.width), height(rect.height) {}
	template <typename T2>
	inline operator Rect<T2>() const
	{
		return { (T2)left,(T2)bottom,(T2)width,(T2)height };
	}
	inline bool intersects(const Rect& rect) const
	{
		return bottom < rect.getTop() && getTop() > rect.bottom &&
			   left < rect.getRight() && getRight() > rect.left;
	}
	template <typename T2>
	inline bool contains(Vector2<T2> p) const
	{
		return p.y >= bottom && p.y <= getTop() && p.x >= left && p.x <= getRight();
	}
	template <typename T2>
	inline bool contains(Rect<T2> p) const
	{
		return p.bottom >= bottom && p.getTop() <= getTop() && p.left >= left && p.getRight() <= getRight();
	}
	inline T getTop() const
	{
		return bottom + height;
	}
	inline T getRight() const
	{
		return left + width;
	}
public:
	T bottom;
	T left;
	T width;
	T height;
};

typedef Rect<int32_t> IntRect;
typedef Rect<float> FloatRect;