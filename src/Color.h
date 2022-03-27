#pragma once

#include "Types.h"

//NOTE: The color is saved in premultiplied alpha "color space"
class Color
{
public:
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
public:
	Color() = default;
	Color(float r, float g, float b, float a = 1.0f) : r(r * a), g(g * a), b(b * a), a(a) {}

	bool operator==(const Color& rhs)
	{
		return ((r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a));
	}
	bool operator!=(const Color& rhs)
	{
		return ((r != rhs.r) && (g != rhs.g) && (b != rhs.b) && (a != rhs.a));
	}
	
	//Clamp!
	/*Color operator+(const Color& rhs)
	{
		return Color(toInteger() + rhs.toInteger());
	}
	Color& operator+=(const Color& rhs)
	{
		*this = Color(toInteger() + rhs.toInteger());
		return *this;
	}

	Color operator-(const Color& rhs)
	{
		return Color(toInteger() - rhs.toInteger());
	}
	Color& operator-=(const Color& rhs)
	{
		*this = Color(toInteger() - rhs.toInteger());
		return *this;
	}

	Color operator*(const Color& rhs)
	{
		return Color(toInteger() * rhs.toInteger());
	}
	Color& operator*=(const Color& rhs)
	{
		*this = Color(toInteger() * rhs.toInteger());
		return *this;
	}*/
};

namespace Colors
{
	static const Color Black = Color();
	static const Color White = Color(1.0f, 1.0f, 1.0f);
	static const Color Red = Color(1.0f, 0.0f, 0.0f);
	static const Color Green = Color(0.0f, 1.0f, 0.0f);
	static const Color Blue = Color(0.0f, 0.0f, 1.0f);
	static const Color Yellow = Color(1.0f, 1.0f, 0.0f);
	static const Color Magenta = Color(1.0f, 0.0f, 1.0f);
	static const Color Cyan = Color(0.0f, 1.0f, 1.0f);
	static const Color Transparent = Color(0, 0, 0, 0);
}