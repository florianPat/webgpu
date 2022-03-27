#include "InscapeAnimationElement.h"
#include "Utils.h"
#include "Ifstream.h"
#include <cstdlib>

InkscapeAnimationElement::InkscapeAnimationElement(const String& inkscapeFileName, const Vector<ShortString>& regionNames)
	: elementMap()
{
	Ifstream file;
	file.open(inkscapeFileName);
	assert(file);

	LongString lineContent;

	for(int32_t iteration = 0; !file.eof(); ++iteration)
	{
		if (lineContent.find("</g>") != String::npos)
		{
			break;
		}
		else if (iteration == 1)
		{
			utils::logBreak("There is an element outside a group!!");
		}

		file.getline(lineContent);

		while (FindGroupLayer(lineContent))
		{
			file.getline(lineContent);
		}

		for (file.getline(lineContent); lineContent.find("<g") != String::npos; file.getline(lineContent))
		{
			file.getline(lineContent); //<g
			ShortString groupId;
			std::unordered_map<ShortString, IntRect> rectMap;
			Vector2i translationVec = { 0, 0 };
			Vector2i scalingVec = { 1, 1 };
			bool shouldAdd = true;
			for (; shouldAdd && lineContent.find("</g>") == String::npos; file.getline(lineContent))
			{
				if (lineContent.find("id") != String::npos)
				{
					groupId = utils::getWordBetweenChars(lineContent, '"', '"');
					if (!regionNames.empty())
					{
						if (regionNames[0] == "Process all")
							continue;
					}
					for (auto it = regionNames.begin(); it != regionNames.end(); ++it)
					{
						if (groupId.find(*it) == String::npos)
						{
							shouldAdd = false;
							break;
						}
					}
				}
				else if (lineContent.find("<rect") != String::npos)
				{
					ShortString id;
					IntRect rect(0, 0, 0, 0);
					bool shouldAdd = true;
					bool addWidth = false;

					Vector2i beforeTranslationVec = translationVec;
					Vector2i beforeScalingVec = scalingVec;

					for (file.getline(lineContent); shouldAdd; file.getline(lineContent))
					{
						if (lineContent.find("x") != String::npos)
						{
							rect.left = atoi(utils::getWordBetweenChars(lineContent, '"', '"').c_str());
						}
						else if (lineContent.find("y") != String::npos)
						{
							if(lineContent.find("r") == String::npos)
								rect.bottom = atoi(utils::getWordBetweenChars(lineContent, '"', '"').c_str());
						}
						else if (lineContent.find("width") != String::npos)
						{
							rect.width = atoi(utils::getWordBetweenChars(lineContent, '"', '"').c_str());
							if (addWidth)
								translationVec.x += -rect.width;
						}
						else if (lineContent.find("height") != String::npos)
						{
							rect.height = atoi(utils::getWordBetweenChars(lineContent, '"', '"').c_str());
						}
						else if (lineContent.find("id") != String::npos)
						{
							id = utils::getWordBetweenChars(lineContent, '"', '"');
							if (id.find("rect") != String::npos || id.find("-") != String::npos)
							{
								shouldAdd = false;
								break;
							}
							uint32_t underlinePos = id.find_first_of('_');
							if (underlinePos != String::npos)
							{
								id = id.substr(0, underlinePos);
							}
						}
						else if (lineContent.find("transform") != String::npos)
						{
							if (lineContent.find("scale") != String::npos)
							{
								auto firstOne = lineContent.find_first_of('1');
								auto lasttOne = lineContent.find_last_of('1');
								if (firstOne == lasttOne)
								{
									utils::logBreak("Scaling is only allowed with a 1");
								}
								else
								{
									bool isXScale = atoi(utils::getWordBetweenChars(lineContent, '(', ',').c_str()) == -1 ? true : false;
									if (isXScale)
									{
										scalingVec.x = -1;
										if (rect.width != 0)
											translationVec.x += -rect.width;
										else
											addWidth = true;
									}
									else
									{
										//TODO: Same as in x?
										utils::logBreak("-1 scaling in Y is currently not implemented");
									}
								}
							}
							else if (lineContent.find("translate") != String::npos)
							{
								translationVec.x += atoi(utils::getWordBetweenChars(lineContent, '(', ',').c_str());
								translationVec.y += atoi(utils::getWordBetweenChars(lineContent, ',', ')').c_str());
							}
							else
							{
								utils::logBreak("Other transformations are currently not supported");
							}
						}

						if (lineContent.find("/>") != String::npos)
						{
							break;
						}
					}
					if (shouldAdd)
					{
						rect.left *= scalingVec.x;
						rect.bottom *= scalingVec.y;
						rect.left += translationVec.x;
						rect.bottom += translationVec.y;
						rectMap.emplace(id, rect);

						scalingVec = beforeScalingVec;
						translationVec = beforeTranslationVec;
					}
					else
					{
						scalingVec = beforeScalingVec;
						translationVec = beforeTranslationVec;

						for (; lineContent.find("/>") == String::npos; file.getline(lineContent));
					}
				}
				else if (lineContent.find("<") != String::npos)
				{
					for (file.getline(lineContent); lineContent.find("/>") == String::npos; file.getline(lineContent));
				}
				else if (lineContent.find("transform") != String::npos)
				{
					if (lineContent.find("translate") != String::npos)
					{
						translationVec.x += atoi(utils::getWordBetweenChars(lineContent, '(', ',').c_str());
						translationVec.y += atoi(utils::getWordBetweenChars(lineContent, ',', ')').c_str());
					}
					else
					{
						utils::logBreak("Other transformations are currently not supported");
					}
				}
			}

			for (; lineContent.find("</g>") == String::npos; file.getline(lineContent));

			if (!rectMap.empty())
			{
				auto base = rectMap.find("Base");
				if (base != rectMap.end())
				{
					if (rectMap.size() == 1)
					{
						utils::logBreak("Only found \"Base\" element, but there is need for more!");
					}

					Vector2i baseVec = { base->second.left, base->second.bottom };
					rectMap.erase(base);
					for (auto it = rectMap.begin(); it != rectMap.end(); ++it)
					{
						assert(baseVec.x <= it->second.left);
						assert(baseVec.y <= it->second.bottom);

						it->second.left -= baseVec.x;
						it->second.bottom -= baseVec.y;

						assert(it->second.left <= IMAGE_SIZE && it->second.left >= 0);
						assert(it->second.bottom <= IMAGE_SIZE && it->second.bottom >= 0);
					}
				}
				else
				{
					utils::logBreak("No \"Base\" found in this rect definition");
				}

				elementMap.emplace(groupId, rectMap);
			}
		}
	}
}

InkscapeAnimationElement::InkscapeAnimationElement(const String & inkscapeFileName) 
	: InkscapeAnimationElement(inkscapeFileName, Vector<ShortString>(1, ShortString("Process all")))
{
}

bool InkscapeAnimationElement::FindGroupLayer(String & lineContent) const
{
	return (lineContent.find("id=\"layer") == String::npos);
}

IntRect InkscapeAnimationElement::getElementRect(ShortString& keyFrameId, ShortString& elementId) const
{
	auto keyFrameResult = elementMap.find(keyFrameId);
	if (keyFrameResult != elementMap.end())
	{
		auto result = keyFrameResult->second.find(elementId);
		if (result != keyFrameResult->second.end())
			return result->second;
		else
		{
			InvalidCodePath;
			return IntRect();
		}
	}
	else
	{
		InvalidCodePath;
		return IntRect();
	}
}

std::unordered_map<ShortString, IntRect> InkscapeAnimationElement::getElementMap(const ShortString & keyFrameId) const
{
	auto result = elementMap.find(keyFrameId);
	if (result != elementMap.end())
		return result->second;
	else
	{
		InvalidCodePath;
		return std::unordered_map<ShortString, IntRect>();
	}
}

bool InkscapeAnimationElement::isElementInMap(const ShortString & keyFrameId) const
{
	auto result = elementMap.find(keyFrameId);
	return result != elementMap.end();
}
