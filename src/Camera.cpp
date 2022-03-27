#include "Camera.h"

Camera::Camera(Mat4x4&& persProj) : persProj(std::move(persProj))
{
	update();
}

Mat4x4& Camera::getViewPersMat()
{
	return cachedMatrix;
}

void Camera::update()
{
	Vector3f from = pos;
	Vector3f to = (rot * Vector3f{ 0.0f, 0.0f, 1.0f }).getNormalized();
	to = from + to;
	cameraTransform = Mat4x4::lookAt(from, to);
	cachedMatrix = persProj * cameraTransform;
}

void Camera::translateByOrientation(const Vector3f& translation)
{
	Vector3f rotatedTranslation = rot * translation;
	pos += rotatedTranslation;
}
