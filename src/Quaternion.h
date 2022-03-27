#pragma once

#include "Vector3.h"
#include "Utils.h"

struct Quaternion
{
	Vector3f v;
	float w = 1.0f;
public:
	Quaternion() = default;
	Quaternion(const Vector3f& axisNormal, float angle)
	{
		v = axisNormal * sinf(angle / 2.0f);
		w = cosf(angle / 2.0f);
	}
	Vector3f getAxisNormal() const
	{
		float s = sqrtf(1.0f - w * w);
		if (s < 0.01f)
		{
			return { 1.0f, 0.0f, 0.0f };
		}
		return v / s;
	}
	float getAngle() const
	{
		return acosf(w) * 2.0f;
	}
	Quaternion operator-() const
	{
		Quaternion result;
		result.v = -v;
		result.w = w;
		return result;
	}
	Quaternion& operator+=(const Quaternion& rhs)
	{
		v += rhs.v;
		w += rhs.w;
		return *this;
	}
	Quaternion operator+(const Quaternion& rhs) const
	{
		Quaternion result = *this;
		result += rhs;
		return result;
	}
	Quaternion& operator-=(const Quaternion& rhs)
	{
		v -= rhs.v;
		w -= rhs.w;
		return *this;
	}
	Quaternion operator-(const Quaternion& rhs) const
	{
		Quaternion result = *this;
		result -= rhs;
		return result;
	}
	Quaternion& operator*=(float rhs)
	{
		v *= rhs;
		w *= rhs;
		return *this;
	}
	Quaternion operator*(float rhs) const
	{
		Quaternion result = *this;
		result *= rhs;
		return result;
	}
	Quaternion operator*=(const Quaternion& rhs)
	{
		v = v * rhs.w + rhs.v * w + rhs.v.crossProduct(v);
		w = w * rhs.w - v.dotProduct(rhs.v);
		return *this;
	}
	Quaternion operator*(const Quaternion& rhs) const
	{
		Quaternion result = *this;
		result *= rhs;
		return result;
	}
	Vector3f operator*(const Vector3f& rhs) const
	{
		Quaternion p;
		p.v = rhs;
		p.w = 0.0f;

		const Quaternion& q = *this;
		Quaternion result = q * p * (-q);
		return result.v;
	}
	Quaternion slerp(const Quaternion& other, float t) const
	{
		const Quaternion& q = *this;
		Quaternion r = other;

		float flCosOmega = w * r.w + r.v.dotProduct(v);
		if (flCosOmega < 0)
		{
			// Avoid going the long way around.
			r.w = -r.w;
			r.v = -r.v;
			flCosOmega = -flCosOmega;
		}

		float k0, k1;
		if (flCosOmega > 0.9999f)
		{
			// Very close, use a linear interpolation.
			k0 = 1 - t;
			k1 = t;
		}
		else
		{
			// Trig identity, sin^2 + cos^2 = 1
			float flSinOmega = sqrtf(1 - flCosOmega * flCosOmega);

			// Compute the angle omega
			float flOmega = atan2f(flSinOmega, flCosOmega);
			float flOneOverSinOmega = 1 / flSinOmega;

			k0 = sinf((1 - t) * flOmega) * flOneOverSinOmega;
			k1 = sinf(t * flOmega) * flOneOverSinOmega;
		}

		// Interpolate
		Quaternion result;
		result.w = q.w * k0 + r.w * k1;
		result.v = q.v * k0 + r.v * k1;

		return result;
	}
	float lengthSqaured() const
	{
		return w * w + v.lengthSquared();
	}
	float length() const
	{
		return sqrtf(lengthSqaured());
	}
	Quaternion& normalize()
	{
		const float quatLength = length();
		v.x /= quatLength;
		v.y /= quatLength;
		v.z /= quatLength;
		w /= quatLength;
		return *this;
	}
	Quaternion getNormalized() const
	{
		Quaternion result = *this;
		result.normalize();
		return result;
	}
	Vector3f toEulerAngles() const
	{
		Vector3f result;

		float sinr_cosp = 2 * (w * v.x + v.y * v.z);
		float cosr_cosp = 1 - 2 * (v.x * v.x + v.y * v.y);
		result.x = atan2f(sinr_cosp, cosr_cosp);

		float sinp = 2 * (w * v.y - v.z * v.x);
		if (fabsf(sinp) >= 1.0f)
			result.y = PIf / 2.0f;
		else
			result.y = asinf(sinp);

		float siny_cosp = 2 * (w * v.z + v.x * v.y);
		float cosy_cosp = 1 - 2 * (v.y * v.y + v.z * v.z);
		result.z = atan2f(siny_cosp, cosy_cosp);

		return result;
	}
};