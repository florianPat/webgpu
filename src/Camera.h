#pragma once

#include "Mat4x4.h"

class Camera
{
	Mat4x4 cachedMatrix;
public:
	Mat4x4 persProj;
	Mat4x4 cameraTransform;
	Vector3f pos;
	Mat4x4 rot;
public:
	Camera(Mat4x4&& persProj);
	Mat4x4& getViewPersMat();
	void update();
	void translateByOrientation(const Vector3f& translation);
};