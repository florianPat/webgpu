#pragma once

#include "Types.h"

template <typename T>
struct Vector3
{
	T x = 0, y = 0, z = 0;
public:
	Vector3() = default;
	Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
	Vector3	operator-() const
	{
		return Vector3(-x, -y, -z);
	}
	Vector3 operator+(const Vector3<T>& rhs) const
	{
		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}
	Vector3& operator+=(const Vector3& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;

		return *this;
	}
	Vector3 operator-(const Vector3<T>& rhs) const
	{
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}
	Vector3& operator-=(const Vector3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;

		return *this;
	}
	Vector3& operator*=(const T &rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}
	Vector3	operator*(const T &rhs) const
	{
		return Vector3(*this) *= rhs;
	}
	Vector3& operator*=(const Vector3& rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}
	Vector3 operator*(const Vector3& rhs) const
	{
		return Vector3(*this) *= rhs;
	}
	T dotProduct(const Vector3& rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}
	Vector3 crossProduct(const Vector3& rhs) const
	{
		return Vector3f(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}
	Vector3& operator/=(const T &rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}
	Vector3 operator/(const T &rhs) const
	{
		return Vector3(*this) /= rhs;
	}
	Vector3& operator/=(const Vector3& rhs)
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		return *this;
	}
	Vector3 operator/(const Vector3& rhs) const
	{
		return Vector3(*this) /= rhs;
	}
	bool operator==(const Vector3 &rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}
	bool operator!=(const Vector3 &rhs) const
	{
		return !(*this == rhs);
	}
	bool operator<(const Vector3& rhs) const
	{
		return (x < rhs.x && y < rhs.y && z < rhs.z);
	}
	T lengthSquared() const
	{
		return x * x + y * y + z * z;
	}
	T length() const
	{
		return sqrtf(lengthSquared());
	}
	Vector3& normalize()
	{
		const T vecLength = length();
		x /= vecLength;
		y /= vecLength;
		z /= vecLength;
		return *this;
	}
	Vector3	getNormalized() const
	{
		Vector3 norm = *this;
		norm.normalize();
		return norm;
	}
};

typedef Vector3<float> Vector3f;
typedef Vector3<int32_t> Vector3i;
typedef Vector3<uint32_t> Vector3ui;