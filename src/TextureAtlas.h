#pragma once

#include "Vector.h"
#include "String.h"
#include <unordered_map>
#include "Vector2.h"
#include "Sprite.h"
#include "AssetManager.h"

class TextureRegion
{
	friend class TextureAtlas;

	const LongString* textureAtlasFileName = nullptr;
	ShortString filename;
	Vector2i xy;
	Vector2i size;

	Texture* atlasTexture;
	Sprite regionSprite;
private:
	TextureRegion() = default;
	void initSprite(AssetManager* assetManager);
public:
	String getAtlasFileName() const { return *textureAtlasFileName; }
	String getRegionName() const { return filename; }
	Vector2i getXY() const { return xy; }
	Vector2i getSize() const { return size; }

	void setRegion(int32_t x, int32_t y, int32_t widht, int32_t height);
	Sprite getRegion() const { return regionSprite; }
};

class TextureAtlas
{
	struct FileHeader
	{
		LongString name;
		Vector2i size;
		ShortString format;
		ShortString filter[2];
		ShortString repeat;
	};
public:
	TextureAtlas(const String& filepath, AssetManager* assetManger);

	const TextureRegion* findRegion(const String& name) const;
	const std::unordered_map<String, TextureRegion>& getRegions() const;
	void addRegion(const TextureRegion& adder);
private:
	String getLineContentBetweeen(String& lineContent, char first, char secound) const;
	Vector2i getLineContentRegionValues(String& lineContent, char firstRealChar) const;
private:
	std::unordered_map<String, TextureRegion> textureAtlas;
	static constexpr int32_t FILE_HEADER_LINE_SIZE = 5;
	static constexpr int32_t FILE_LINES_PER_REGION = 7;
	FileHeader fileHeader;
};