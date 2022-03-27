#pragma once

#include <math.h>
#include "Types.h"

template <typename T>
struct Vector2
{
	Vector2() = default;
	Vector2(T x, T y) : x(x), y(y) {}

	template <typename T2>
	explicit operator Vector2<T2>()
	{
		return{ (T2)x,(T2)y };
	}
	T LenSq() const
	{
		return x * x + y * y;
	}
	T Len() const
	{
		return sqrt(LenSq());
	}
	Vector2& Normalize()
	{
		const T length = Len();
		x /= length;
		y /= length;
		return *this;
	}
	Vector2	GetNormalized() const
	{
		Vector2 norm = *this;
		norm.Normalize();
		return norm;
	}
	Vector2	operator-() const
	{
		return Vector2(-x, -y);
	}
	/*Vector2& operator=(const Vector2 &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}*/
	Vector2& operator+=(const Vector2 &rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	Vector2& operator-=(const Vector2 &rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	Vector2	operator+(const Vector2 &rhs) const
	{
		return Vector2(*this) += rhs;
	}
	Vector2	operator-(const Vector2 &rhs) const
	{
		return Vector2(*this) -= rhs;
	}
	Vector2& operator*=(const T &rhs)
	{
		x *= rhs;
		y *= rhs;
		return *this;
	}
	Vector2	operator*(const T &rhs) const
	{
		return Vector2(*this) *= rhs;
	}
	Vector2& operator*=(const Vector2& rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		return *this;
	}
	Vector2 operator*(const Vector2& rhs) const
	{
		return Vector2(*this) *= rhs;
	}
	T dotProduct(const Vector2& rhs) const
	{
		return x * rhs.x + y * rhs.y;
	}
	Vector2& operator/=(const T &rhs)
	{
		x /= rhs;
		y /= rhs;
		return *this;
	}
	Vector2	operator/(const T &rhs) const
	{
		return Vector2(*this) /= rhs;
	}
	Vector2& operator/=(const Vector2& rhs)
	{
		x /= rhs.x;
		y /= rhs.y;
		return *this;
	}
	Vector2 operator/(const Vector2& rhs) const
	{
		return Vector2(*this) /= rhs;
	}
	bool operator==(const Vector2 &rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Vector2 &rhs) const
	{
		return !(*this == rhs);
	}
public:
	T x = 0;
	T y = 0;
};

typedef Vector2<float> Vector2f;
typedef Vector2<int32_t> Vector2i;
typedef Vector2<uint32_t> Vector2ui;