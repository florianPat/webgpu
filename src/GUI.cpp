#include "GUI.h"

FloatRect GUI::computeDrawingRect(const DrawInformation& info)
{
	FloatRect result = info.rect;

	if (info.parent != (uint32_t)-1)
	{
		// TODO: The -1 does not get stored correctly in all cases due to uint32_t stuff but why?
		// result = computeDrawingRect(drawInformation[info.parent]);
	}

	result.left -= info.center.x * result.width;
	result.bottom -= info.center.y * result.height;

	Vector2ui viewSize = gfx.getDefaultView().getSize();
	float guiLeftAnchor = info.anchor.left * GUI_RESOLUTION_X;
	float guiBottomAnchor = info.anchor.bottom * GUI_RESOLUTION_Y;
	float viewLeftAnchor = info.anchor.left * (float)viewSize.x;
	float viewBottomAnchor = info.anchor.bottom * (float)viewSize.y;
	float leftDifference = result.left - guiLeftAnchor;
	float bottomDifference = result.bottom - guiBottomAnchor;
	float newLeft = viewLeftAnchor + leftDifference;
	float newBottom = viewBottomAnchor + bottomDifference;

	float guiRightAnchor = info.anchor.getRight() * GUI_RESOLUTION_X;
	float guiTopAnchor = info.anchor.getTop() * GUI_RESOLUTION_Y;
	float viewRightAnchor = info.anchor.getRight() * (float)viewSize.x;
	float viewTopAnchor = info.anchor.getTop() * (float)viewSize.y;
	float rightDifference = result.getRight() - guiRightAnchor;
	float topDifference = result.getTop() - guiTopAnchor;
	result.width = (viewRightAnchor + rightDifference) - result.left;
	result.height = (viewTopAnchor + topDifference) - result.bottom;
	
	result.left = newLeft;
	result.bottom = newBottom;

	return result;
}

GUI::GUI(Graphics2D& gfxIn, Keyboard& keyboardIn, const Mouse& mouseIn) : gfx(gfxIn), keyboard(keyboardIn), mouse(mouseIn)
{
	font = Globals::window->getAssetManager()->getOrAddRes<Font>("fonts/framd.ttf", 64);
}

bool GUI::button(const String& text, const Vector2f& pos, int32_t pixelSize, const Vector2f& padding, const FloatRect& anchor, const Vector2f& center, Color textColor,
	Color backgroundColor, uint32_t animationType, float animationPercentage, uint32_t parentGroup)
{
	DrawInformation info = {Type::BUTTON, text};
	info.rect.left = pos.x;
	info.rect.bottom = pos.y;
	Vector2f textDimension = computeTextDimension(text, pixelSize);
	info.rect.width = textDimension.x;
	info.rect.height = (float)pixelSize;
	info.textColor = textColor;
	info.color = backgroundColor;
	info.anchor = anchor;
	info.center = center;
	info.parent = parentGroup;
	info.animationType = animationType;
	info.animationPercentage = animationPercentage;
	setupDrawingInfoVars(info);
	drawInformation.push_back(std::move(info));

	computeMousePos();
	// TODO: Take into account the clicking, so just return true on mouse button release
	if (mouse.isButtonReleased(Mouse::Button::left) && info.drawingRect.contains(mousePos))
	{
		return true;
	}
	return false;
}

void GUI::label(const String& text, const Vector2f& pos, int32_t pixelSize, const FloatRect& anchor, const Vector2f& center, Color textColor, Color backgroundColor,
	uint32_t animationType, float animationPercentage, uint32_t parentGroup)
{
	DrawInformation info = {Type::LABEL, text};
	info.rect.left = pos.x;
	info.rect.bottom = pos.y;
	Vector2f textDimension = computeTextDimension(text, pixelSize);
	info.rect.width = textDimension.x;
	info.rect.height = (float)pixelSize;
	info.color = backgroundColor;
	info.textColor = textColor;
	info.anchor = anchor;
	info.center = center;
	info.animationType = animationType;
	info.animationPercentage = animationPercentage;
	info.parent = parentGroup;
	setupDrawingInfoVars(info);
	drawInformation.push_back(std::move(info));
}

bool GUI::input(String& text, const FloatRect& rect, const FloatRect& anchor, const Vector2f& center, Color textColor, Color backgroundColor, bool activeWidget,
	uint32_t maxChars, uint32_t animationType, float animationPercentage, uint32_t parentGroup)
{
	DrawInformation info = { Type::INPUT, text };
	info.rect = rect;
	info.textColor = textColor;
	info.color = backgroundColor;
	info.anchor = anchor;
	info.center = center;
	info.animationType = animationType;
	info.animationPercentage = animationPercentage;
	info.parent = parentGroup;
	info.activeWidget = activeWidget;
	setupDrawingInfoVars(info);
	drawInformation.push_back(std::move(info));

	/*if (mouse.isButtonPressed(Mouse::Button::left))
	{
		computeMousePos();

		if (info.drawingRect.contains(mousePos))
		{
			activeWidget = true;
		}
		else
		{
			activeWidget = false;
		}
	}*/
	
	if (activeWidget && animationPercentage > 0.95f)
	{
		if (keyboard.isKeyPressed(VK_CONTROL) && keyboard.isKeyPressed('V'))
		{
			String clipboardText = Globals::window->pasteTextFromClipboard();
			for (auto it = clipboardText.begin(); it != clipboardText.end(); ++it)
			{
				if (text.size() >= maxChars)
				{
					break;
				}

				if (*it >= ' ' && *it <= '~')
				{
					text += *it;
				}
			}
		}

		while (keyboard.hasNextCharacterInQueue())
		{
			char nextChar = keyboard.getNextCharacterFromQueue();

			if (nextChar >= ' ' && nextChar <= '~' && text.size() < maxChars)
			{
				text += nextChar;
			}
			else if (nextChar == BACKSPACE)
			{
				if (!text.empty())
				{
					text.pop_back();
				}
			}
		}
	}

	return activeWidget;
}

uint32_t GUI::group(const FloatRect& rect, const FloatRect& anchor, const Vector2f& center, uint32_t parentGroup)
{
	DrawInformation info = {Type::GROUP, ""};
	info.rect = rect;
	info.anchor = anchor;
	info.center = center;
	info.parent = parentGroup;
	setupDrawingInfoVars(info);
	drawInformation.push_back(std::move(info));
	return drawInformation.size() - 1;
}

void GUI::submit()
{
	mousePos = Vector2f{ 0.0f, 0.0f };

	for (auto it = drawInformation.begin(); it != drawInformation.end(); ++it)
	{
		float animationPercentage = computeAnimationPercentage(*it);
		Color animationColor = computeAnimationColor(it->color, animationPercentage, it->animationType);

		RectangleShape shape(Vector2f{ it->drawingRect.left, it->drawingRect.bottom }, Vector2f{ it->drawingRect.width, it->drawingRect.height }, animationColor);
		gfx.draw(shape);

		if (it->type == Type::INPUT && it->activeWidget)
		{
			Vector2f textDimension = computeTextDimension(it->text, (int32_t)it->drawingRect.height);
			Color cursorColor = computeAnimationColor(Colors::White, animationPercentage, it->animationType);
			RectangleShape cursor = RectangleShape(Vector2f{ it->drawingRect.left + textDimension.x, it->drawingRect.bottom }, Vector2f{ 5, it->drawingRect.height }, cursorColor);
			gfx.draw(cursor);
		}
	}

	for (auto it = drawInformation.begin(); it != drawInformation.end(); ++it)
	{
		float animationPercentage = computeAnimationPercentage(*it);

		Color textColor = computeAnimationColor(it->textColor, animationPercentage, it->animationType);
		font->drawText(it->text, Vector2f{ it->drawingRect.left, it->drawingRect.bottom }, (int32_t)it->drawingRect.height, textColor);
	}

	drawInformation.clear();

	while (keyboard.hasNextCharacterInQueue())
	{
		keyboard.getNextCharacterFromQueue();
	}
}

Vector2f GUI::computeTextDimension(const String& text, int32_t pixelSize)
{
	return font->computeTextDimension(text, pixelSize);
}

float GUI::computeAnimationPercentage(const DrawInformation& info)
{
	if (info.animationType & (uint32_t)GuiAnimation::FADE_IN ||
		info.animationType & (uint32_t)GuiAnimation::FADE_OUT ||
		info.animationType & (uint32_t)GuiAnimation::SLIDE_FROM_LEFT ||
		info.animationType & (uint32_t)GuiAnimation::SLIDE_FROM_BOTTOM ||
		info.animationType & (uint32_t)GuiAnimation::SLIDE_FROM_RIGHT ||
		info.animationType & (uint32_t)GuiAnimation::SLIDE_FROM_TOP)
	{
		return info.animationPercentage;
	}

	if (info.animationType & (uint32_t)GuiAnimation::NONE)
	{
		return 1.0f;
	}

	InvalidCodePath;
	return 0.0f;
}

FloatRect GUI::computeAnimationDrawingRect(const FloatRect& rect, float animationPercentage, uint32_t animationType)
{
	Vector2ui viewSize = gfx.getDefaultView().getSize();
	FloatRect result = rect;
	float newSlidePos = SLIDE_OFFSET / GUI_RESOLUTION_X * viewSize.x * (1.0f - animationPercentage);

	if (animationType & (uint32_t)GuiAnimation::SLIDE_FROM_LEFT)
	{
		result.left -= newSlidePos;
		return result;
	}

	if (animationType & (uint32_t)GuiAnimation::SLIDE_FROM_BOTTOM)
	{
		result.bottom -= newSlidePos;
		return result;
	}

	if (animationType & (uint32_t)GuiAnimation::SLIDE_FROM_RIGHT)
	{
		result.left += newSlidePos;
		return result;
	}

	if (animationType & (uint32_t)GuiAnimation::SLIDE_FROM_TOP)
	{
		result.bottom += newSlidePos;
		return result;
	}

	if (animationType & (uint32_t)GuiAnimation::NONE ||
		animationType & (uint32_t)GuiAnimation::FADE_IN ||
		animationType & (uint32_t)GuiAnimation::FADE_OUT)
	{
		return result;
	}

	InvalidCodePath;
	return result;
}

Color GUI::computeAnimationColor(const Color& color, float animationPercentage, uint32_t animationType)
{
	if (animationType & (uint32_t)GuiAnimation::FADE_IN)
	{
		return Color(color.r, color.g, color.b, animationPercentage);
	}

	if (animationType & (uint32_t)GuiAnimation::FADE_OUT)
	{
		return Color(color.r, color.g, color.b, (1.0f - animationPercentage));
	}

	if (animationType & (uint32_t)GuiAnimation::NONE ||
		animationType & (uint32_t)GuiAnimation::SLIDE_FROM_BOTTOM ||
		animationType & (uint32_t)GuiAnimation::SLIDE_FROM_LEFT ||
		animationType & (uint32_t)GuiAnimation::SLIDE_FROM_RIGHT ||
		animationType & (uint32_t)GuiAnimation::SLIDE_FROM_TOP)
	{
		return color;
	}

	InvalidCodePath;
	return Color();
}

inline void GUI::computeMousePos()
{
	if (mousePos.x != 0.0f && mousePos.y != 0.0f)
	{
		return;
	}

	Vector2ui viewSize = gfx.getDefaultView().getSize();

	mousePos.x = (float)mouse.pos.x  / Globals::window->getGfx().screenWidth * viewSize.x;
	mousePos.y = (float)mouse.pos.y / Globals::window->getGfx().screenHeight * viewSize.y;
}

void GUI::setupDrawingInfoVars(DrawInformation& info)
{
	float animationPercentage = computeAnimationPercentage(info);

	info.drawingRect = computeDrawingRect(info);
	info.drawingRect = computeAnimationDrawingRect(info.drawingRect, animationPercentage, info.animationType);
	// NOTE: Make sure the rect is as long as the rendered text is. For another aspect ratio, the width shrinks, but the text does not, because it should not be streched together
	if (info.text != "" || info.type == Type::INPUT)
	{
		info.drawingRect.width = info.rect.width;
	}
}
