#include "IntroComponent.h"
#include "Globals.h"
#include "Window.h"
#include "PlayerComponent.h"
#include "Audio.h"

IntroComponent::IntroComponent() : Component(utils::getGUID()), renderer2d(Globals::window->getGfx().getRenderer<Graphics2D>())
{
	Globals::window->getGfx().getRenderer<Graphics3D>()->camera.rot = Mat4x4::rotateX(-PIf / 2.0f);
	Globals::window->getGfx().getRenderer<Graphics3D>()->camera.pos = -Vector3f{ 0.0f, (float)PlayerComponent::SPECTATOR_POSITION_Y, 0.0f };
	Globals::window->getGfx().getRenderer<Graphics3D>()->camera.update();

	introMusic = Globals::window->getAssetManager()->getOrAddRes<Sound>("sound/intro.wav");
	logoMusic = Globals::window->getAssetManager()->getOrAddRes<Sound>("sound/logo.wav");
	Globals::window->getAudio().start(logoMusic);

	logoTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("images/logo.png");
	new (&logoSprite) Sprite(logoTexture);
	logoSprite.pos = Vector2f{ Globals::window->getGfx().renderWidth / 2.0f, Globals::window->getGfx().renderHeight / 2.0f };
	logoSprite.org = Vector2f(0.5f, 0.5f);
}

IntroComponent::~IntroComponent()
{
	logoSprite.~Sprite();
	Globals::window->getAssetManager()->unloadNotUsedRes("sound/intro.wav");
	Globals::window->getAssetManager()->unloadNotUsedRes("sound/logo.wav");
	Globals::window->getAssetManager()->unloadNotUsedRes("images/logo.png");
}

void IntroComponent::update(float dt, Actor* owner)
{
}

void IntroComponent::render()
{
	if (logoMusic->isFinished() && introMusic->isFinished())
	{
		Globals::window->getAudio().start(introMusic);
	}
	else if (!logoMusic->isFinished())
	{
		renderer2d->flush();
		Globals::window->getGfx().clear();
		renderer2d->draw(logoSprite);
	}
}
