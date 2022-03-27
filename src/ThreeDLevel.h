#pragma once

#include "Level.h"
#include "FireTrigger.h"
#include "NetworkPacket.h"

class ThreeDLevel: public Level
{
	FireTriggers fireTriggers;
	const String lobbyName;
	const String username;
	NetworkPacket networkPacket;
private:
	void eventLevelReloadHandler(EventData* eventData);
public:
	ThreeDLevel(const String& lobbyName, const String& username);
	void init() override;
};