#include "Utils.h"
#include "Types.h"

static uint32_t counter = 0;

uint32_t utils::getGUID()
{
	return counter++;
}

float utils::lerp(float v0, float v1, float t)
{
	return (1 - t) * v0 + t * v1;
}

float utils::degreesToRadians(float degree)
{
	float radians = PiOver180 * degree;
	return radians;
}

float utils::radiansToDegrees(float radians)
{
	float degrees = _180OverPi * radians;
	return degrees;
}
