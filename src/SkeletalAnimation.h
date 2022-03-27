#pragma once

#include "Clock.h"
#include "Quaternion.h"
#include "Vector.h"
#include "Animation.h"
#include "VulkanBuffer.h"

class SkeletalAnimation
{
	struct PosRotScl
	{
		Vector3f pos;
		Quaternion rot;
		Vector3f scl;
	};
private:
	bool paused = false;
	Animation::PlayMode playMode;
	VulkanBuffer& uniformBuffer;
	Vector<Mat4x4>& inverseBindMatrices;
	Vector<Vector<uint32_t>>& jointChildrenIndicies;
	Vector<PosRotScl> jointVectors;
	Vector<Mat4x4> absoluteJointTransforms;
public:
	struct KeyFrame
	{
		float second = 0.0f;
		union Target {
			Vector3f pos;
			Quaternion rot;
			Vector3f scl;
			Target() : pos() {}
			~Target() {};
		} target;
	};
	enum class Target
	{
		POS = utils::getIntFromChars('t', 'n'),
		ROT = utils::getIntFromChars('r', 'n'),
		SCL = utils::getIntFromChars('s', 'e'),
	};
	struct Channel
	{
		uint32_t jointIndex = 0;
		uint32_t keyFrameIt = 0;
		Vector<KeyFrame> keyFrames;
		Target target = Target::POS;
		float currentTime = 0.0f;
	};
	struct KeyFrameResult
	{
		uint32_t currenKeyFrameIt = 0;
		uint32_t nextKeyFrameIt = 0;
	};
	static constexpr uint32_t MAX_JOINT_COUNT = 128;
public:
	// NOTE: Used for animations who have an "integrated walk speed"
	Vector3f baseTranslation;
private:
	Vector<Channel> channels;
	Clock clock;
private:
	void computeGlobalJointTransform(uint32_t parentIndex = 0);
	Vector3f applyBaseTranslation(const Vector3f& basePos);
	KeyFrameResult stepKeyFrameForward(float dt, Iterator<Channel>& it);
	KeyFrameResult stepKeyFrameBackward(float dt, Iterator<Channel>& it);
public:
	SkeletalAnimation(VulkanBuffer& jointUniformBuffer, Vector<Channel>&& channels, Vector<Mat4x4>& inverseBindMatrices, Vector<Vector<uint32_t>>& jointChildrenIndicies,
		Animation::PlayMode playMode = Animation::PlayMode::LOOPED);
	void updateJoints();
	void setPlayMode(Animation::PlayMode playMode);
	void pause();
	void resume();
	void restart();
	Animation::PlayMode getPlayMode() const;
	bool isAnimationFinished() const;
};