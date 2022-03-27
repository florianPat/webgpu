#pragma once

#include "Level.h"
#include "FireTrigger.h"
#include "NetworkPacket.h"

class MenuLevel : public Level
{
	uint32_t usernameStepId = 0;
	uint32_t lobbyStepId = 0;
	String username = "";
	String lobbyId = "";
private:
	void eventLevelReloadHandler(EventData* eventData);
	void guiInputFromDefinitionFinishedHandler();
	void usernameInputHandler(const String& username);
	void lobbyIdInputHandler(const String& lobbyId);
	void newLobbyButtonPressHandler();
	void addPeerDisplayActor();
public:
	MenuLevel();
	void init() override;
};