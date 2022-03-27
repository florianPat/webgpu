#pragma once

#include "GltfModel.h"

class AnimationGltfModel : public GltfModel
{
protected:
	struct Sampler
	{
		Accessor* inputKeyFrameTimes = nullptr, * outputKeyFrameValues = nullptr;
		String interpolation = "LINEAR";
	};
	struct Channel
	{
		Sampler* sampler = nullptr;
		struct Target
		{
			enum class Path
			{
				TRANSLATION = utils::getIntFromChars('t', 'n'),
				ROTATION = utils::getIntFromChars('r', 'n'),
				SCALE = utils::getIntFromChars('s', 'e'),
				WEIGHTS = utils::getIntFromChars('w', 's'),
			};

			int32_t nodeIndex = 0;
			Path path = Path::TRANSLATION;
		} target;
	};
	struct Animation
	{
		String name = "";
		Vector<Channel> channles;
		Vector<Sampler> samplers;
	};
public:
	struct ParseStruct : public GltfModel::ParseStruct
	{
		Vector<Animation> animations;
	};
protected:
	NodeJointMap& nodeJointMap;
	std::unordered_map<uint32_t, uint32_t> nodeMeshMap;
	Vector<Node>& meshNodes;
	VulkanBuffer& uniformBuffer;
	Vector<Mat4x4>& inverseBindMatrices;
	Vector<Vector<uint32_t>>& jointChildrenIndicies;
public:
	std::unordered_map<String, SkeletalAnimation> animations;
public:
	AnimationGltfModel(const String& filename, NodeJointMap& nodeJointMap, Vector<Node>& meshNodes,
		VulkanBuffer& uniformBuffer, Vector<Mat4x4>& inverseBindMatrices, Vector<Vector<uint32_t>>& jointChildrenIndicies);
protected:
	Vector<Animation> parseAnimations(const JsonParser::JsonObject& jsonObject) const;
	void decompileAnimations(ParseStruct& parseStruct);
	void mapMeshNodes(ParseStruct& parseStruct);

	friend class GltfModel;
	void parseObjects(const JsonParser::JsonObject& jsonObject, ParseStruct& parseStruct) const;
	void fixPointersInParseStruct(ParseStruct& parseStruct) const;
	void decompileObjects(ParseStruct& parseStruct, const String& filename);
};