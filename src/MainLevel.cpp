#include "MainLevel.h"
#include "EventLevelReload.h"
#include "TiledMapComponent.h"
#include "Delegate.h"

void MainLevel::eventLevelReloadHandler(EventData * eventData)
{
	newLevel = makeUnique<MainLevel>(tiledMapName);
	endLevel = true;
}

MainLevel::MainLevel(const String& tiledMapName) : Level(), tiledMapName(tiledMapName)
{
}

void MainLevel::init()
{
	gom.addRenderComponent<TiledMapComponent>(0, tiledMapName);

	eventManager.addListener(EventLevelReload::eventId, Delegate<void(EventData*)>::from<MainLevel, &MainLevel::eventLevelReloadHandler>(this));
}
