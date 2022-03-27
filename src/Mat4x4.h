#pragma once

#include "Vector2.h"
#include "Utils.h"
#include <math.h>
#include "Vector3.h"
#include "Quaternion.h"

struct Mat4x4
{
	//					column			row
	//NOTE: Column major. 0    *   4  +   0
	float matrix[4 * 4] = { 0 };
public:
	Mat4x4() = default;
	//col * 4 + row
	inline Mat4x4& operator*=(const Mat4x4& rhs)
	{
		Mat4x4 result;

		result.matrix[0 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[0 * 4 + 3];

		result.matrix[1 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[1 * 4 + 3];
		result.matrix[2 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[2 * 4 + 3];
		result.matrix[3 * 4 + 0] = matrix[0 * 4 + 0] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 0] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 0] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 0] * rhs.matrix[3 * 4 + 3];

		result.matrix[0 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[0 * 4 + 3];
		result.matrix[0 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[0 * 4 + 3];
		result.matrix[0 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[0 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[0 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[0 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[0 * 4 + 3];

		result.matrix[1 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[1 * 4 + 3];
		result.matrix[1 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[1 * 4 + 3];
		result.matrix[1 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[1 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[1 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[1 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[1 * 4 + 3];

		result.matrix[2 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[2 * 4 + 3];
		result.matrix[2 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[2 * 4 + 3];
		result.matrix[2 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[2 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[2 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[2 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[2 * 4 + 3];

		result.matrix[3 * 4 + 1] = matrix[0 * 4 + 1] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 1] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 1] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 1] * rhs.matrix[3 * 4 + 3];
		result.matrix[3 * 4 + 2] = matrix[0 * 4 + 2] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 2] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 2] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 2] * rhs.matrix[3 * 4 + 3];
		result.matrix[3 * 4 + 3] = matrix[0 * 4 + 3] * rhs.matrix[3 * 4 + 0] + matrix[1 * 4 + 3] * rhs.matrix[3 * 4 + 1] + matrix[2 * 4 + 3] * rhs.matrix[3 * 4 + 2] + matrix[3 * 4 + 3] * rhs.matrix[3 * 4 + 3];

		*this = result;

		return *this;
	}

	inline Mat4x4 operator*(const Mat4x4& rhs) const
	{
		Mat4x4 lhs = *this;
		lhs *= rhs;
		return lhs;
	}

	inline Vector2f operator*(const Vector2f& rhs) const
	{
		Vector2f result;

		result.x = matrix[0 * 4 + 0] * rhs.x + matrix[1 * 4 + 0] * rhs.y + matrix[3 * 4 + 0] * 1.0f;
		result.y = matrix[0 * 4 + 1] * rhs.x + matrix[1 * 4 + 1] * rhs.y + matrix[3 * 4 + 1] * 1.0f;

		return result;
	}

	inline Vector3f operator*(const Vector3f& rhs) const
	{
		Vector3f result;

		result.x = matrix[0 * 4 + 0] * rhs.x + matrix[1 * 4 + 0] * rhs.y + matrix[2 * 4 + 0] * rhs.z + matrix[3 * 4 + 0] * 1.0f;
		result.y = matrix[0 * 4 + 1] * rhs.x + matrix[1 * 4 + 1] * rhs.y + matrix[2 * 4 + 1] * rhs.z + matrix[3 * 4 + 1] * 1.0f;
		result.z = matrix[0 * 4 + 2] * rhs.x + matrix[1 * 4 + 2] * rhs.y + matrix[2 * 4 + 2] * rhs.z + matrix[3 * 4 + 2] * 1.0f;

		return result;
	}

	static Mat4x4 identity()
	{
		Mat4x4 result;

		result.matrix[0 * 4 + 0] = 1;
		result.matrix[1 * 4 + 1] = 1;
		result.matrix[2 * 4 + 2] = 1;
		result.matrix[3 * 4 + 3] = 1;

		return result;
	}

	static Mat4x4 translate(const Vector2f& trans)
	{
		Mat4x4 result = identity();

		result.matrix[3 * 4 + 0] = trans.x;
		result.matrix[3 * 4 + 1] = trans.y;

		return result;
	}

	static Mat4x4 translate(const Vector3f& trans)
	{
		Mat4x4 result = identity();

		result.matrix[3 * 4 + 0] = trans.x;
		result.matrix[3 * 4 + 1] = trans.y;
		result.matrix[3 * 4 + 2] = trans.z;

		return result;
	}

	static Mat4x4 scale(const Vector2f& scl)
	{
		Mat4x4 result = identity();

		result.matrix[0 * 4 + 0] = scl.x;
		result.matrix[1 * 4 + 1] = scl.y;

		return result;
	}

	static Mat4x4 scale(const Vector3f& scl)
	{
		Mat4x4 result = identity();

		result.matrix[0 * 4 + 0] = scl.x;
		result.matrix[1 * 4 + 1] = scl.y;
		result.matrix[2 * 4 + 2] = scl.z;

		return result;
	}

	static Mat4x4 rotate(const float rot)
	{
		Mat4x4 result = identity();

		if (rot != 0.0f)
		{
			float cosA = cosf(rot);
			float sinA = sinf(rot);

			Mat4x4 rotMat = identity();
			rotMat.matrix[0 * 4 + 0] = cosA;
			rotMat.matrix[1 * 4 + 0] = sinA;
			rotMat.matrix[0 * 4 + 1] = -sinA;
			rotMat.matrix[1 * 4 + 1] = cosA;

			result *= rotMat;
		}

		return result;
	}

	static Mat4x4 rotateX(float rot)
	{
		Mat4x4 result = identity();

		if (rot != 0.0f)
		{
			float cosA = cosf(rot);
			float sinA = sinf(rot);

			Mat4x4 rotMat = identity();
			rotMat.matrix[1 * 4 + 1] = cosA;
			rotMat.matrix[2 * 4 + 1] = sinA;
			rotMat.matrix[1 * 4 + 2] = -sinA;
			rotMat.matrix[2 * 4 + 2] = cosA;

			result *= rotMat;
		}

		return result;
	}

	static Mat4x4 rotateY(float rot)
	{
		Mat4x4 result = identity();

		if (rot != 0.0f)
		{
			float cosA = cosf(rot);
			float sinA = sinf(rot);

			Mat4x4 rotMat = identity();
			rotMat.matrix[0 * 4 + 0] = cosA;
			rotMat.matrix[0 * 4 + 2] = sinA;
			rotMat.matrix[2 * 4 + 0] = -sinA;
			rotMat.matrix[2 * 4 + 2] = cosA;

			result *= rotMat;
		}

		return result;
	}

	static Mat4x4 rotateZ(float rot)
	{
		return rotate(rot);
	}

	static Mat4x4 rotate(const Vector3f& eulerAngle)
	{
		return Mat4x4::rotateZ(eulerAngle.z) * Mat4x4::rotateY(eulerAngle.y) * Mat4x4::rotateX(eulerAngle.x);
	}

	static Mat4x4 rotate(float angle, const Vector3f& axisNormal)
	{
		float cosAngle = cosf(angle);
		float sinAngle = sinf(angle);

		Mat4x4 result = identity();
		// NOTE: Transposed, because its column major with column * 4 + row, but the definition is in row major (see milton)
		result.matrix[0 * 4 + 0] = cosAngle + (1 - cosAngle) * (axisNormal.x * axisNormal.x) + 0.0f;
		result.matrix[0 * 4 + 1] = 0.0f + (1 - cosAngle) * (axisNormal.x * axisNormal.y) - axisNormal.z * sinAngle;
		result.matrix[0 * 4 + 2] = 0.0f + (1 - cosAngle) * (axisNormal.x * axisNormal.z) + axisNormal.y * sinAngle;
		result.matrix[1 * 4 + 0] = 0.0f + (1 - cosAngle) * (axisNormal.y * axisNormal.x) + axisNormal.z * sinAngle;
		result.matrix[1 * 4 + 1] = cosAngle + (1 - cosAngle) * (axisNormal.y * axisNormal.y) + 0.0f;
		result.matrix[1 * 4 + 2] = 0.0f + (1 - cosAngle) * (axisNormal.y * axisNormal.z) - axisNormal.x * sinAngle;
		result.matrix[2 * 4 + 0] = 0.0f + (1 - cosAngle) * (axisNormal.z * axisNormal.x) - axisNormal.y * sinAngle;
		result.matrix[2 * 4 + 1] = 0.0f + (1 - cosAngle) * (axisNormal.z * axisNormal.y) + axisNormal.x * sinAngle;
		result.matrix[2 * 4 + 2] = cosAngle + (1 - cosAngle) * (axisNormal.z * axisNormal.z) + 0.0f;
		return result;
	}

	static Mat4x4 rotate(const Quaternion& quat)
	{
		float angle = quat.getAngle();
		Vector3f axisNormal = quat.getAxisNormal();

		return rotate(angle, axisNormal);
	}

	static Mat4x4 orthoProj(float left, float bottom, float right, float top)
	{
		Mat4x4 result = identity();

		result.matrix[0 * 4 + 0] = 2 / (right - left);
		//NOTE: Minus here because 0 is top left and not bottom left, so that width = -2 and not 2 and 0 = 0 and 0 in both cases
		//(flip)
		result.matrix[1 * 4 + 1] = -2 / (top - bottom);
		result.matrix[2 * 4 + 2] = 0.0f;
		result.matrix[3 * 4 + 0] = -(right + left) / (right - left);
		//NOTE: Need to add because middle coord is -1 and not 1
		result.matrix[3 * 4 + 1] = (top + bottom) / (top - bottom);
		result.matrix[3 * 4 + 2] = 0.0f;

		return result;
	}

	static Mat4x4 orthoProj(float nearPlane, float farPlane, float left, float bottom, float right, float top)
	{
		Mat4x4 result = identity();

		//NOTE: Aspect ratio in here for free (because right = top * aspectRatio is what one passes in here)
		//NOTE: Also look at persProj.

		//NOTE: scl
		result.matrix[0 * 4 + 0] = 2 / (right - left);
		result.matrix[1 * 4 + 1] = -2 / (top - bottom);
		result.matrix[2 * 4 + 2] = (2 / (farPlane - nearPlane));
		//NOTE: translation
		result.matrix[3 * 4 + 0] = -(right + left) / (right - left);
		result.matrix[3 * 4 + 1] = (top + bottom) / (top - bottom);
		result.matrix[3 * 4 + 2] = -((farPlane + nearPlane) / (farPlane - nearPlane));

		return result;
	}

	static Mat4x4 persProj(float nearPlane, float farPlane, float aspectRatio, float fov)
	{
		//Coords of the projection plane at the nearPlane
		//Could also be width, height. Then one could compute the aspectRatio, and the vfov as follows: 
		float top = tanf(utils::degreesToRadians(fov / 2.0f)) * nearPlane;
		float bottom = -top;
		float right = top * aspectRatio;
		float left = -right;

		Mat4x4 result = identity();

		//It really is (2 * near) / width, so the same as in ortho land. But one scales by near, to take that into account
		result.matrix[0 * 4 + 0] = (2 * nearPlane) / (right - left);
		result.matrix[1 * 4 + 1] = (-2 * nearPlane) / (top - bottom);
		//to normalize z between 0 and 1
		//to get to these formulars, one has to solve the following equations:
		// an + b = 0 and af + b = f, where n = near and f = far and a and b are the coefficients to solve for
		//really we want it to be = 0 and = 1, but we have to take into account that the perspective divide comes later, and then 0/n = 0 and f/f = 1 and in between there is a not linear curve (because divide)
		result.matrix[2 * 4 + 2] = (farPlane / (farPlane - nearPlane));

		result.matrix[3 * 4 + 0] = -(right + left) / (right - left);
		result.matrix[3 * 4 + 1] = (top + bottom) / (top - bottom);
		//Other solved equation
		result.matrix[3 * 4 + 2] = -((farPlane * nearPlane) / (farPlane - nearPlane));

		//to get z in w for perspective divide
		result.matrix[2 * 4 + 3] = 1;
		result.matrix[3 * 4 + 3] = 0;

		return result;
	}

	static Mat4x4 lookAt(const Vector3f& from, const Vector3f& to)
	{
		Vector3f forward = (to - from).getNormalized();
		Vector3f up = { 0.0f, 1.0f, 0.0f };
		Vector3f right = up.crossProduct(forward).getNormalized();
		up = forward.crossProduct(right).getNormalized();

		Mat4x4 result = Mat4x4::identity();
		// bases conversion: first column x, second y and third z. Forword should point in z, so it is the third column
		result.matrix[0 * 4 + 0] = right.x;
		result.matrix[1 * 4 + 0] = right.y;
		result.matrix[2 * 4 + 0] = right.z;
		result.matrix[0 * 4 + 1] = up.x;
		result.matrix[1 * 4 + 1] = up.y;
		result.matrix[2 * 4 + 1] = up.z;
		result.matrix[0 * 4 + 2] = forward.x;
		result.matrix[1 * 4 + 2] = forward.y;
		result.matrix[2 * 4 + 2] = forward.z;
		result.matrix[3 * 4 + 0] = right.dotProduct(from);
		result.matrix[3 * 4 + 1] = up.dotProduct(from);
		result.matrix[3 * 4 + 2] = forward.dotProduct(from);

		return result;
	}
};