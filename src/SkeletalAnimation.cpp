#include "SkeletalAnimation.h"
#include "Globals.h"
#include "Window.h"
#include "GltfModel.h"

void SkeletalAnimation::computeGlobalJointTransform(uint32_t parentJointIndex)
{
	for (auto childrenIndex = jointChildrenIndicies[parentJointIndex].begin(); childrenIndex != jointChildrenIndicies[parentJointIndex].end(); ++childrenIndex)
	{
		absoluteJointTransforms[*childrenIndex] = absoluteJointTransforms[parentJointIndex] * Mat4x4::translate(jointVectors[*childrenIndex].pos) *
			Mat4x4::rotate(-jointVectors[*childrenIndex].rot) * Mat4x4::scale(jointVectors[*childrenIndex].scl);
		computeGlobalJointTransform(*childrenIndex);
	}
}

Vector3f SkeletalAnimation::applyBaseTranslation(const Vector3f& basePos)
{
	Vector3f result = basePos;

	if (baseTranslation.x != 0.0f)
	{
		result.x = baseTranslation.x;
	}
	if (baseTranslation.y != 0.0f)
	{
		result.y = baseTranslation.y;
	}
	if (baseTranslation.z != 0.0f)
	{
		result.z = baseTranslation.z;
	}

	return result;
}

SkeletalAnimation::KeyFrameResult  SkeletalAnimation::stepKeyFrameForward(float dt, Iterator<Channel>& it)
{
	it->currentTime += dt;
	float frameDuration = it->keyFrames[it->keyFrameIt + 1].second - it->keyFrames[it->keyFrameIt].second;
	while (it->currentTime > frameDuration)
	{
		it->currentTime -= frameDuration;
		++it->keyFrameIt;
		if (it->keyFrameIt >= (it->keyFrames.size() - 2))
		{
			if (playMode == Animation::PlayMode::LOOPED)
			{
				it->keyFrameIt = 0;
			}
		}
		frameDuration = it->keyFrames[it->keyFrameIt + 1].second - it->keyFrames[it->keyFrameIt].second;
	}

	return {it->keyFrameIt, it->keyFrameIt + 1};
}

SkeletalAnimation::KeyFrameResult SkeletalAnimation::stepKeyFrameBackward(float dt, Iterator<Channel>& it)
{
	it->currentTime += dt;
	float frameDuration = it->keyFrames[it->keyFrameIt + 1].second - it->keyFrames[it->keyFrameIt].second;
	while (it->currentTime > frameDuration)
	{
		it->currentTime -= frameDuration;
		--it->keyFrameIt;
		if (it->keyFrameIt == 0)
		{
			if (playMode == Animation::PlayMode::LOOP_REVERSED)
			{
				it->keyFrameIt = it->keyFrames.size() - 2;
			}
		}
		frameDuration = it->keyFrames[it->keyFrameIt + 1].second - it->keyFrames[it->keyFrameIt].second;
	}

	return {it->keyFrameIt, it->keyFrameIt + 1};
}

SkeletalAnimation::SkeletalAnimation(VulkanBuffer& jointUniformBuffer, Vector<Channel>&& channelsIn, Vector<Mat4x4>& inverseBindMatrices,
	Vector<Vector<uint32_t>>& jointChildrenIndicies, Animation::PlayMode playMode)
	: playMode(playMode), uniformBuffer(jointUniformBuffer), inverseBindMatrices(inverseBindMatrices), jointChildrenIndicies(jointChildrenIndicies),
	  jointVectors(inverseBindMatrices.size()), absoluteJointTransforms(inverseBindMatrices.size(), Mat4x4::identity()),
	  channels(std::move(channelsIn)), clock()
{
	PosRotScl initializeValue = {};
	initializeValue.pos = { 0.0f, 0.0f, 0.0f };
	initializeValue.rot = { {0.0f, 0.0f, 0.0f}, 1.0f };
	initializeValue.scl = { 1.0f, 1.0f, 1.0f };
	for (uint32_t i = 0; i < inverseBindMatrices.size(); ++i)
	{
		jointVectors.push_back(initializeValue);
	}
}

void SkeletalAnimation::updateJoints()
{
	float dt = clock.getTime().asSeconds();

	auto* joints = (Mat4x4*)uniformBuffer.map(0, VK_WHOLE_SIZE);

	for (auto it = channels.begin(); it != channels.end(); ++it)
	{
		if (!paused)
		{
			PosRotScl& posRotScl = jointVectors[it->jointIndex];
			if (it->keyFrames.size() == 1)
			{
				switch (it->target)
				{
					case Target::POS:
					{
						posRotScl.pos = it->keyFrames[0].target.pos;
						break;
					}
					case Target::ROT:
					{
						posRotScl.rot = it->keyFrames[0].target.rot;
						break;
					}
					case Target::SCL:
					{
						posRotScl.scl = it->keyFrames[0].target.scl;
						break;
					}
				}
				continue;
			}

			KeyFrameResult keyFrameResult;
			switch (playMode)
			{
				case Animation::PlayMode::NORMAL:
				{
					if (it->keyFrameIt >= (it->keyFrames.size() - 2))
					{
						keyFrameResult.currenKeyFrameIt = it->keyFrameIt;
						keyFrameResult.nextKeyFrameIt = it->keyFrameIt + 1;
						break;
					}
				}
				case Animation::PlayMode::LOOPED:
				{
					keyFrameResult = stepKeyFrameForward(dt, it);
					break;
				}
				case Animation::PlayMode::REVERSED:
				{
					if (it->keyFrameIt == 0)
					{
						keyFrameResult.currenKeyFrameIt = 0;
						keyFrameResult.nextKeyFrameIt = 1;
						break;
					}
				}
				case Animation::PlayMode::LOOP_REVERSED:
				{
					keyFrameResult = stepKeyFrameBackward(dt, it);
					break;
				}
				default:
				{
					InvalidCodePath;
				}
			}

			auto& currentKeyFrame = it->keyFrames[keyFrameResult.currenKeyFrameIt];
			auto& nextKeyFrame = it->keyFrames[keyFrameResult.nextKeyFrameIt];
			float animationTime = nextKeyFrame.second - currentKeyFrame.second;
			float t = it->currentTime / animationTime;

			switch (it->target)
			{
				case Target::POS:
				{
					Vector3f pos;
					pos.x = utils::lerp(currentKeyFrame.target.pos.x, nextKeyFrame.target.pos.x, t);
					pos.y = utils::lerp(currentKeyFrame.target.pos.y, nextKeyFrame.target.pos.y, t);
					pos.z = utils::lerp(currentKeyFrame.target.pos.z, nextKeyFrame.target.pos.z, t);
					posRotScl.pos = pos;
					break;
				}
				case Target::ROT:
				{
					posRotScl.rot = currentKeyFrame.target.rot.slerp(nextKeyFrame.target.rot, t).getNormalized();
					break;
				}
				case Target::SCL:
				{
					Vector3f scl;
					scl.x = utils::lerp(currentKeyFrame.target.scl.x, nextKeyFrame.target.scl.x, t);
					scl.y = utils::lerp(currentKeyFrame.target.scl.y, nextKeyFrame.target.scl.y, t);
					scl.z = utils::lerp(currentKeyFrame.target.scl.z, nextKeyFrame.target.scl.z, t);
					posRotScl.scl = scl;
					break;
				}
			}
		}
	}
	Vector3f baseTranslation = applyBaseTranslation(jointVectors[0].pos);
	absoluteJointTransforms[0] = Mat4x4::translate(baseTranslation) * Mat4x4::rotate(-jointVectors[0].rot) * Mat4x4::scale(jointVectors[0].scl);
	computeGlobalJointTransform();
	for (uint32_t i = 0; i < inverseBindMatrices.size(); ++i)
	{
		joints[i] = absoluteJointTransforms[i] * inverseBindMatrices[i];
	}
	uniformBuffer.unmap(0, VK_WHOLE_SIZE);
}

Animation::PlayMode SkeletalAnimation::getPlayMode() const
{
	return playMode;
}

bool SkeletalAnimation::isAnimationFinished() const
{
	bool result = false;

	switch (playMode)
	{
		case Animation::PlayMode::NORMAL:
		{
			for (auto it = channels.begin(); it != channels.end(); ++it)
			{
				if (it->keyFrameIt >= (it->keyFrames.size() - 2))
				{
					result = true;
				}
				else
				{
					result = false;
				}
			}
			break;
		}
		case Animation::PlayMode::REVERSED:
		{
			for (auto it = channels.begin(); it != channels.end(); ++it)
			{
				if (it->keyFrameIt == 0)
				{
					result = true;
				}
				else
				{
					result = false;
				}
			}
			break;
		}
	}

	return result;
}

void SkeletalAnimation::setPlayMode(Animation::PlayMode playModeIn)
{
	playMode = playModeIn;
}

void SkeletalAnimation::pause()
{
	paused = true;
}

void SkeletalAnimation::resume()
{
	paused = false;
}

void SkeletalAnimation::restart()
{
	for (auto it = channels.begin(); it != channels.end(); ++it)
	{
		it->currentTime = 0.0f;
		it->keyFrameIt = 0;
	}
	clock.restart();
}
