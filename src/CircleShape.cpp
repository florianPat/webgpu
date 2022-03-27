#include "CircleShape.h"

Mat4x4 CircleShape::getTransform() const
{
	Mat4x4 result = Mat4x4::identity();

	result *= Mat4x4::translate(pos);

	return result;
}
