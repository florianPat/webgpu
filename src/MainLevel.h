#pragma once

#include "Level.h"
#include "TiledMap.h"

class MainLevel: public Level
{
	String tiledMapName;
private:
	void eventLevelReloadHandler(EventData* eventData);
public:
	MainLevel(const String& tiledMapName);
	void init() override;
};