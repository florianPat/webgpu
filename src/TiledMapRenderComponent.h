#pragma once

#include "Component.h"
#include "Sprite.h"
#include "Window.h"
#include "Utils.h"

class TiledMapRenderComponent : public Component
{
	const Sprite sprite;
	Graphics2D& gfx;
public:
	TiledMapRenderComponent(Sprite& sprite, Graphics2D& gfx)
		: Component(utils::getGUID()), sprite(sprite),
		  gfx(gfx)
		  {}
	void render() override { gfx.draw(sprite); }
};