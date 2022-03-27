#pragma once

#include "Component.h"
#include "GuiInputFormDefinition.h"

class GuiInputFormComponent : public Component
{
	GuiInputFormDefinition guiInputFormDefinition;
	GUI gui;
	uint32_t currentStepIndex = 0;
	bool transitioningToNextStep = false;
	bool ended = false;
private:
	void transitionToNextStep();
	void prepareFadeOutForStep(GuiInputFormDefinition::Step& step);
public:
	// TODO: Add font size and padding as arguemnts
	GuiInputFormComponent(GuiInputFormDefinition&& guiInputFormDefinition, Graphics2D& gfx, Keyboard& keyboard, const Mouse& mouse);
	void update(float dt, Actor* owner) override;
	void render() override;
};