#include "GuiInputFormDefinition.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

GuiInputFormDefinition::GuiInputFormDefinition() : steps(), finishHandler(Delegate<void()>::empty())
{
}

void GuiInputFormDefinition::addStep(Step&& step)
{
	steps.push_back(std::move(step));
}

void GuiInputFormDefinition::addFinishDelegate(Delegate<void()>&& finishHandlerIn)
{
	finishHandler = finishHandlerIn;
}

Vector<GuiInputFormDefinition::Step>& GuiInputFormDefinition::getSteps()
{
	return steps;
}

Delegate<void()> GuiInputFormDefinition::getFinishHandler()
{
	return finishHandler;
}

Vector2f GuiInputFormDefinition::Step::getNextWidgetPosForNextRow()
{
	if (widgets.empty())
	{
		return Vector2f(GUI::GUI_RESOLUTION_X / 2.0f, GUI::GUI_RESOLUTION_Y - (FONT_SIZE + GUI::GUI_RESOLUTION_Y / 3.0f));
	}

	Vector2f lastWidgetPos = widgets.back().textPos;
	lastWidgetPos.x = GUI::GUI_RESOLUTION_X / 2.0f;
	lastWidgetPos.y -= FONT_SIZE + FONT_PADDING;
	return lastWidgetPos;
}

Vector2f GuiInputFormDefinition::Step::getLabelPosForNextRow()
{
	Vector2f result = getNextWidgetPosForNextRow();

	result.x = GUI::GUI_RESOLUTION_X * 0.30f;
	return result;
}

Vector2f GuiInputFormDefinition::Step::getInputPosForCurrentRow()
{
	assert(!widgets.empty());
	assert(widgets.back().type == GUI::Type::LABEL);
	Vector2f result = widgets.back().textPos;
	result.x = GUI::GUI_RESOLUTION_X * 0.65f;
	return result;
}

void GuiInputFormDefinition::Step::addHeader(const String& text)
{
	Widget widget;
	new (&widget.text) String(text);
	widget.backgroundColor = Color(1.0f, 1.0f, 1.0f, 0.5f);
	widget.textColor = Colors::Red;
	widget.type = GUI::Type::LABEL;
	widget.animationBitmask = (uint32_t)GUI::GuiAnimation::FADE_IN | (uint32_t)GUI::GuiAnimation::SLIDE_FROM_TOP;
	widget.textPos = getNextWidgetPosForNextRow();
	widgets.push_back(std::move(widget));
}

void GuiInputFormDefinition::Step::addLabel(const String& text)
{
	Widget widget;
	new (&widget.text) String(text);
	widget.backgroundColor = Colors::Transparent;
	widget.textColor = Colors::White;
	widget.type = GUI::Type::LABEL;
	widget.animationBitmask = (uint32_t)GUI::GuiAnimation::FADE_IN | (uint32_t)GUI::GuiAnimation::SLIDE_FROM_LEFT;
	widget.textPos = getLabelPosForNextRow();
	widgets.push_back(std::move(widget));
}

void GuiInputFormDefinition::Step::addInput(const Delegate<void(const String&)>& handler, uint32_t maxChars, uint32_t constraints)
{
	Widget widget;
	if (maxChars > String::SHORT_STRING_SIZE)
	{
		new (&widget.text) LongString();
	}
	widget.backgroundColor = Color(0.1f, 0.1f, 0.1f, 0.4f);
	widget.textColor = Colors::White;
	widget.type = GUI::Type::INPUT;
	widget.animationBitmask = (uint32_t)GUI::GuiAnimation::FADE_IN | (uint32_t)GUI::GuiAnimation::SLIDE_FROM_RIGHT;
	widget.textPos = getInputPosForCurrentRow();
	widget.handler = Widget::Handler{};
	widget.handler.inputHandler = handler;
	widget.maxChars = maxChars;
	widget.constraints = constraints;
	widgets.push_back(std::move(widget));
}

void GuiInputFormDefinition::Step::addButton(const String& text, const Delegate<void()>& handler)
{
	Widget widget;
	new (&widget.text) String(text);
	widget.backgroundColor = Colors::White;
	widget.textColor = Colors::Black;
	widget.type = GUI::Type::BUTTON;
	widget.animationBitmask = (uint32_t)GUI::GuiAnimation::FADE_IN | (uint32_t)GUI::GuiAnimation::SLIDE_FROM_BOTTOM;
	widget.textPos = getNextWidgetPosForNextRow();
	widget.handler = Widget::Handler{};
	widget.handler.buttonHandler = handler;
	widgets.push_back(std::move(widget));
}

Vector<GuiInputFormDefinition::Widget>& GuiInputFormDefinition::Step::getWidgets()
{
	return widgets;
}

GuiInputFormDefinition::Widget::Handler::Handler() : buttonHandler(Delegate<void()>::empty())
{
}
