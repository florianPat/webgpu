#pragma once

#include "Component.h"
#include "Globals.h"
#include "Window.h"
#include <random>
#include "Physics.h"
#include "Quaternion.h"
#include "PhysicsLayers.h"
#include "MeshGltfModelPerMaterialFactory.h"
#include "Graphics.h"
#include "Terrain.h"

struct ModelComponent : public Component
{
	static constexpr float PLANE_SIZE = 512.0f;
private:
	Texture* skyboxTexture = nullptr;
	Texture* grassTexture = nullptr;
	MeshGltfModelPerMaterialFactory* mapleTreeFactory = nullptr;
	Texture* mapleBarkTexture = nullptr;
	Texture* mapleLeafTexture = nullptr;
	Graphics3D* renderer = nullptr;
	Terrain* terrain = nullptr;
	Vector<InstancedInfo> treeInstanceInfos;
public:
	ModelComponent(Physics& physics)
		: Component(utils::getGUID()), renderer(Globals::window->getGfx().getRenderer<Graphics3D>())
	{
		skyboxTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("skybox.png", Texture::TextureType::CUBE_MAP);
		grassTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("images/grass.png");
		mapleTreeFactory = Globals::window->getAssetManager()->getOrAddRes<MeshGltfModelPerMaterialFactory>("models/trees/maple.glm");
		mapleBarkTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("models/trees/bark05.png");
		mapleLeafTexture = Globals::window->getAssetManager()->getOrAddRes<Texture>("models/trees/leaf maple.png");
		terrain = Globals::window->getAssetManager()->getOrAddRes<Terrain>("Terrain.ter", 8, PLANE_SIZE, 16, Vector3f{ -PLANE_SIZE / 2.0f, 5.0f, -PLANE_SIZE / 2.0f });

		std::random_device rd;
		std::mt19937 rng(rd());
		std::uniform_real_distribution dist(-PLANE_SIZE / 2.0f, PLANE_SIZE / 2.0f);

		for (uint32_t i = 0; i < 12; ++i)
		{
			InstancedInfo instanceInfo;
			float x = dist(rng), z = dist(rng);
			instanceInfo.pos = { x, terrain->getHeight(x, z) + 3.0f, z };
			instanceInfo.rot = { 0.0f, 0.0f, 0.0f };
			instanceInfo.scl = { 0.25f, 0.25f, 0.25f };
			treeInstanceInfos.push_back(instanceInfo);
		}

		Physics::Collider terrainCollider(terrain);
		Physics::Body terrainBody("terrain", std::move(terrainCollider));
		physics.addElementValue(std::move(terrainBody), (int32_t)PhysicsLayer::PLANE);
	}
	void render() override
	{
		renderer->drawTerrain(*terrain, terrain->pos, grassTexture);
		Model cube = Model::cube();
		renderer->drawSkybox(skyboxTexture);
		// TODO: Because of double sided material one side looks ugly. Fix this!
		for (auto it = mapleTreeFactory->getMeshModels().begin(); it != mapleTreeFactory->getMeshModels().end(); ++it)
		{
			renderer->drawInstanced(it->model, treeInstanceInfos, it->texture);
		}
	}
};