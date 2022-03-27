#include "AnimationGltfModel.h"

AnimationGltfModel::AnimationGltfModel(const String& filename, NodeJointMap& nodeJointMap, Vector<Node>& meshNodes,
									   VulkanBuffer& uniformBuffer, Vector<Mat4x4>& inverseBindMatrices, Vector<Vector<uint32_t>>& jointChildrenIndicies)
	: GltfModel(), nodeJointMap(nodeJointMap), meshNodes(meshNodes), uniformBuffer(uniformBuffer), inverseBindMatrices(inverseBindMatrices),
	  jointChildrenIndicies(jointChildrenIndicies)
{
	parseVertices<AnimationGltfModel>(filename);
}

Vector<AnimationGltfModel::Animation> AnimationGltfModel::parseAnimations(const JsonParser::JsonObject& jsonObject) const
{
	Vector<Animation> result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& animationArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	for (uint8_t* it : animationArray)
	{
		Animation animation;
		JsonParser::Object* object = (JsonParser::Object*) it;
		assert(object->type == JsonParser::ObjectType::OBJECT);

		auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
		for (uint8_t* it : properties)
		{
			JsonParser::JsonObject* animationJsonObject = (JsonParser::JsonObject*) it;
			if (animationJsonObject->name == "name")
			{
				assert(animationJsonObject->property.type == JsonParser::ObjectType::STRING);
				new (&animation.name) String(std::get<String>(animationJsonObject->property.jsonUnion));
			}
			else if (animationJsonObject->name == "channels")
			{
				assert(animationJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& channelArray = std::get<Vector<uint8_t*>>(animationJsonObject->property.jsonUnion);
				for (uint8_t* it : channelArray)
				{
					Channel channel;

					JsonParser::Object* channelObject = (JsonParser::Object*) it;
					assert(channelObject->type == JsonParser::ObjectType::OBJECT);
					auto& properties = std::get<Vector<uint8_t*>>(channelObject->jsonUnion);
					for (uint8_t* it : properties)
					{
						JsonParser::JsonObject* channelJsonObject = (JsonParser::JsonObject*) it;
						if (channelJsonObject->name == "sampler")
						{
							assert(channelJsonObject->property.type == JsonParser::ObjectType::INTEGER);
							channel.sampler = (Sampler*)(intptr_t)std::get<int32_t>(channelJsonObject->property.jsonUnion);
						}
						else if (channelJsonObject->name == "target")
						{
							assert(channelJsonObject->property.type == JsonParser::ObjectType::OBJECT);
							auto& properties = std::get<Vector<uint8_t*>>(channelJsonObject->property.jsonUnion);
							for (uint8_t* it : properties)
							{
								JsonParser::JsonObject* targetJsonObject = (JsonParser::JsonObject*) it;
								if (targetJsonObject->name == "node")
								{
									assert(targetJsonObject->property.type == JsonParser::ObjectType::INTEGER);
									channel.target.nodeIndex = std::get<int32_t>(targetJsonObject->property.jsonUnion);
								}
								else if (targetJsonObject->name == "path")
								{
									assert(targetJsonObject->property.type == JsonParser::ObjectType::STRING);
									String& path = std::get<String>(targetJsonObject->property.jsonUnion);
									channel.target.path = utils::getFromString<Channel::Target::Path>(path);
								}
							}
						}
					}

					animation.channles.push_back(std::move(channel));
				}
			}
			else if (animationJsonObject->name == "samplers")
			{
				assert(animationJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& samplerArray = std::get<Vector<uint8_t*>>(animationJsonObject->property.jsonUnion);
				for (uint8_t* it : samplerArray)
				{
					Sampler sampler;

					JsonParser::Object* samplerObject = (JsonParser::Object*) it;
					assert(samplerObject->type == JsonParser::ObjectType::OBJECT);
					auto& properties = std::get<Vector<uint8_t*>>(samplerObject->jsonUnion);
					for (uint8_t* it : properties)
					{
						JsonParser::JsonObject* samplerJsonObject = (JsonParser::JsonObject*) it;
						if (samplerJsonObject->name == "input")
						{
							assert(samplerJsonObject->property.type == JsonParser::ObjectType::INTEGER);
							sampler.inputKeyFrameTimes = (Accessor*)(intptr_t)std::get<int32_t>(samplerJsonObject->property.jsonUnion);
						}
						else if (samplerJsonObject->name == "interpolation")
						{
							assert(samplerJsonObject->property.type == JsonParser::ObjectType::STRING);
							sampler.interpolation = std::get<String>(samplerJsonObject->property.jsonUnion);
						}
						else if (samplerJsonObject->name == "output")
						{
							assert(samplerJsonObject->property.type == JsonParser::ObjectType::INTEGER);
							sampler.outputKeyFrameValues = (Accessor*)(intptr_t)std::get<int32_t>(samplerJsonObject->property.jsonUnion);
						}
					}
					animation.samplers.push_back(std::move(sampler));
				}
			}
		}
		result.push_back(std::move(animation));
	}

	return result;
}

void AnimationGltfModel::decompileAnimations(ParseStruct& parseStruct)
{
	for (auto it = parseStruct.animations.begin(); it != parseStruct.animations.end(); ++it)
	{
		Vector<SkeletalAnimation::Channel> channels;

		for (auto channelIt = it->channles.begin(); channelIt != it->channles.end(); ++channelIt)
		{
			SkeletalAnimation::Channel channel;
			channel.target = (SkeletalAnimation::Target)channelIt->target.path;

			auto nodeMapperId = nodeMeshMap.find(channelIt->target.nodeIndex);
			assert(nodeMapperId != nodeMeshMap.end());
			auto findIt = nodeJointMap.find(nodeMapperId->second);
			assert(findIt != nodeJointMap.end());
			channel.jointIndex = findIt->second.jointIndex;

			auto& keyFrameTimes = channelIt->sampler->inputKeyFrameTimes;
			uint32_t keyFrameCount = keyFrameTimes->count;
			assert(keyFrameTimes->type == Type::SCALAR);
			assert(keyFrameTimes->componentType == ComponentType::FLOAT);
			channel.keyFrames.reserve(keyFrameCount);
			uint8_t* secondData = &keyFrameTimes->bufferView->buffer->data[keyFrameTimes->byteOffset + keyFrameTimes->bufferView->byteOffset];
			uint32_t secondStride = keyFrameTimes->bufferView->byteStride == 0 ? getSizeInBytes(keyFrameTimes->componentType) * getNComponents(keyFrameTimes->type)
				: keyFrameTimes->bufferView->byteStride;

			auto& keyFrameValues = channelIt->sampler->outputKeyFrameValues;
			uint8_t* valueData = &keyFrameValues->bufferView->buffer->data[keyFrameValues->byteOffset + keyFrameValues->bufferView->byteOffset];
			uint32_t valueStride = keyFrameValues->bufferView->byteStride == 0 ? getSizeInBytes(keyFrameValues->componentType) * getNComponents(keyFrameValues->type)
				: keyFrameValues->bufferView->byteStride;
			for (uint32_t i = 0; i < keyFrameCount; ++i)
			{
				float* secondTimes = (float*)secondData;
				SkeletalAnimation::KeyFrame keyFrame;
				keyFrame.second = *secondTimes;
				secondData += secondStride;

				channel.keyFrames.push_back(std::move(keyFrame));
			}
			for (auto keyFrameIt = channel.keyFrames.begin(); keyFrameIt != channel.keyFrames.end(); ++keyFrameIt)
			{
				switch (channel.target)
				{
				case SkeletalAnimation::Target::POS:
				{
					assert(keyFrameValues->type == Type::VEC3);
					assert(keyFrameValues->componentType == ComponentType::FLOAT);
					float* positionData = (float*)valueData;
					float x = *(positionData + 0);
					float y = *(positionData + 1);
					float z = *(positionData + 2);
					keyFrameIt->target.pos = { x, y, z };
					break;
				}
				case SkeletalAnimation::Target::ROT:
				{
					assert(keyFrameValues->type == Type::VEC4);
					assert(keyFrameValues->componentType == ComponentType::FLOAT); // TODO: Handle other componentTypes!
					float* rotationData = (float*)valueData;
					float x = *(rotationData + 0);
					float y = *(rotationData + 1);
					float z = *(rotationData + 2);
					float w = *(rotationData + 3);
					keyFrameIt->target.rot.v.x = x;
					keyFrameIt->target.rot.v.y = y;
					keyFrameIt->target.rot.v.z = z;
					keyFrameIt->target.rot.w = w;
					break;
				}
				case SkeletalAnimation::Target::SCL:
				{
					assert(keyFrameValues->type == Type::VEC3);
					assert(keyFrameValues->componentType == ComponentType::FLOAT);
					float* scaleData = (float*)valueData;
					float x = *(scaleData + 0);
					float y = *(scaleData + 1);
					float z = *(scaleData + 2);
					keyFrameIt->target.scl = { x, y, z };
					break;
				}
				}
				valueData += valueStride;
			}

			channels.push_back(std::move(channel));
		}

		SkeletalAnimation animation(uniformBuffer, std::move(channels), inverseBindMatrices, jointChildrenIndicies);
		animations.emplace(std::move(it->name), std::move(animation));
	}
}

void AnimationGltfModel::mapMeshNodes(ParseStruct& parseStruct)
{
#if DEBUG
	bool addedEntrie = false;
#endif

	for (uint32_t i = 0; i < parseStruct.nodes.size(); ++i)
	{
		for (uint32_t j = 0; j < meshNodes.size(); ++j)
		{
			if (parseStruct.nodes[i].name == meshNodes[j].name)
			{
				nodeMeshMap.emplace(i, j);
#if DEBUG
				addedEntrie = true;
#endif
				break;
			}
		}
#if DEBUG
		assert(addedEntrie == true);
		addedEntrie = false;
#endif
	}
}

void AnimationGltfModel::parseObjects(const JsonParser::JsonObject& jsonObject, ParseStruct& parseStruct) const
{
	GltfModel::parseObjects(jsonObject, (GltfModel::ParseStruct&)parseStruct);

	if (jsonObject.name == "animations")
	{
		parseStruct.animations = parseAnimations(jsonObject);
	}
}

void AnimationGltfModel::fixPointersInParseStruct(ParseStruct& parseStruct) const
{
	GltfModel::fixPointersInParseStruct((GltfModel::ParseStruct&)parseStruct);

	for (auto it = parseStruct.animations.begin(); it != parseStruct.animations.end(); ++it)
	{
		for (auto samplerIt = it->samplers.begin(); samplerIt != it->samplers.end(); ++samplerIt)
		{
			samplerIt->inputKeyFrameTimes = &parseStruct.accessors[(uint32_t)(intptr_t)samplerIt->inputKeyFrameTimes];
			samplerIt->outputKeyFrameValues = &parseStruct.accessors[(uint32_t)(intptr_t)samplerIt->outputKeyFrameValues];
		}

		for (auto channelIt = it->channles.begin(); channelIt != it->channles.end(); ++channelIt)
		{
			channelIt->sampler = &it->samplers[(uint32_t)(intptr_t)channelIt->sampler];
		}
	}
}

void AnimationGltfModel::decompileObjects(ParseStruct& parseStruct, const String& filename)
{
	mapMeshNodes(parseStruct);
	decompileAnimations(parseStruct);
}
