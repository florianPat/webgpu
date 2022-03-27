#pragma once

#include "String.h"
#include "Rect.h"
#include "Graphics.h"
#include "Globals.h"
#include "Window.h"
#include "Font.h"

class GUI
{
	static constexpr char BACKSPACE = 8;
	static constexpr float SLIDE_OFFSET = 100.0f;
public:
	enum class Type
	{
		BUTTON,
		LABEL,
		INPUT,
		GROUP,
	};
	enum class GuiAnimation
	{
		NONE = 1 << 0,
		FADE_IN = 1 << 1,
		FADE_OUT = 1 << 2,
		SLIDE_FROM_LEFT = 1 << 3,
		SLIDE_FROM_RIGHT = 1 << 4,
		SLIDE_FROM_TOP = 1 << 5,
		SLIDE_FROM_BOTTOM = 1 << 6,
	};
private:
	struct DrawInformation
	{
		Type type;
		String text;
		FloatRect rect;
		Color color;
		Color textColor;
		FloatRect anchor;
		Vector2f center;
		uint32_t animationType;
		float animationPercentage;
		uint32_t parent = (uint32_t)-1;
		bool activeWidget = false;
		FloatRect drawingRect;
	};
	Vector<DrawInformation> drawInformation;
	Vector<uint32_t> drawInformationFreeList;
	Graphics2D& gfx;
	Keyboard& keyboard;
	const Mouse& mouse;
	Font* font = nullptr;
	Vector2f mousePos = Vector2f{ 0.0f, 0.0f };
private:
	FloatRect computeDrawingRect(const DrawInformation& info);
	float computeAnimationPercentage(const DrawInformation& info);
	FloatRect computeAnimationDrawingRect(const FloatRect& rect, float animationPercentage, uint32_t animationType);
	Color computeAnimationColor(const Color& color, float animationPercentage, uint32_t animationType);
	inline void computeMousePos();
	void setupDrawingInfoVars(DrawInformation& info);
public:
	static constexpr float GUI_RESOLUTION_X = 1280;
	static constexpr float GUI_RESOLUTION_Y = 720;
	struct AnchorPresets
	{
		static inline const FloatRect topLeft = FloatRect(0.0f, 1.0f, 0.0f, 0.0f);
		static inline const FloatRect topMiddle = FloatRect(0.5f, 1.0f, 0.0f, 0.0f);
		static inline const FloatRect topRight = FloatRect(1.0f, 1.0f, 0.0f, 0.0f);
		static inline const FloatRect middleLeft = FloatRect(0.0f, 0.5f, 0.0f, 0.0f);
		static inline const FloatRect center = FloatRect(0.5f, 0.5f, 0.0f, 0.0f);
		static inline const FloatRect middleRight = FloatRect(1.0f, 0.5f, 0.0f, 0.0f);
		static inline const FloatRect bottomLeft = FloatRect(0.0f, 0.0f, 0.0f, 0.0f);
		static inline const FloatRect bottomMiddle = FloatRect(0.5f, 0.0f, 0.0f, 0.0f);
		static inline const FloatRect bottomRight = FloatRect(1.0f, 0.0f, 0.0f, 0.0f);
		static inline const FloatRect rowTop = FloatRect(0.0f, 1.0f, 1.0f, 0.0f);
		static inline const FloatRect rowMiddle = FloatRect(0.0f, 0.5f, 1.0f, 0.0f);
		static inline const FloatRect rowBottom = FloatRect(0.0f, 0.0f, 1.0f, 0.0f);
		static inline const FloatRect columnLeft = FloatRect(0.0f, 0.0f, 0.0f, 1.0f);
		static inline const FloatRect columnMiddle = FloatRect(0.5f, 0.0f, 0.0f, 1.0f);
		static inline const FloatRect columnRight = FloatRect(1.0f, 0.0f, 0.0f, 1.0f);
	};
public:
	GUI(Graphics2D& gfx, Keyboard& keyboard, const Mouse& mouse);
	bool button(const String& text, const Vector2f& pos, int32_t pixelSize, const Vector2f& padding, const FloatRect& anchor, const Vector2f& center, Color textColor,
		Color backgroundColor, uint32_t animationType = (uint32_t)GuiAnimation::NONE, float animationPercentage = 1.0f, uint32_t parentGroup = (uint32_t)-1);
	void label(const String& text, const Vector2f& pos, int32_t pixelSize, const FloatRect& anchor, const Vector2f& center, Color textColor, Color backgroundColor, uint32_t animationType = (uint32_t)GuiAnimation::NONE, float animationPercentage = 1.0f, uint32_t parentGroup = (uint32_t)-1);
	bool input(String& text, const FloatRect& rect, const FloatRect& anchor, const Vector2f& center, Color textColor, Color backgroundColor, bool activeWidget,
		uint32_t maxChars, uint32_t animationType = (uint32_t)GuiAnimation::NONE, float animationPercentage = 1.0f, uint32_t parentGroup = (uint32_t)-1);
	uint32_t group(const FloatRect& rect, const FloatRect& anchor, const Vector2f& center, uint32_t parentGroup = (uint32_t)-1);
	void submit();
	Vector2f computeTextDimension(const String& text, int32_t pixelSize);
};