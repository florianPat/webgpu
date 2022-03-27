#pragma once

#include <unordered_map>
#include "Physics.h"
#include "GameObjectManager.h"
#include "AssetManager.h"
#include "Ifstream.h"
#include "Graphics.h"
#include "Sprite.h"
#include "RenderTexture.h"
#include "Graphics.h"

class TiledMap
{
	struct Tile
	{
		int32_t id;
		int32_t width, height;
		Texture* source;
	};
	struct Layer
	{
		ShortString name;
		int32_t width, height;
		Vector<Tile> tiles;
	};
	struct ObjectGroup
	{
		ShortString name;
		Vector<Physics::Collider> objects;
	};

	Vector<Tile> tiles;
	Vector<Layer> layers;
	std::unordered_map<ShortString, ObjectGroup> objectGroups;
	int32_t mapWidth = 0, mapHeight = 0, tileWidth = 0, tileHeight = 0;

	RenderTexture texture;
	Sprite textureSprite;
	Graphics2D* gfx;
public:
	bool reloadFromFile(const String& filename);
public:
	TiledMap(const String& filename);
	const Vector<Physics::Collider>& getObjectGroup(const ShortString& objectGroupName) const;
	const std::unordered_map<ShortString, ObjectGroup>& getObjectGroups() const;
	void draw();
	uint64_t getSize() const { return sizeof(TiledMap); }
	Vector2f getMapSize() const;
	const Texture* getTexture() const;
private:
	uint32_t getEndOfWord(const String& word, const String& lineContent, bool* result);
	String getLineContentBetween(String& lineContent, const String& endOfFirst, char secound);

	String ParseTiles(Ifstream& file, AssetManager* assetManager, const String& filename);
	void ParseLayer(Ifstream& file, String& lineContent);
	void ParseObjectGroups(Ifstream& file, String& lineContent);
	void MakeRenderTexture();
};