#pragma once

#include "Vector.h"
#include "TextureAtlas.h"
#include "stdint.h"
#include "Clock.h"

class Animation
{
public:
	enum class PlayMode
	{
		LOOPED,
		LOOP_REVERSED,
		NORMAL,
		REVERSED
	};
protected:
	uint32_t keyFrameIt;
	uint32_t keyFrameItReverse;
	uint64_t currentTime = 0;
	PlayMode playMode;
	Clock clock;
	bool paused = false;
public:
	Vector<Sprite> keyFrames;
	uint64_t frameDuration;
public:
	Animation(Vector<TextureRegion>& keyFrames, uint64_t frameDuration, PlayMode type);
	Animation(const Vector<ShortString>& regionNames, const TextureAtlas& atlas, uint64_t frameDuration = Time::seconds(0.2f).asMicroseconds(), PlayMode type = PlayMode::LOOPED);
	PlayMode getPlayMode() const;
	//NOTE: animation goes on if you call this (and this is maybe not want I want)
	bool isAnimationFinished();
	const Sprite& getKeyFrame();
	void setPlayMode(PlayMode& playMode);
	void restartFrameTimer();
	void pause();
	void resume();
	void restart();
};