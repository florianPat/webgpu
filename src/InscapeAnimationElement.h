#pragma once

#include <unordered_map>
#include "Vector2.h"
#include "Rect.h"
#include "AssetManager.h"

//TODO: Add ellipse / (path) processing and rotation / scale transforms

//NOTE: Class to get out rects with specific ids in specifig groups from Inkscape-files
class InkscapeAnimationElement
{
	static constexpr int32_t IMAGE_SIZE = 64;
	std::unordered_map<ShortString, std::unordered_map<ShortString, IntRect>> elementMap;
private:
	bool FindGroupLayer(String& lineContent) const;
public:
	InkscapeAnimationElement() = default;
	InkscapeAnimationElement(const String& inkscapeFileName, const Vector<ShortString>& regionNames);
	InkscapeAnimationElement(const String& inkscapeFileName);
	IntRect getElementRect(ShortString& keyFrameId, ShortString& elementId) const;
	std::unordered_map<ShortString, IntRect> getElementMap(const ShortString& keyFrameId) const;
	bool isElementInMap(const ShortString& keyFrameId) const;
};