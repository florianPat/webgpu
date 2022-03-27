#pragma once

#include "Component.h"
#include "MeshGltfModel.h"
#include "Physics.h"
#include "Sprite.h"
#include "FireTrigger.h"
#include "NetworkPacket.h"
#include "GUI.h"
#include "Sound.h"

struct PlayerComponent : public Component
{
	// NOTE: max 127 because int8_t in sending struct
	static constexpr int8_t SPECTATOR_POSITION_Y = 127;
private:
	static constexpr float MOVE_SPEED = 50.0f;
	static constexpr float MOUSE_SENSITIVITY = PIf / 4.0f;
	static constexpr float HIT_HEALTH_COST = 100.0f;
	static constexpr float SHOW_WAVE_TIME = 2.0f;
	static constexpr uint32_t FONT_SIZE = 64;
	static constexpr float AMMO_COST = 50.0f;
	const Vector3f spectatorPosition = { 0.0f, (float)SPECTATOR_POSITION_Y, 0.0f };
	const Vector3f modelOffset = { 0.0f, 15.0f, 0.0f };
	const Vector3f weaponOffset = { -1.2f, -1.0f, 4.0f };
	static constexpr float GUI_ANIMATION_DURATION = 2.0f;

	MeshGltfModel* model = nullptr;
	MeshGltfModel* weapon = nullptr;
	Texture* crosshairTexture = nullptr;
	Sprite crosshair;
	Texture* texture = nullptr;
	Texture* weaponTexture = nullptr;
	Physics& physics;
	Graphics3D* renderer = nullptr;
	Graphics2D* renderer2d = nullptr;
	Vector3f rotAxisAngle;
	int32_t lineBodyId = -1;
	int32_t playerBodyId = -1;
	bool reset = true;
	bool startedGame = false;
	bool gameOver = false;
	bool resetZombieId = false;
	FireTriggers& fireTriggers;
	NetworkPacket& networkPacket;
	uint32_t fireTriggerIndex = 0;
	float health = -1.0f;
	float hitPoints = 0.0f;
	float showNewWaveTimer = 0.0f;
	float ammo = 100.0f;
	uint32_t wave = 1;
	String animationName = "models/theboss_walk.glb";
	GUI gui;
	String playerName = ShortString("");
	float guiAnimationPercentage = 0.0f;
	uint32_t labelAnimationBitmask = (uint32_t)GUI::GuiAnimation::FADE_IN | (uint32_t)GUI::GuiAnimation::SLIDE_FROM_TOP;
	uint32_t inputAnimationBitmask = (uint32_t)GUI::GuiAnimation::FADE_IN | (uint32_t)GUI::GuiAnimation::SLIDE_FROM_BOTTOM;
	Sound* fireSound = nullptr;
	Sound* reloadSound = nullptr;
	Audio& audio;
public:
	PlayerComponent(Physics& physics, FireTriggers& fireTriggers, NetworkPacket& networkPacket);
	void update(float dt, Actor* owner) override;
	void render() override;
protected:
	void handleTranslation(float dt, PeerTransform& peerTransforms);
	void handleRotation(float dt, PeerTransform& peerTransforms);
private:
	void eventHit(EventData* eventData);
	void eventZombieReset(EventData* eventData);
	void eventGameOver(EventData* eventData);
};