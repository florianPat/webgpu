#pragma once

#include "GUI.h"
#include "Delegate.h"
#include <variant>

struct GuiInputFormDefinition
{
	static constexpr int32_t FONT_SIZE = 64;
	static constexpr float FONT_PADDING = 16;
	static constexpr float GUI_ANIMATION_DURATION = 2.0f;
public:
	enum class Constraint
	{
		NONE = 1 << 0,
		NOT_EMPTY = 1 << 1,
	};
	struct Widget
	{
		String text = "";
		GUI::Type type = GUI::Type::LABEL;
		Vector2f textPos;
		Color textColor;
		Color backgroundColor;
		uint32_t animationBitmask = 0;
		float animationPercentage = 0.0f;
		bool activeWidget = true; // TODO: Handle for multiple input fields!
		uint32_t maxChars = 0;
		uint32_t constraints = (uint32_t)Constraint::NONE;
		union Handler {
			Delegate<void()> buttonHandler;
			Delegate<void(const String&)> inputHandler;
		public:
			Handler();
		} handler;
	};
	class Step
	{
		Vector<Widget> widgets;
	public:
		void addHeader(const String& text);
		void addLabel(const String& text);
		void addInput(const Delegate<void(const String&)>& handler, uint32_t maxChars, uint32_t constraints = (uint32_t)Constraint::NONE);
		void addButton(const String& text, const Delegate<void()>& handler);
	public:
		Vector2f getNextWidgetPosForNextRow();
		Vector2f getLabelPosForNextRow();
		Vector2f getInputPosForCurrentRow();
		Vector<Widget>& getWidgets();
	};
private:
	Vector<Step> steps;
	Delegate<void()> finishHandler;
public:
	GuiInputFormDefinition();
	void addStep(Step&& step);
	void addFinishDelegate(Delegate<void()>&& finishHandler);
	Vector<Step>& getSteps();
	Delegate<void()> getFinishHandler();
};