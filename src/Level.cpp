#include "Level.h"
#include "Physics.h"
#include "Globals.h"
#include "EventChangeLevel.h"

void Level::eventChangeLevelHandler(EventData* eventData)
{
	EventChangeLevel* eventChangeLevel = (EventChangeLevel*) eventData;

	// TODO: Fix audio looping on new level load
	Globals::window->getAudio().clear();
	Globals::window->getAudio().update();

	newLevel = std::move(eventChangeLevel->newLevel);
	endLevel = true;
}

Level::Level()
	:	window(*Globals::window), clock(window.getClock()), physics(), eventManager(), gom(),
		gfx(window.getGfx())
{
}

Level::Level(uint32_t gomRenderActorSize)
		:	window(*Globals::window), clock(window.getClock()), physics(), eventManager(),
			gom(gomRenderActorSize), gfx(window.getGfx())
{
}

UniquePtr<Level> Level::getNewLevel()
{
    endLevel = false;
	auto gfx2D = gfx.getRenderer<Graphics2D>(); 
	gfx2D->getDefaultView().setCenter(gfx2D->getDefaultView().getSize().x / 2.0f, gfx2D->getDefaultView().getSize().y / 2.0f);
    //TODO: Set zoom and rotation of defaultView to default (if that is implemented ;))!
	return std::move(newLevel);
}

bool Level::shouldEndLevel() const
{
	return endLevel;
}

void Level::Go()
{
	float dt = clock.getTime().asSeconds();

	uint32_t targetFPS = 100;
	float targetDt = 1.0f / targetFPS;
	if (dt < targetDt)
	{
		uint32_t sleepMilliseconds = (uint32_t)((targetDt - dt) * 1000.0f);
		Sleep(sleepMilliseconds);
	}
	dt += clock.getTime().asSeconds();
	float fps = 1.0f / dt;
	char fpsString[64];
	sprintf_s(fpsString, sizeof(fpsString), "Zombie Modus | V. 0.0.8 | FPS: %3.0f", fps);
	window.setWindowText(fpsString);

	window.getAudio().update();

	gfx.clear();

	gom.updateAndDrawActors(dt);

	gfx.render();
}

void Level::setup()
{
#ifdef DEBUG
	Globals::window->getNativeThreadQueue().resetStartedFlushing();
#endif
	Globals::window->getNativeThreadQueue().setNextWriteIndex(0);
	Globals::window->getAudio().clear();

	Globals::eventManager = &eventManager;
	eventManager.addClearTriggerAndAddListenerEventsJob();
	init();
	eventManager.addListener(EventChangeLevel::eventId, GET_DELEGATE_FROM(Level, &Level::eventChangeLevelHandler));
	window.callToGetEventJobsBeginning();
	clock.restart();
}
