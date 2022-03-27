#pragma once

#include "Vector3.h"

template <typename T>
class Cube
{
public:
	inline Cube() = default;
	inline Cube(T left, T bottom, T back, T width, T height, T depth) : bottom(bottom), left(left), back(back), width(width),
		height(height), depth(depth) {}
	//Rect(const Rect& rect) : bottom(rect.bottom), left(rect.left), width(rect.width), height(rect.height) {}
	template <typename T2>
	inline operator Cube<T2>() const
	{
		return { (T2)left,(T2)bottom,(T2)back,(T2)width,(T2)height,(T2)depth };
	}
	inline bool intersects(const Cube& cube) const
	{
		return bottom < cube.getTop() && getTop() > cube.bottom &&
			left < cube.getRight() && getRight() > cube.left &&
			back < cube.getFront() && getFront() > cube.back;
	}
	template <typename T2>
	inline bool contains(Vector3<T2> p) const
	{
		return p.y >= bottom && p.y <= getTop() && p.x >= left && p.x <= getRight() && p.z >= back && p.z <= getFront();
	}
	template <typename T2>
	inline bool contains(Cube<T2> p) const
	{
		return p.bottom >= bottom && p.getTop() <= getTop() && p.left >= left && p.getRight() <= getRight() && p.back >= back &&
			p.getFront() <= getFront();
	}
	inline T getTop() const
	{
		return bottom + height;
	}
	inline T getRight() const
	{
		return left + width;
	}
	inline T getFront() const
	{
		return back + depth;
	}
public:
	T bottom;
	T left;
	T back;
	T width;
	T height;
	T depth;
};

typedef Cube<int32_t> IntCube;
typedef Cube<float> FloatCube;