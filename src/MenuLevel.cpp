#include "MenuLevel.h"
#include "EventLevelReload.h"
#include "Delegate.h"
#include "EventChangeLevel.h"
#include "ThreeDLevel.h"
#include "GuiInputFormDefinition.h"
#include "GuiInputFormComponent.h"
#include "Globals.h"
#include "ModelComponent.h"
#include "PlayerComponent.h"
#include "PeerComponent.h"
#include "IntroComponent.h"

void MenuLevel::eventLevelReloadHandler(EventData* eventData)
{
	newLevel = makeUnique<MenuLevel>();
	endLevel = true;
}

void MenuLevel::guiInputFromDefinitionFinishedHandler()
{
	Globals::eventManager->TriggerEvent<EventChangeLevel>(makeUnique<ThreeDLevel>(lobbyId, username));
}

void MenuLevel::usernameInputHandler(const String& usernameIn)
{
	username = usernameIn;
}

void MenuLevel::lobbyIdInputHandler(const String& lobbyIdIn)
{
	lobbyId = lobbyIdIn;
}

void MenuLevel::newLobbyButtonPressHandler()
{
	lobbyId = "";
}

void MenuLevel::addPeerDisplayActor()
{
	Actor* actor = gom.addActor(sizeof(PeerComponent) + (sizeof(uint32_t) * 1));
	Vector<PeerTransform> peerTransforms;
	PeerTransform peerTransform = {};
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution dist(-25.0f, 25.0f);
	// NOTE: Already there
	Terrain* terrain = terrain = Globals::window->getAssetManager()->getOrAddRes<Terrain>("Terrain.ter", 0, 0.0f, 0, Vector3f());
	for (uint32_t i = 0; i < 6; ++i)
	{
		float x = dist(rng), z = dist(rng);
		peerTransform.posX = (int16_t)x;
		peerTransform.posY = (int8_t)(terrain->getHeight(x, z) + 5.0f);
		peerTransform.posZ = (int16_t)z;
		peerTransform.rotY = (int16_t)(rand() % (int32_t)PIf);
		peerTransforms.push_back(std::move(peerTransform));
	}
	gom.addComponent<PeerComponent>(0, *actor, physics, std::move(peerTransforms));
}

MenuLevel::MenuLevel() : Level()
{
}

void MenuLevel::init()
{
	GuiInputFormDefinition guiInputFormDefinition;

	GuiInputFormDefinition::Step usernameStep;
	usernameStep.addHeader("Zombie Modus");
	usernameStep.addLabel("Benutzername:");
	usernameStep.addInput(Delegate<void(const String&)>::from<MenuLevel, &MenuLevel::usernameInputHandler>(this), 12,
		(uint32_t)GuiInputFormDefinition::Constraint::NOT_EMPTY);
	guiInputFormDefinition.addStep(std::move(usernameStep));
	
	GuiInputFormDefinition::Step lobbyStep;
	lobbyStep.addHeader("Zombie Modus");
	lobbyStep.addLabel("Lobby-Id:");
	lobbyStep.addInput(Delegate<void(const String&)>::from<MenuLevel, &MenuLevel::lobbyIdInputHandler>(this), 32,
		(uint32_t)GuiInputFormDefinition::Constraint::NOT_EMPTY);
	lobbyStep.addButton("Ich will der Horst sein!", Delegate<void()>::from<MenuLevel, &MenuLevel::newLobbyButtonPressHandler>(this));
	guiInputFormDefinition.addStep(std::move(lobbyStep));

	guiInputFormDefinition.addFinishDelegate(Delegate<void()>::from<MenuLevel,
		&MenuLevel::guiInputFromDefinitionFinishedHandler>(this));

	Actor* actor = gom.addActor(sizeof(GuiInputFormComponent) + sizeof(uint32_t));
	gom.addComponent<GuiInputFormComponent>(1, *actor, std::move(guiInputFormDefinition), *Globals::window->getGfx().getRenderer<Graphics2D>(),
		Globals::window->getKeyboard(), Globals::window->getMouse());
	gom.addActor(sizeof(ModelComponent) + (sizeof(uint32_t) * 1));
	gom.addRenderComponent<ModelComponent>(0, physics);
	
	actor = gom.addActor(sizeof(IntroComponent) + (sizeof(uint32_t) * 1));
	gom.addComponent<IntroComponent>(3, *actor);
	
	addPeerDisplayActor();

	gom.endPipeline();

	eventManager.addListener(EventLevelReload::eventId, Delegate<void(EventData*)>::from<MenuLevel,
		&MenuLevel::eventLevelReloadHandler>(this));

	Globals::window->showAndUnclipCursor();
}
