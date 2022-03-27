#include "TextureAtlas.h"
#include "Ifstream.h"
#include "Utils.h"
#include "Rect.h"
#include <cstdlib>

TextureAtlas::TextureAtlas(const String& filepath, AssetManager* assetManager) : textureAtlas(), fileHeader{}
{
	LongString dirsToAdd = utils::getDirPathToFile(filepath);

	Ifstream file;
	file.open(filepath);

	if (!file)
	{
		utils::logBreak("Cant open file!");
	}

	file.readTempLine();

	if (!file.eof())
	{
		LongString lineContent;

		for (int32_t i = 0; i < FILE_HEADER_LINE_SIZE; ++i)
		{
			file.getline(lineContent);

			switch (i)
			{
				case 0:
				{
					//NOTE: The dirsToAdd are added here!
					fileHeader.name = dirsToAdd + lineContent;
				}	break;
				case 1:
				{
					int32_t width, height;
					ShortString widthChar, heightChar;
					
					widthChar = getLineContentBetweeen(lineContent, ' ', ',');
					
					uint32_t nCharsToCopyHeight = lineContent.size();
					heightChar = lineContent.substr(0, nCharsToCopyHeight);

					width = atoi(widthChar.c_str());
					height = atoi(heightChar.c_str());

					fileHeader.size = Vector2i(width, height);
				}	break;
				case 2:
				{
					uint32_t spacePos = lineContent.find(' ');
					lineContent.erase(0, ++spacePos);
					fileHeader.format = lineContent;
				}	break;
				case 3:
				{
					fileHeader.filter[0] = getLineContentBetweeen(lineContent, ' ', ',');
					
					uint32_t nCharsToCopyFilter1 = lineContent.size();
					fileHeader.filter[1] = lineContent.substr(0, nCharsToCopyFilter1);
				}	break;
				case 4:
				{
					uint32_t spacePos = lineContent.find(' ');
					lineContent.erase(0, ++spacePos);
					fileHeader.repeat = lineContent;
					break;
				}
				default:
				{
					InvalidCodePath;
					break;
				}
			}
		}
	}

	while(!file.eof())
	{
		bool isEof = false;
		TextureRegion region;

		region.textureAtlasFileName = &fileHeader.name;

		for (int32_t i = 0; i < FILE_LINES_PER_REGION; ++i)
		{
			if (file.eof())
			{
				isEof = true;
				break;
			}

			LongString lineContent;
			file.getline(lineContent);

			switch (i)
			{
				case 0:
				{
					region.filename = lineContent;
				}	break;
				case 1: {}	break;
				case 2:
				{
					region.xy = getLineContentRegionValues(lineContent, 'x');
				}	break;
				case 3:
				{
					region.size = getLineContentRegionValues(lineContent, 's');
				}	break;
				case 4: {}	break;
				case 5: {}	break;
				case 6: {}	break;
				default:
				{
					InvalidCodePath;
					break;
				}
			}
		}

		region.xy.y = fileHeader.size.y - region.size.y - region.xy.y;

		if(!isEof)
			textureAtlas.insert({ region.filename, region });
	}

	file.close();

	for (auto i = textureAtlas.begin(); i != textureAtlas.end(); ++i)
	{
		i->second.initSprite(assetManager);
	}
}

const TextureRegion* TextureAtlas::findRegion(const String& name) const
{
	auto result = textureAtlas.find(name);
	if (result != textureAtlas.end())
	{
		return &result->second;
	}
	else
	{
		return nullptr;
	}
}

const std::unordered_map<String, TextureRegion>& TextureAtlas::getRegions() const
{
	return textureAtlas;
}

void TextureAtlas::addRegion(const TextureRegion & adder)
{
	textureAtlas.insert({ adder.filename, adder });
}

String TextureAtlas::getLineContentBetweeen(String & lineContent, char first, char secound) const
{
	String result = LongString();

	uint32_t spacePos = lineContent.find(first);
	lineContent.erase(0, ++spacePos);

	uint32_t kommaPos = lineContent.find(secound);

	uint32_t nCharsToCopy = kommaPos;
	result = lineContent.substr(0, nCharsToCopy);

	lineContent.erase(0, ++kommaPos);

	return result;
}

Vector2i TextureAtlas::getLineContentRegionValues(String & lineContent, char firstRealChar) const
{
	Vector2i result;

	lineContent.erase(0, lineContent.find(firstRealChar));
	result.x = atoi(getLineContentBetweeen(lineContent, ' ', ',').c_str());
	lineContent.erase(0, 1);
	result.y = atoi(lineContent.c_str());

	return result;
}

void TextureRegion::initSprite(AssetManager* assetManager)
{
	assert((!textureAtlasFileName->empty()) || (!filename.empty()));

	atlasTexture = assetManager->getOrAddRes<Texture>(*textureAtlasFileName);
	regionSprite = Sprite(atlasTexture, IntRect(xy.x, xy.y, size.x, size.y));
}

void TextureRegion::setRegion(int32_t x, int32_t y, int32_t widht, int32_t height)
{
	if(x >= 0 && y >= 0)
		if ((uint32_t)widht <= regionSprite.getTexture()->getWidth() && (uint32_t)height <= regionSprite.getTexture()->getHeight())
		{
			regionSprite.setTextureRect(IntRect(x, y, widht, height));
			xy.x = x;
			xy.y = y;
			size.x = widht;
			size.y = height;
		}
}
