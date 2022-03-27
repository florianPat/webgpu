#include "MeshGltfModel.h"
#include "Globals.h"
#include "Window.h"

MeshGltfModel::MeshGltfModel(const String& filename, Vector<String>&& animationFilenames, Vector<String>&& meshPartExclusions) : GltfModel(),
	meshPartExclusions(std::move(meshPartExclusions))
{
	for (auto it = animationFilenames.begin(); it != animationFilenames.end(); ++it)
	{
		animations.emplace(*it, SkeletalAnimation(uniformBuffer, Vector<SkeletalAnimation::Channel>(), inverseBindMatrices, jointChildrenIndicies));
	}
	parseVertices<MeshGltfModel>(filename);
}

Vector<MeshGltfModel::Mesh> MeshGltfModel::parseMeshes(const JsonParser::JsonObject& jsonObject) const
{
	Vector<Mesh> result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& meshArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	for (uint8_t* it : meshArray)
	{
		Mesh mesh;
		JsonParser::Object* object = (JsonParser::Object*)it;
		assert(object->type == JsonParser::ObjectType::OBJECT);

		auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
		for (uint8_t* it : properties)
		{
			JsonParser::JsonObject* meshJsonObject = (JsonParser::JsonObject*) it;
			if (meshJsonObject->name == "name")
			{
				assert(meshJsonObject->property.type == JsonParser::ObjectType::STRING);
				new (&mesh.name) String(std::get<String>(meshJsonObject->property.jsonUnion));
			}
			else if (meshJsonObject->name == "primitives")
			{
				assert(meshJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& primitiveArray = std::get<Vector<uint8_t*>>(meshJsonObject->property.jsonUnion);
				for (uint8_t* it : primitiveArray)
				{
					JsonParser::Object* object = (JsonParser::Object*) it;
					assert(object->type == JsonParser::ObjectType::OBJECT);
					auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
					for (uint8_t* it : properties)
					{
						JsonParser::JsonObject* primitiveJsonObject = (JsonParser::JsonObject*) it;

						if (primitiveJsonObject->name == "attributes")
						{
							assert(primitiveJsonObject->property.type == JsonParser::ObjectType::OBJECT);
							auto& properties = std::get<Vector<uint8_t*>>(primitiveJsonObject->property.jsonUnion);
							for (uint8_t* it : properties)
							{
								JsonParser::JsonObject* attributeJsonObject = (JsonParser::JsonObject*) it;
								if (attributeJsonObject->name == "POSITION")
								{
									assert(attributeJsonObject->property.type == JsonParser::ObjectType::INTEGER);
									mesh.position = (Accessor*)(intptr_t)std::get<int32_t>(attributeJsonObject->property.jsonUnion);
								}
								else if (attributeJsonObject->name == "TEXCOORD_0")
								{
									assert(attributeJsonObject->property.type == JsonParser::ObjectType::INTEGER);
									mesh.texCoord = (Accessor*)(intptr_t)std::get<int32_t>(attributeJsonObject->property.jsonUnion);
								}
								else if (attributeJsonObject->name == "JOINTS_0")
								{
									assert(attributeJsonObject->property.type == JsonParser::ObjectType::INTEGER);
									mesh.joints = (Accessor*)(intptr_t)std::get<int32_t>(attributeJsonObject->property.jsonUnion);
								}
								else if (attributeJsonObject->name == "WEIGHTS_0")
								{
									assert(attributeJsonObject->property.type == JsonParser::ObjectType::INTEGER);
									mesh.weights = (Accessor*)(intptr_t)std::get<int32_t>(attributeJsonObject->property.jsonUnion);
								}
							}
						}
						else if (primitiveJsonObject->name == "indices")
						{
							assert(primitiveJsonObject->property.type == JsonParser::ObjectType::INTEGER);
							mesh.indicies = (Accessor*)(intptr_t)std::get<int32_t>(primitiveJsonObject->property.jsonUnion);
						}
						else if (primitiveJsonObject->name == "material")
						{
							assert(primitiveJsonObject->property.type == JsonParser::ObjectType::INTEGER);
							mesh.material = (Material*)(intptr_t)std::get<int32_t>(primitiveJsonObject->property.jsonUnion);
						}
					}

					result.push_back(std::move(mesh));
					mesh = result.back();
				}
			}
		}
	}

	return result;
}

Vector<MeshGltfModel::Skin> MeshGltfModel::parseSkins(const JsonParser::JsonObject& jsonObject) const
{
	Vector<Skin> result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& skinArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	for (uint8_t* it : skinArray)
	{
		Skin skin;
		JsonParser::Object* object = (JsonParser::Object*) it;
		assert(object->type == JsonParser::ObjectType::OBJECT);

		auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
		for (uint8_t* it : properties)
		{
			JsonParser::JsonObject* nodeJsonObject = (JsonParser::JsonObject*) it;
			if (nodeJsonObject->name == "joints")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& jointsArray = std::get<Vector<uint8_t*>>(nodeJsonObject->property.jsonUnion);
				for (uint8_t* it : jointsArray)
				{
					JsonParser::Object* jointObject = (JsonParser::Object*) it;
					assert(jointObject->type == JsonParser::ObjectType::INTEGER);
					skin.joints.push_back((Node*)(intptr_t)std::get<int32_t>(jointObject->jsonUnion));
				}
			}
			else if (nodeJsonObject->name == "inverseBindMatrices")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::INTEGER);
				skin.inversedBindMatrices = (Accessor*)(intptr_t)std::get<int32_t>(nodeJsonObject->property.jsonUnion);
			}
		}

		result.push_back(std::move(skin));
	}

	return result;
}

Vector<MeshGltfModel::Material> MeshGltfModel::parseMaterials(const JsonParser::JsonObject& jsonObject) const
{
	Vector<Material> result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& skinArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	for (uint8_t* it : skinArray)
	{
		Material material;
		JsonParser::Object* object = (JsonParser::Object*)it;
		assert(object->type == JsonParser::ObjectType::OBJECT);

		auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
		for (uint8_t* it : properties)
		{
			JsonParser::JsonObject* nodeJsonObject = (JsonParser::JsonObject*)it;
			if (nodeJsonObject->name == "name")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::STRING);
				new (&material.name) String(std::get<String>(nodeJsonObject->property.jsonUnion));
			}
			else if (nodeJsonObject->name == "pbrMetallicRoughness")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::OBJECT);
				auto& properties = std::get<Vector<uint8_t*>>(nodeJsonObject->property.jsonUnion);
				for (uint8_t* it : properties)
				{
					JsonParser::JsonObject* pbrMetallicRoughnessJsonObject = (JsonParser::JsonObject*)it;

					if (pbrMetallicRoughnessJsonObject->name == "baseColorTexture")
					{
						assert(pbrMetallicRoughnessJsonObject->property.type == JsonParser::ObjectType::OBJECT);
						auto& properties = std::get<Vector<uint8_t*>>(pbrMetallicRoughnessJsonObject->property.jsonUnion);
						for (uint8_t* it : properties)
						{
							JsonParser::JsonObject* baseColorTextureJsonObject = (JsonParser::JsonObject*)it;
							if (baseColorTextureJsonObject->name == "index")
							{
								assert(baseColorTextureJsonObject->property.type == JsonParser::ObjectType::INTEGER);
								material.texture = (uint32_t) std::get<int32_t>(baseColorTextureJsonObject->property.jsonUnion);
							}
							else if (baseColorTextureJsonObject->name == "texCoord")
							{
								assert(baseColorTextureJsonObject->property.type == JsonParser::ObjectType::INTEGER);
								material.texCoordIndex = (uint32_t) std::get<int32_t>(baseColorTextureJsonObject->property.jsonUnion);
							}
						}
					}
				}
			}
		}

		result.push_back(std::move(material));
	}

	return result;
}

Vector<uint32_t> MeshGltfModel::decompileVertexBuffer(ParseStruct& parseStruct)
{
	Vector<uint32_t> skipIndexes;
	uint32_t currentSkipIndex = 0;

	uint32_t i = 0;
	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		const Mesh& mesh = *it;

		if (!meshPartExclusions.empty())
		{
			for (auto it = meshPartExclusions.begin(); it != meshPartExclusions.end(); ++it)
			{
				if (mesh.name == *it)
				{
					skipIndexes.push_back(i);
				}
			}

			if (skipIndexes.size() && skipIndexes.back() == i)
			{
				continue;
			}
		}

		nVertices += mesh.position->count;
	}

	new (&vertexBuffer) VulkanBuffer(sizeof(Vertex) * nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, Globals::window->getGfx());
	Vertex* vertices = (Vertex*)vertexBuffer.map(0, VK_WHOLE_SIZE);

	i = 0;
	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		if (skipIndexes.size() > currentSkipIndex && skipIndexes[currentSkipIndex] == i)
		{
			currentSkipIndex++;
			continue;
		}

		const Mesh& mesh = *it;

		const uint32_t meshVertices = mesh.position->count;

		assert(mesh.position->componentType == ComponentType::FLOAT);
		assert(mesh.position->type == Type::VEC3);
		uint32_t positionStride = mesh.position->bufferView->byteStride == 0 ? getSizeInBytes(mesh.position->componentType) * getNComponents(mesh.position->type)
			: mesh.position->bufferView->byteStride;
		uint8_t* positionData = &mesh.position->bufferView->buffer->data[mesh.position->byteOffset + mesh.position->bufferView->byteOffset];

		assert(mesh.texCoord->type == Type::VEC2);
		uint32_t texCoordStride = mesh.texCoord->bufferView->byteStride == 0 ? getSizeInBytes(mesh.texCoord->componentType) * getNComponents(mesh.texCoord->type)
			: mesh.texCoord->bufferView->byteStride;
		uint8_t* texCoordData = &mesh.texCoord->bufferView->buffer->data[mesh.texCoord->byteOffset + mesh.texCoord->bufferView->byteOffset];

		uint32_t jointStride = 0;
		uint8_t* jointData = nullptr;
		uint32_t weightStride = 0;
		uint8_t* weightData = nullptr;
		if (mesh.joints != nullptr && mesh.weights != nullptr)
		{
			assert(mesh.joints->componentType == ComponentType::USHORT);
			assert(mesh.joints->type == Type::VEC4);
			jointStride = mesh.joints->bufferView->byteStride == 0 ? getSizeInBytes(mesh.joints->componentType) * getNComponents(mesh.joints->type)
				: mesh.joints->bufferView->byteStride;
			jointData = &mesh.joints->bufferView->buffer->data[mesh.joints->byteOffset + mesh.joints->bufferView->byteOffset];

			assert(mesh.weights->componentType == ComponentType::FLOAT);
			assert(mesh.weights->type == Type::VEC4);
			weightStride = mesh.weights->bufferView->byteStride == 0 ? getSizeInBytes(mesh.weights->componentType) * getNComponents(mesh.weights->type)
				: mesh.weights->bufferView->byteStride;
			weightData = &mesh.weights->bufferView->buffer->data[mesh.weights->byteOffset + mesh.weights->bufferView->byteOffset];
		}

		for (uint32_t i = 0; i < meshVertices; ++i)
		{
			float* positions = (float*)positionData;
			float x = *(positions + 0);
			float y = *(positions + 1);
			float z = *(positions + 2);
			vertices->pos = { x, y, z };
			positionData += positionStride;

			float u = 0.0f, v = 0.0f;
			switch (mesh.texCoord->componentType)
			{
				case ComponentType::FLOAT:
				{
					float* texCoords = (float*)texCoordData;
					u = *(texCoords + 0);
					v = *(texCoords + 1);
					break;
				}
				case ComponentType::USHORT:
				{
					uint16_t* texCoords = (uint16_t*)texCoordData;
					u = (*(texCoords + 0)) / 65535.0f;
					v = (*(texCoords + 1)) / 65535.0f;
					break;
				}
				case ComponentType::UBYTE:
				{
					uint8_t* texCoords = (uint8_t*)texCoordData;
					u = (*(texCoords + 0)) / 255.0f;
					v = (*(texCoords + 1)) / 255.0f;
					break;
				}
				default:
				{
					InvalidCodePath;
					break;
				}
			}
			vertices->texCoord = { u, v };
			texCoordData += texCoordStride;

			if (jointData != nullptr)
			{
				assert(weightData != nullptr);
				// NOTE: This works because there is only one skin in this mesh. If there a more than one, this breaks!! RISKIY!!
				uint16_t* jointIndicies = (uint16_t*)jointData;
				float* weights = (float*)weightData;
				for (uint32_t j = 0; j < 4; ++j)
				{
					vertices->jointIndices[j] = (uint32_t)jointIndicies[j];
					vertices->weights[j] = weights[j];
				}

				jointData += jointStride;
				weightData += weightStride;
			}
			else
			{
				for (uint32_t j = 0; j < 4; ++j)
				{
					vertices->jointIndices[j] = 0;
					vertices->weights[j] = 0;
				}
				vertices->weights[0] = 1.0f;
			}

			++vertices;
		}
	}
	vertexBuffer.unmap(0, VK_WHOLE_SIZE);

	return skipIndexes;
}

void MeshGltfModel::decompileIndexBuffer(ParseStruct& parseStruct, const Vector<uint32_t>& meshSkipIndexes)
{
	uint32_t currentSkipIndex = 0;
	uint32_t i = 0;

	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		if (meshSkipIndexes.size() > currentSkipIndex && meshSkipIndexes[currentSkipIndex] == i)
		{
			++currentSkipIndex;
			continue;
		}

		const Mesh& mesh = *it;
		nIndices += mesh.indicies->count;
	}

	new (&indexBuffer) VulkanBuffer(sizeof(uint16_t) * nIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Globals::window->getGfx());
	uint16_t* indicies = (uint16_t*)indexBuffer.map(0, VK_WHOLE_SIZE);

	uint16_t indicesOffset = 0;
	i = 0, currentSkipIndex = 0;
	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		if (meshSkipIndexes.size() > currentSkipIndex && meshSkipIndexes[currentSkipIndex] == i)
		{
			++currentSkipIndex;
			continue;
		}

		const Mesh& mesh = *it;

		const uint32_t meshIndicies = mesh.indicies->count;
		const uint16_t meshVertices = (uint16_t)mesh.position->count;

		assert(mesh.indicies->componentType == ComponentType::USHORT);
		assert(mesh.indicies->type == Type::SCALAR);
		uint32_t indicesStride = mesh.indicies->bufferView->byteStride == 0 ? getSizeInBytes(mesh.indicies->componentType) * getNComponents(mesh.indicies->type)
			: mesh.indicies->bufferView->byteStride;
		uint8_t* indiciesData = &mesh.indicies->bufferView->buffer->data[mesh.indicies->byteOffset + mesh.indicies->bufferView->byteOffset];
		assert(nIndices % 3 == 0);
		for (uint32_t i = 0; i < meshIndicies;)
		{
			*(indicies + 0) = *((uint16_t*)indiciesData) + indicesOffset;
			indiciesData += indicesStride;

			*(indicies + 2) = *((uint16_t*)indiciesData) + indicesOffset;
			indiciesData += indicesStride;

			*(indicies + 1) = *((uint16_t*)indiciesData) + indicesOffset;
			indiciesData += indicesStride;

			indicies += 3;
			i += 3;
		}
		indicesOffset += meshVertices;
	}

	indexBuffer.unmap(0, VK_WHOLE_SIZE);
}

Vector<Mat4x4> MeshGltfModel::decompileSkinBuffer(ParseStruct& parseStruct)
{
	if (parseStruct.skins.size() == 0)
	{
		return Vector<Mat4x4>();
	}

	// TODO: Implement multiple skins
	assert(parseStruct.skins.size() == 1);
	Skin& skin = parseStruct.skins[0];
	uint32_t nInverseBindMatrices = skin.inversedBindMatrices->count;
	Vector<Mat4x4> inverseBindMatrices(nInverseBindMatrices);
	assert(skin.inversedBindMatrices->componentType == ComponentType::FLOAT);
	assert(skin.inversedBindMatrices->type == Type::MAT4);
	uint32_t inverseBindMatrixStride = skin.inversedBindMatrices->bufferView->byteStride == 0 ? getSizeInBytes(skin.inversedBindMatrices->componentType) *
		getNComponents(skin.inversedBindMatrices->type) : skin.inversedBindMatrices->bufferView->byteStride;
	uint8_t* inverseBindMatrixData = &skin.inversedBindMatrices->bufferView->buffer->data[skin.inversedBindMatrices->byteOffset + skin.inversedBindMatrices->bufferView->byteOffset];
	for (uint32_t i = 0; i < nInverseBindMatrices; ++i)
	{
		float* inverseBindMatrix = (float*)inverseBindMatrixData;

		Mat4x4 matrix;
		for (uint32_t j = 0; j < arrayCount(matrix.matrix); ++j, ++inverseBindMatrix)
		{
			matrix.matrix[j] = *inverseBindMatrix;
		}
		inverseBindMatrices.push_back(matrix);

		inverseBindMatrixData += inverseBindMatrixStride;
	}
	skin.joints[0]->transform = Mat4x4::translate(skin.joints[0]->pos) * Mat4x4::rotate(skin.joints[0]->rot) * Mat4x4::scale(skin.joints[0]->scl);
	computeGlobalJointTransform(skin.joints[0], parseStruct.nodes);
	assert(skin.joints.size() == inverseBindMatrices.size());
	assert(skin.joints.size() <= SkeletalAnimation::MAX_JOINT_COUNT);
	new (&uniformBuffer) VulkanBuffer(skin.joints.size() * sizeof(Mat4x4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		Globals::window->getGfx());
	Mat4x4* jointMatrices = (Mat4x4*)uniformBuffer.map(0, VK_WHOLE_SIZE);
	for (uint32_t i = 0; i < skin.joints.size(); ++i)
	{
		new (&jointMatrices[i]) Mat4x4(skin.joints[i]->transform * inverseBindMatrices[i]);
	}
	uniformBuffer.unmap(0, VK_WHOLE_SIZE);

	intptr_t nodeStart = (intptr_t)parseStruct.nodes.data();
	createNodeJointMap(skin, nodeStart);

	return inverseBindMatrices;
}

void MeshGltfModel::parseObjects(const JsonParser::JsonObject& jsonObject, ParseStruct& parseStruct) const
{
	GltfModel::parseObjects(jsonObject, (GltfModel::ParseStruct&)parseStruct);

	if (jsonObject.name == "meshes")
	{
		parseStruct.meshes = parseMeshes(jsonObject);
	}
	else if (jsonObject.name == "skins")
	{
		parseStruct.skins = parseSkins(jsonObject);
	}
	else if (jsonObject.name == "materials")
	{
		parseStruct.materials = parseMaterials(jsonObject);
	}
}

void MeshGltfModel::fixPointersInParseStruct(ParseStruct& parseStruct) const
{
	GltfModel::fixPointersInParseStruct((GltfModel::ParseStruct&)parseStruct);

	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it)
	{
		it->position = &parseStruct.accessors[(uint32_t)(intptr_t)it->position];
		it->texCoord = &parseStruct.accessors[(uint32_t)(intptr_t)it->texCoord];
		it->indicies = &parseStruct.accessors[(uint32_t)(intptr_t)it->indicies];
		if (it->joints != nullptr && it->weights != nullptr)
		{
			it->joints = &parseStruct.accessors[(uint32_t)(intptr_t)it->joints]; 
			it->weights = &parseStruct.accessors[(uint32_t)(intptr_t)it->weights];
		}
		it->material = &parseStruct.materials[(uint32_t)(intptr_t)it->material];
	}

	for (auto it = parseStruct.skins.begin(); it != parseStruct.skins.end(); ++it)
	{
		it->inversedBindMatrices = &parseStruct.accessors[(uint32_t)(intptr_t)it->inversedBindMatrices];
		for (uint32_t i = 0; i < it->joints.size(); ++i)
		{
			it->joints[i] = &parseStruct.nodes[(uint32_t)(intptr_t)it->joints[i]];
		}
	}
}

void MeshGltfModel::decompileObjects(ParseStruct& parseStruct, const String& filename)
{
	GltfModel::decompileObjects((GltfModel::ParseStruct&)parseStruct, filename);

	Vector<uint32_t> meshSkipIndexes = decompileVertexBuffer(parseStruct);
	decompileIndexBuffer(parseStruct, meshSkipIndexes);
	inverseBindMatrices = decompileSkinBuffer(parseStruct);
	decompileAnimation(parseStruct);
}

void MeshGltfModel::createNodeJointMap(Skin& skin, intptr_t nodeStart)
{
	for (uint32_t i = 0; i < skin.joints.size(); ++i)
	{
		intptr_t jointNodeAddress = (intptr_t)skin.joints[i];
		int32_t skinNodeIndex = (int32_t)((jointNodeAddress - nodeStart) / sizeof(Node));
		nodeJointMap.emplace(skinNodeIndex, NodeJointMapValue{ i, skin.joints[i]->name });
	}
	jointChildrenIndicies = Vector<Vector<uint32_t>>(skin.joints.size());
	for (uint32_t i = 0; i < skin.joints.size(); ++i)
	{
		Node* skinNode = skin.joints[i];
		Vector<uint32_t> childrenIndices(skinNode->children.size());
		for (auto childrenIt = skinNode->children.begin(); childrenIt != skinNode->children.end(); ++childrenIt)
		{
			auto findIt = nodeJointMap.find(*childrenIt);
			if (findIt != nodeJointMap.end())
			{
				childrenIndices.push_back(findIt->second.jointIndex);
			}
		}
		jointChildrenIndicies.push_back(std::move(childrenIndices));
	}
}

void MeshGltfModel::decompileAnimation(ParseStruct& parseStruct)
{
	auto it = animations.begin();
	uint32_t size = (uint32_t)animations.size();
	for (uint32_t i = 0; i < size; ++i)
	{
		AnimationGltfModel animation(it->first, nodeJointMap, parseStruct.nodes, uniformBuffer, inverseBindMatrices, jointChildrenIndicies);
		if (animation.animations.size() == 1)
		{
			it->second.~SkeletalAnimation();
			new (&it->second) SkeletalAnimation(std::move(animation.animations.begin()->second));
			++it;
			continue;
		}
		it = animations.erase(it);
		for (auto animationIt = animation.animations.begin(); animationIt != animation.animations.end(); ++animationIt)
		{
			animations.emplace(std::move(animationIt->first), std::move(animationIt->second));
		}
	}
}

void MeshGltfModel::computeGlobalJointTransform(Node* node, Vector<Node>& nodes) const
{
	for (auto it = node->children.begin(); it != node->children.end(); ++it)
	{
		Node* child = &nodes[*it];
		child->transform = node->transform * Mat4x4::translate(child->pos) * Mat4x4::rotate(-child->rot) * Mat4x4::scale(child->scl);

		computeGlobalJointTransform(child, nodes);
	}
}