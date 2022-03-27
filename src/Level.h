#pragma once

#include "Window.h"
#include "Physics.h"
#include "GameObjectManager.h"
#include "EventManager.h"
#include "Utils.h"
#include "Clock.h"
#include "NativeThreadQueue.h"

class Level
{
protected:
    Window& window;
	Clock& clock;
	Physics physics;
	EventManager eventManager;
    GameObjectManager gom;
    GraphicsIniter& gfx;

	UniquePtr<Level> newLevel = nullptr;
	bool endLevel = false;
private:
	virtual void init() = 0;
	void eventChangeLevelHandler(EventData* eventData);
public:
	Level();
	Level(uint32_t gomRenderActorSize);
	void setup();
	virtual ~Level() = default;
    UniquePtr<Level> getNewLevel();
    bool shouldEndLevel() const;
	void Go();
};