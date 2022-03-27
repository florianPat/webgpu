#include "GuiInputFormComponent.h"
#include "Utils.h"

void GuiInputFormComponent::transitionToNextStep()
{
	if (currentStepIndex < (guiInputFormDefinition.getSteps().size() - 1))
	{
		prepareFadeOutForStep(guiInputFormDefinition.getSteps()[currentStepIndex]);
		transitioningToNextStep = true;
	}
	else
	{
		if (guiInputFormDefinition.getFinishHandler())
		{
			(guiInputFormDefinition.getFinishHandler())();
		}
		ended = true;
	}
}

void GuiInputFormComponent::prepareFadeOutForStep(GuiInputFormDefinition::Step& step)
{
	for (GuiInputFormDefinition::Widget& widget : step.getWidgets())
	{
		widget.animationPercentage = 0.0f;
		if (widget.animationBitmask & (uint32_t)GUI::GuiAnimation::FADE_IN)
		{
			widget.animationBitmask &= ~(uint32_t)GUI::GuiAnimation::FADE_IN;
			widget.animationBitmask |= (uint32_t)GUI::GuiAnimation::FADE_OUT;
		}
		widget.animationBitmask &= (uint32_t)GUI::GuiAnimation::FADE_OUT;
	}
}

GuiInputFormComponent::GuiInputFormComponent(GuiInputFormDefinition&& guiInputFormDefinition, Graphics2D& gfx, Keyboard& keyboard, const Mouse& mouse)
	: Component(utils::getGUID()), guiInputFormDefinition(guiInputFormDefinition), gui(gfx, keyboard, mouse)
{
}

void GuiInputFormComponent::update(float dt, Actor* owner)
{
	if (ended)
	{
		return;
	}

	assert(currentStepIndex < guiInputFormDefinition.getSteps().size());

	float animationDelta = dt / GuiInputFormDefinition::GUI_ANIMATION_DURATION;

	GuiInputFormDefinition::Step& currentStep = guiInputFormDefinition.getSteps()[currentStepIndex];
	for (GuiInputFormDefinition::Widget& widget : currentStep.getWidgets())
	{
		widget.animationPercentage += animationDelta;
		if (widget.animationPercentage > 1.0f)
		{
			widget.animationPercentage = 1.0f;

			if (transitioningToNextStep)
			{
				transitioningToNextStep = false;
				++currentStepIndex;
			}
		}
	}

	for (GuiInputFormDefinition::Widget& widget : currentStep.getWidgets())
	{
		switch (widget.type)
		{
			case GUI::Type::LABEL:
			{
				gui.label(widget.text, widget.textPos, GuiInputFormDefinition::FONT_SIZE, GUI::AnchorPresets::bottomMiddle, Vector2f{ 0.5f, 0.5f }, widget.textColor, widget.backgroundColor, widget.animationBitmask, widget.animationPercentage);
				break;
			}
			case GUI::Type::INPUT:
			{
				if (Globals::window->getKeyboard().isKeyPressed(VK_RETURN))
				{
					if (widget.constraints & (uint32_t)GuiInputFormDefinition::Constraint::NOT_EMPTY)
					{
						if (widget.text.empty())
						{
							// TODO: Add an not empty indicator!
							continue;
						}
					}

					widget.handler.inputHandler(widget.text);
					transitionToNextStep();
				}

				Vector2f textDimension = gui.computeTextDimension(widget.text, GuiInputFormDefinition::FONT_SIZE);
				gui.input(widget.text, FloatRect(widget.textPos.x, widget.textPos.y, textDimension.x, textDimension.y), GUI::AnchorPresets::bottomMiddle,
					Vector2f{ 0.5f, 0.5f }, widget.textColor, widget.backgroundColor, widget.activeWidget, widget.maxChars, widget.animationBitmask,
					widget.animationPercentage);
				break;
			}
			case GUI::Type::BUTTON:
			{
				bool pressed = gui.button(widget.text, widget.textPos, GuiInputFormDefinition::FONT_SIZE, Vector2f{ 5.0f, 5.0f }, GUI::AnchorPresets::bottomMiddle,
					Vector2f{ 0.5f, 0.5f }, widget.textColor, widget.backgroundColor, widget.animationBitmask, widget.animationPercentage);

				if (pressed)
				{
					widget.handler.buttonHandler();
					transitionToNextStep();
				}
				break;
			}
			default:
			{
				InvalidCodePath;
				break;
			}
		}
	}
}

void GuiInputFormComponent::render()
{
	if (ended)
	{
		return;
	}

	gui.submit();
}
