#include "PlayerComponent.h"
#include "Globals.h"
#include "Window.h"
#include "EventHit.h"
#include "PhysicsLayers.h"
#include "EventZombieReset.h"
#include "EventGameOver.h"
#include "ModelComponent.h"

PlayerComponent::PlayerComponent(Physics& physicsIn, FireTriggers& fireTriggersIn, NetworkPacket& networkPacketIn) : Component(utils::getGUID()), physics(physicsIn),
renderer(Globals::window->getGfx().getRenderer<Graphics3D>()), renderer2d(Globals::window->getGfx().getRenderer<Graphics2D>()),
fireTriggers(fireTriggersIn), networkPacket(networkPacketIn), gui(*renderer2d, Globals::window->getKeyboard(), Globals::window->getMouse()),
audio(Globals::window->getAudio())
{
	model = Globals::window->getAssetManager()->getOrAddRes<MeshGltfModel>("models/theboss.glb", Vector<String>{ "models/theboss_walk.glb", "models/theboss_reload.glb" },
		Vector<String>{ "boss:Hat_Geo", "boss:Head_Geo", "boss:L_Eye_Geo", "boss:R_Eye_Geo", "boss:Teeth_Down_Geo", "boss:Teeth_Up_Geo", "boss:Cigar_Geo", "boss:Jacket_Geo" });
	model->animations.find("models/theboss_reload.glb")->second.setPlayMode(Animation::PlayMode::NORMAL);
	weapon = Globals::window->getAssetManager()->getOrAddRes<MeshGltfModel>("models/weapon.glb");
	fireSound = Globals::window->getAssetManager()->getOrAddRes<Sound>("sound/fire.wav");
	reloadSound = Globals::window->getAssetManager()->getOrAddRes<Sound>("sound/reload.wav");

	uint32_t jointIndex = 0;
	for (auto jointNodeMapIt : model->nodeJointMap)
	{
		if (jointNodeMapIt.second.nodeName == "boss:LeftHand")
		{
			jointIndex = jointNodeMapIt.second.jointIndex;
			break;
		}
	}
	uint8_t* vertexData = weapon->vertexBuffer.map(0, VK_WHOLE_SIZE);
	Model::Vertex* vertices = (Model::Vertex*)vertexData;
	for (uint32_t i = 0; i < weapon->nVertices; ++i)
	{
		vertices->jointIndices[0] = jointIndex;
	}
	weapon->vertexBuffer.unmap(0, VK_WHOLE_SIZE);

	texture = Globals::window->getAssetManager()->getOrAddRes<Texture>("models/Boss_diffuse.png");
	weaponTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("models/Weapon_texture.png");
	crosshairTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("images/crosshair.png");
	new (&crosshair) Sprite(crosshairTexture);
	crosshair.pos = Vector2f{ Globals::window->getGfx().renderWidth / 2.0f, Globals::window->getGfx().renderHeight / 2.0f };
	crosshair.org = Vector2f(0.5f, 0.5f);
	crosshair.scl = Vector2f(0.25f, 0.25f);

	Physics::Collider lineCollider(Physics::FloatLine{});
	Physics::Body lineBody({ 10.0f, 100.0f, 0.0f }, "line", std::move(lineCollider), {}, true, false);
	lineBody.setIsActive(false);
	lineBodyId = physics.addElementPointer(std::move(lineBody), (int32_t)PhysicsLayer::GUN);

	FloatCube floatCube(100.0f, 10.0f, 0.0f, 5.0f, 5.0f, 5.0f);
	Physics::Collider cubeCollider(std::move(floatCube));
	Physics::Body cubeBody({ 10.0f, 100.0f, 0.0f }, "player", std::move(cubeCollider), { (int32_t)PhysicsLayer::PLANE, (int32_t)PhysicsLayer::RADIUS }, false, false);
	playerBodyId = physics.addElementPointer(std::move(cubeBody), (int32_t)PhysicsLayer::PLAYER);

	fireTriggerIndex = fireTriggers.size();
	fireTriggers.emplace_back();
	Globals::eventManager->addListener(fireTriggers[fireTriggerIndex].healthEventIndex, Delegate<void(EventData*)>::from<PlayerComponent, &PlayerComponent::eventHit>(this));
	Globals::eventManager->addListener(EventZombieReset::eventId, Delegate<void(EventData*)>::from<PlayerComponent, &PlayerComponent::eventZombieReset>(this));
	Globals::eventManager->addListener(EventGameOver::eventId, Delegate<void(EventData*)>::from<PlayerComponent, &PlayerComponent::eventGameOver>(this));
}

void PlayerComponent::update(float dt, Actor* owner)
{
	if (reset)
	{
		health = 100.0f;
		rotAxisAngle.x = 0.0f;
		int32_t realIndex = physics.getRealIndex(playerBodyId);
		Physics::Body* playerBody = physics.getBodyFromRealIndex(realIndex);
		playerBody->setIsActive(true);
		playerBody->vel = { 0.0f, 0.0f, 0.0f };
		playerBody->pos = spectatorPosition;
		startedGame = true;
		gameOver = false;
		reset = false;

		showNewWaveTimer = 0.0f;
		resetZombieId = false;
	}

	hitPoints = HIT_HEALTH_COST * dt;

	if (showNewWaveTimer <= SHOW_WAVE_TIME)
	{
		showNewWaveTimer += dt;
	}
	else if(!resetZombieId)
	{
		resetZombieId = true;
		networkPacket.zombie = (uint8_t)-1;
	}

	guiAnimationPercentage += dt;
	if (guiAnimationPercentage > GUI_ANIMATION_DURATION)
	{
		guiAnimationPercentage = GUI_ANIMATION_DURATION;
	}

	int32_t realIndex = physics.getRealIndex(lineBodyId);
	Physics::Body* lineBody = physics.getBodyFromRealIndex(realIndex);

	if (health <= 0.0f)
	{
		rotAxisAngle.x = -PIf / 2.0f;
		renderer->camera.rot = Mat4x4::rotateX(rotAxisAngle.x);
		int32_t realIndex = physics.getRealIndex(playerBodyId);
		Physics::Body* playerBody = physics.getBodyFromRealIndex(realIndex);
		playerBody->setIsActive(false);
		lineBody->setIsActive(false);
		renderer->camera.pos = -spectatorPosition;
		networkPacket.peerTransforms.posX = (uint16_t)spectatorPosition.x;
		networkPacket.peerTransforms.posY = (uint8_t)spectatorPosition.y;
		networkPacket.peerTransforms.posZ = (uint16_t)spectatorPosition.z;
		renderer->camera.update();
		return;
	}

	crosshair.color = Colors::White;
	if (lineBody->getIsTriggerd())
	{
		for (auto it = fireTriggers.begin(); it != fireTriggers.end(); ++it)
		{
			if (it->bodyIndex == lineBody->getTriggerInformation().index)
			{
				EventHit::eventId = it->healthEventIndex;
				Globals::eventManager->TriggerEvent<EventHit>();
				break;
			}
		}
		crosshair.color = Colors::Magenta;
	}

	if (Globals::window->getMouse().isButtonPressed(Mouse::Button::left) && ammo > 0.0f)
	{
		lineBody->setIsActive(true);
		if (fireSound->isFinished())
		{
			// audio.start(fireSound);
		}
		ammo -= AMMO_COST * dt;

		if (ammo <= 0.0f)
		{
			animationName = "models/theboss_reload.glb";
			model->animations.find(animationName)->second.restart();
			// audio.start(reloadSound);
		}
	}
	else
	{
		lineBody->setIsActive(false);
		if (!fireSound->isFinished())
		{
			// audio.stop(fireSound);
		}
	}
	if (model->animations.find(animationName)->second.isAnimationFinished())
	{
		ammo = 100.0f;
		animationName = "models/theboss_walk.glb";

		if (!reloadSound->isFinished())
		{
			// audio.stop(reloadSound);
		}
	}

	PeerTransform playerTransform;
	handleRotation(dt, playerTransform);
	handleTranslation(dt, playerTransform);
	renderer->camera.update();
	networkPacket.peerTransforms = playerTransform;

	lineBody->getCollider().collider.line.p0 = -renderer->camera.pos;
	lineBody->getCollider().collider.line.p1 = -renderer->camera.pos + ((Mat4x4::rotate(rotAxisAngle) * Vector3f(0.0f, 0.0f, 1.0f)) * 50.0f);
}

void PlayerComponent::render()
{
	Vector2f textPos(GUI::GUI_RESOLUTION_X / 2.0f, GUI::GUI_RESOLUTION_Y - (FONT_SIZE + GUI::GUI_RESOLUTION_Y / 3.0f));

	if (health <= 0.0f)
	{
		if (!startedGame)
		{
			float animationPercentage = guiAnimationPercentage / GUI_ANIMATION_DURATION;

			gui.label("Zombie Modus", textPos, FONT_SIZE, GUI::AnchorPresets::bottomMiddle, Vector2f{ 0.5f, 0.5f }, Colors::Red, Color(1.0f, 1.0f, 1.0f, 0.5f), 
				labelAnimationBitmask, animationPercentage);
		}

		if (gameOver)
		{
			gui.label("Zombie Modus", textPos, FONT_SIZE, GUI::AnchorPresets::bottomMiddle, Vector2f{ 0.5f, 0.5f }, Colors::Red, Color(1.0f, 1.0f, 1.0f, 0.5f));
		}

		gui.submit();

		return;
	}

	SkeletalAnimation& walkAnimation = model->animations.find(animationName)->second;
	walkAnimation.baseTranslation.z = 0.1f;
	walkAnimation.updateJoints();
	Mat4x4 modelCameraTransform = renderer->camera.persProj * Mat4x4::translate(-modelOffset) * Mat4x4::scale({0.1f, 0.1f, 0.1f});
	renderer->draw(*model, modelCameraTransform, texture, model->uniformBuffer);

	modelCameraTransform = renderer->camera.persProj * Mat4x4::translate(weaponOffset) * Mat4x4::rotateY(PIf / 2.0f) * Mat4x4::scale({ 4.0f, 5.0f, 5.0f });
	renderer->draw(*weapon, modelCameraTransform, weaponTexture);

	renderer2d->draw(crosshair);

	if (showNewWaveTimer <= SHOW_WAVE_TIME)
	{
		String text("Welle ");
		text.append(1, '0' + (char)wave);
		gui.label(text, textPos, FONT_SIZE, GUI::AnchorPresets::bottomMiddle, Vector2f{ 0.5f, 0.5f }, Colors::White, Colors::Transparent);
	}

	gui.submit();
}

void PlayerComponent::handleTranslation(float dt, PeerTransform& peerTransforms)
{
	int32_t realIndex = physics.getRealIndex(playerBodyId);
	fireTriggers[fireTriggerIndex].bodyIndex = realIndex;
	Physics::Body* playerBody = physics.getBodyFromRealIndex(realIndex);
	playerBody->vel.x = 0.0f;
	playerBody->vel.z = 0.0f;

	if (Globals::window->getKeyboard().isKeyPressed('W'))
		playerBody->vel.z += MOVE_SPEED;
	if (Globals::window->getKeyboard().isKeyPressed('S'))
		playerBody->vel.z -= MOVE_SPEED;
	if (Globals::window->getKeyboard().isKeyPressed('A'))
		playerBody->vel.x -= MOVE_SPEED;
	if (Globals::window->getKeyboard().isKeyPressed('D'))
		playerBody->vel.x += MOVE_SPEED;

	float previousY = playerBody->vel.y;
	playerBody->vel = renderer->camera.rot * playerBody->vel;
	playerBody->vel.y = previousY + Physics::GRAVITY * dt;

	FloatCube& playerCollider = playerBody->getCollider().collider.cube;
	playerCollider.left = playerBody->pos.x + (playerBody->vel.x * dt);
	playerCollider.bottom = playerBody->pos.y + (playerBody->vel.y * dt);
	playerCollider.back = playerBody->pos.z + (playerBody->vel.z * dt);

	renderer->camera.pos = -Vector3f{ playerCollider.left, playerCollider.bottom, playerCollider.back } - modelOffset;

	peerTransforms.posX = (int16_t)playerCollider.left;
	peerTransforms.posY = (int8_t)playerBody->pos.y;
	peerTransforms.posZ = (int16_t)playerCollider.back;

	if (playerCollider.left < (-ModelComponent::PLANE_SIZE / 2.0f - playerCollider.width) || playerCollider.left >= ModelComponent::PLANE_SIZE / 2.0f ||
		playerCollider.back < (-ModelComponent::PLANE_SIZE / 2.0f - playerCollider.depth) || playerCollider.back >= ModelComponent::PLANE_SIZE / 2.0f)
	{
		health = -1.0f;
	}
}

void PlayerComponent::handleRotation(float dt, PeerTransform& peerTransforms)
{
	rotAxisAngle.x -= Globals::window->getMouse().rawDelta.y * dt * MOUSE_SENSITIVITY;
	rotAxisAngle.y -= Globals::window->getMouse().rawDelta.x * dt * MOUSE_SENSITIVITY;

	float clamp = 0.95f * (PIf / 2.0f);
	if (rotAxisAngle.x >= clamp)
	{
		rotAxisAngle.x = clamp;
	}
	else if (rotAxisAngle.x <= -clamp)
	{
		rotAxisAngle.x = -clamp;
	}

	if (rotAxisAngle.y >= PIf)
	{
		rotAxisAngle.y -= 2.0f * PIf;
	}
	else if (rotAxisAngle.y <= -PIf)
	{
		rotAxisAngle.y += 2.0f * PIf;
	}
	renderer->camera.rot = Mat4x4::rotateY(rotAxisAngle.y) * Mat4x4::rotateX(rotAxisAngle.x);

	peerTransforms.rotY = (int8_t)rotAxisAngle.y;
}

void PlayerComponent::eventHit(EventData* eventData)
{
	health = -1.0f;
}

void PlayerComponent::eventZombieReset(EventData* eventData)
{
	EventZombieReset* eventZombieResetData = (EventZombieReset*)eventData;

	reset = true;
	if (eventZombieResetData->resetWaves)
	{
		wave = 1;
	}
	else
	{
		++wave;
	}
	showNewWaveTimer = 0.0f;
}

void PlayerComponent::eventGameOver(EventData* eventData)
{
	gameOver = true;
}
