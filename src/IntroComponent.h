#pragma once

#include "Component.h"
#include "Sound.h"
#include "Texture.h"
#include "Graphics.h"

class IntroComponent : public Component
{
	Sound* introMusic = nullptr;
	Sound* logoMusic = nullptr;
	Texture* logoTexture = nullptr;
	Sprite logoSprite;
	Graphics2D* renderer2d;
public:
	IntroComponent();
	~IntroComponent();
	void update(float dt, Actor* owner) override;
	void render() override;
};