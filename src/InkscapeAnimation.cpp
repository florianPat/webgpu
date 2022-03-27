#include "InkscapeAnimation.h"
#include "Utils.h"

void InkscapeAnimation::setupInkscapeKeyFrames(const Vector<ShortString>& regionNames)
{
	for (auto it = regionNames.begin(); it != regionNames.end(); ++it)
	{
		if (iae.isElementInMap(*it))
			inkscapeKeyFrames.push_back(std::unordered_map<ShortString, IntRect>{ iae.getElementMap(*it) });
	}
}

InkscapeAnimation::InkscapeAnimation(const Vector<ShortString>& regionNames, const TextureAtlas & atlas, const String& inkscapeFileName, int64_t frameDuration, PlayMode type)
	: Animation(regionNames, atlas, frameDuration, type), iae(inkscapeFileName, regionNames), inkscapeKeyFrames()
{
	setupInkscapeKeyFrames(regionNames);
}

InkscapeAnimation::InkscapeAnimation(const Vector<ShortString>& regionNames, const TextureAtlas & atlas, const InkscapeAnimationElement & iae, int64_t frameDuration, PlayMode type)
	: Animation(regionNames, atlas, frameDuration, type), iae(iae), inkscapeKeyFrames()
{
	setupInkscapeKeyFrames(regionNames);
}

void InkscapeAnimation::setInkscapeAnimationElement(const String& inkscapeFileName, const Vector<ShortString>& regionNames)
{
	inkscapeKeyFrames.clear();

	iae = InkscapeAnimationElement(inkscapeFileName, regionNames);

	for (auto it = regionNames.begin(); it != regionNames.end(); ++it)
	{
		inkscapeKeyFrames.push_back(std::unordered_map<ShortString, IntRect>{ iae.getElementMap(*it) });
	}
}

IntRect InkscapeAnimation::getInkscapeAnimationElementKeyFrame(const ShortString & keyFrameId) const
{
	uint32_t i = (playMode == PlayMode::LOOPED || playMode == PlayMode::NORMAL) ? keyFrameIt : keyFrameItReverse;

	auto result = inkscapeKeyFrames[i].find(keyFrameId);
	if (result != inkscapeKeyFrames[i].end())
		return result->second;
	else
	{
		InvalidCodePath;
		return IntRect();
	}
}
