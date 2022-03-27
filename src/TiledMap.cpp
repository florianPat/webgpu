#include "TiledMap.h"
#include "Ifstream.h"
#include "Utils.h"
#include "TiledMapRenderComponent.h"
#include <cstdlib>
#include "Globals.h"

const Vector<Physics::Collider>& TiledMap::getObjectGroup(const ShortString& objectGroupName) const
{
	auto result = objectGroups.find(objectGroupName);
	if (result != objectGroups.end())
		return result->second.objects;
	else
	{
		InvalidCodePath;
		return result->second.objects;
	}
}

const std::unordered_map<ShortString, TiledMap::ObjectGroup>& TiledMap::getObjectGroups() const
{
	return objectGroups;
}

void TiledMap::draw()
{
	assert(gfx != nullptr);

	gfx->draw(textureSprite);
}

uint32_t TiledMap::getEndOfWord(const String & word, const String & lineContent, bool* result)
{
	uint32_t o = 0;
	*result = false;
	while (o < lineContent.size() && !(*result))
	{
		String searchWord(word);
		auto it = searchWord.begin();
		o = lineContent.find(it[0], o);
		++it;
		for (; o < lineContent.size(); ++it)
		{
			if (it != searchWord.end())
			{
				if (lineContent.at(++o) == it[0])
					continue;
				else
					break;
			}
			else
			{
				*result = true;
				break;
			}
		}
	}

	return ++o;
}

String TiledMap::getLineContentBetween(String & lineContent, const String & endOfFirst, char secound)
{
	bool resultValue;
	uint32_t widthEndPos = getEndOfWord(endOfFirst, lineContent, &resultValue);
	if (resultValue)
	{
		lineContent.erase(0, widthEndPos += 2);

		uint32_t kommaPos = lineContent.find(secound);

		String result(kommaPos);

		result = lineContent.substr(0, kommaPos);

		lineContent.erase(0, ++kommaPos);

		return result;
	}
	else
		return ShortString();
}

void TiledMap::ParseLayer(Ifstream & file, String& lineContent)
{
	while (utils::isWordInLine("<layer", lineContent))
	{
		String layerName = getLineContentBetween(lineContent, "name", '"');
		int32_t layerWidth = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
		int32_t layerHeight = atoi(getLineContentBetween(lineContent, "height", '"').c_str());

		layers.push_back(Layer{ layerName, layerWidth, layerHeight, Vector<Tile>() });

		Layer& currentLayer = layers.back();
		currentLayer.tiles.reserve(layerWidth * layerHeight);

		file.getline(lineContent); //  <data encoding="csv">
		if (!utils::isWordInLine("csv", lineContent))
		{
			utils::logBreak("Maps encoding has to be \"csv\"");
		}

		file.getline(lineContent); //Begin of encoding

		for (int32_t y = 0; y < layerHeight; ++y)
		{
			for (int32_t x = 0; x < layerWidth; ++x)
			{
				uint32_t kommaPos = lineContent.find(',');
				int32_t nextTileId = atoi(lineContent.substr(0, kommaPos).c_str());
				lineContent.erase(0, ++kommaPos);

				currentLayer.tiles.push_back(tiles.at(nextTileId));
			}
			file.getline(lineContent);
		}
		assert(utils::isWordInLine("</data>", lineContent));
		file.getline(lineContent); // </layer>
		assert(utils::isWordInLine("</layer>", lineContent));
		file.getline(lineContent); //Maybe new <layer>
	}
}

void TiledMap::ParseObjectGroups(Ifstream & file, String & lineContent)
{
	while (utils::isWordInLine("<objectgroup", lineContent))
	{
		String objectGroupName = getLineContentBetween(lineContent, "name", '"');
		file.getline(lineContent);

		Vector<Physics::Collider> objectVector;
		while (!utils::isWordInLine("</objectgroup>", lineContent))
		{
			assert(utils::isWordInLine("<object", lineContent));

			int32_t x = atoi(getLineContentBetween(lineContent, "x", '"').c_str());
			int32_t y = atoi(getLineContentBetween(lineContent, "y", '"').c_str());
			int32_t width = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
			int32_t height = atoi(getLineContentBetween(lineContent, "height", '"').c_str());

			//NOTE: Need to do this because it is stored top down and not bottom up!
			y = (mapHeight * tileHeight) - tileHeight - y;

			objectVector.push_back(FloatRect((float)x, (float)y, (float)width, (float)height));

			file.getline(lineContent);
		}
		objectGroups.emplace(objectGroupName, ObjectGroup{ objectGroupName, objectVector });

		file.getline(lineContent);
	}
}

void TiledMap::MakeRenderTexture()
{
	assert(gfx != nullptr);
	new (&texture) RenderTexture(mapWidth*tileWidth, mapHeight*tileHeight, &Globals::window->getGfx(), gfx->id);
	assert(texture);

	Mat4x4 orthoProj = Mat4x4::orthoProj(0.0f, 0.0f, (float)mapWidth * tileWidth, (float)mapHeight * tileHeight);

	texture.clear();
	gfx->bindOtherFramebuffer(texture.getFramebufferDrawStruct());
	gfx->bindOtherOrthoProj(orthoProj);

	for (auto it = layers.begin(); it != layers.end(); ++it)
	{
		//NOTE: posY is needed here, because renderTexture 0 is bottom, but here it is top...
		for (int32_t y = 0, posY = mapHeight - 1; y < mapHeight; ++y, --posY)
		{
			for (int32_t x = 0; x < mapWidth; ++x)
			{
				Layer& currentLayer = *it;
				Texture* source = currentLayer.tiles.at(mapWidth * y + x).source;
				if (source == nullptr)
					continue;
				Sprite sprite(source);
				sprite.pos = Vector2f((float)x * tileWidth, (float)posY * tileHeight);

				gfx->draw(sprite);
			}
		}
	}
	gfx->unbindOtherFramebuffer();
	gfx->unbindOtherOrthoProj();
	texture.changeToShaderReadMode();

	textureSprite = Sprite(&texture.getTexture());
}

String TiledMap::ParseTiles(Ifstream & file, AssetManager* assetManager, const String& filename)
{
    LongString dirsToAdd = utils::getDirPathToFile(filename);

	String lineContent = LongString();
	file.getline(lineContent);

	bool gridInFile = false;
	LongString grid;
	file.getline(grid); // <grid...
	if (utils::isWordInLine("<grid", grid))
	{
		gridInFile = true;
	}

	while (utils::isWordInLine("<tileset", lineContent))
	{
		int32_t firstgrid = atoi(getLineContentBetween(lineContent, "firstgid", '"').c_str());

		for (int32_t i = tiles.size(); i < firstgrid; ++i)
		{
			tiles.push_back(Tile{i, 0, 0, nullptr});
		}

		int32_t tileCount = atoi(getLineContentBetween(lineContent, "tilecount", '"').c_str());
		for (int32_t i = 0; i < tileCount; ++i)
		{
			if (gridInFile)
			{
				file.getline(lineContent);
			}
			else
			{
				lineContent = grid;
				//NOTE: Just set it here, because in the next it`s it will get the next line
				gridInFile = true;
			}
			assert(utils::isWordInLine("<tile", lineContent));
			int32_t id = atoi(getLineContentBetween(lineContent, "id", '"').c_str()) + firstgrid;

			file.getline(lineContent);
			assert(utils::isWordInLine("<image", lineContent));
			int32_t width = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
			int32_t height = atoi(getLineContentBetween(lineContent, "height", '"').c_str());
			String source = getLineContentBetween(lineContent, "source", '"');
			assert(tiles.size() == (uint32_t)id);
			tiles.push_back(Tile{ id, width, height, assetManager->getOrAddRes<Texture>(dirsToAdd + source) });

			file.getline(lineContent); //</tile>
			assert(utils::isWordInLine("</tile>", lineContent));
		}
		file.getline(lineContent); //</tileset>
		assert(utils::isWordInLine("</tileset>", lineContent));
		file.getline(lineContent); //Maybe new <tileset>...
	}

	return lineContent;
}

bool TiledMap::reloadFromFile(const String& filename)
{
    texture = RenderTexture();

	MakeRenderTexture();

	return true;
}

Vector2f TiledMap::getMapSize() const
{
    return Vector2f{ (float)mapWidth * tileWidth, (float)mapHeight * tileHeight };
}

TiledMap::TiledMap(const String& filename)
{
	gfx = Globals::window->getGfx().getRenderer<Graphics2D>();

	Ifstream file;
	file.open(filename);

	if (!file)
	{
		utils::logBreak("Cant open file!");
		InvalidCodePath;
	}

	file.readTempLine();

	if (!file.eof())
	{
		LongString lineContent;
		file.getline(lineContent);
		assert(utils::isWordInLine("<map", lineContent));

		if (!utils::isWordInLine("orthogonal", lineContent))
		{
			utils::logBreak("Map has to be orthogonal!");
			InvalidCodePath;
		}

		if (!utils::isWordInLine("right-down", lineContent))
		{
			utils::logBreak("Maps render-order has to be right-down!");
			InvalidCodePath;
		}

		mapWidth = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
		mapHeight = atoi(getLineContentBetween(lineContent, "height", '"').c_str());

		tileWidth = atoi(getLineContentBetween(lineContent, "tilewidth", '"').c_str());
		tileHeight = atoi(getLineContentBetween(lineContent, "tileheight", '"').c_str());

		lineContent = ParseTiles(file, Globals::window->getAssetManager(), filename);

		ParseLayer(file, lineContent);

		ParseObjectGroups(file, lineContent);

		if (!utils::isWordInLine("</map>", lineContent))
		{
			utils::logBreak("We should be at the end of the file!");
			InvalidCodePath;
		}

		MakeRenderTexture();
	}
}

const Texture* TiledMap::getTexture() const
{
    return &texture.getTexture();
}
