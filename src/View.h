#pragma once

#include "Vector2.h"
#include "Mat4x4.h"

class View
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t viewportWidth = 0, viewportHeight = 0;
	Vector2f center = { 0.0f, 0.0f };
	bool shouldUpdate = false;
	//TODO: Implement
	/*float rot = 0.0f;
	float scl = 1.0f;*/
public:
	enum class ViewportType
	{
		FIT,
		EXTEND
	};
public:
	View() = default;
	View(uint32_t renderWidth, uint32_t renderHeight, uint32_t screenWidth, uint32_t screenHeight, View::ViewportType viewportType);
	Mat4x4 getOrthoProj(const Vector2f scale = { 1.0f, 1.0f }) const;
	bool updated();
	void setCenter(const Vector2f& center);
	void setCenter(float x, float y);
	void setSize(const Vector2i& size);
	void setSize(uint32_t width, uint32_t height);
	Vector2ui getSize() const;
	Vector2ui getViewportSize() const;
	const Vector2f& getCenter() const;
	explicit operator bool () const { return (viewportWidth != 0); }
};