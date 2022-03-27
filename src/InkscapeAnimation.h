#pragma once

#include "InscapeAnimationElement.h"
#include "Animation.h"

//NOTE: Get rect from id to keyFrame
class InkscapeAnimation : public Animation
{
	InkscapeAnimationElement iae;
	Vector<std::unordered_map<ShortString, IntRect>> inkscapeKeyFrames;
private:
	void setupInkscapeKeyFrames(const Vector<ShortString>& regionNames);
public:
	InkscapeAnimation(const Vector<ShortString>& regionNames, const TextureAtlas& atlas, const String& inkscapeFileName, int64_t frameDuration = Time::seconds(0.2f).asMicroseconds(), PlayMode type = PlayMode::LOOPED);
	InkscapeAnimation(const Vector<ShortString>& regionNames, const TextureAtlas& atlas, const InkscapeAnimationElement& iae, int64_t frameDuration = Time::seconds(0.2f).asMicroseconds(), PlayMode type = PlayMode::LOOPED);
	//NOTE: Should only be used if Animation::SetKeyFrames() was used before
	void setInkscapeAnimationElement(const String& inkscapeFileName, const Vector<ShortString>& regionNames);
	IntRect getInkscapeAnimationElementKeyFrame(const ShortString& keyFrameId) const;
};