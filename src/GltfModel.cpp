#include "GltfModel.h"
#include "Ifstream.h"
#include "Window.h"
#include "Globals.h"

ShortString GltfModel::parseIntToAscii(uint8_t* readBytes, uint32_t offset) const
{
	ShortString result = "";

	for (uint32_t i = 0; i < sizeof(uint32_t); ++i)
	{
		result += readBytes[offset + i];
	}

	return result;
}

void GltfModel::parseHeader(Ifstream& file) const
{
	uint8_t header[sizeof(uint32_t) * 3];
	file.read(&header, sizeof(header));
	String magic = parseIntToAscii(header, 0);
	uint32_t* version = (uint32_t*)&header[sizeof(uint32_t)];
	//uint32_t* length = (uint32_t*)&header[sizeof(uint32_t) * 2];

	assert(magic == "glTF");
	assert((*version) == 2);
}

GltfModel::ChunkHeader GltfModel::parseChunkHeader(Ifstream& file) const
{
	uint8_t chunkHeader[sizeof(uint32_t) * 2];
	file.read(&chunkHeader, sizeof(chunkHeader));
	uint32_t* chunkLength = (uint32_t*)&chunkHeader[0];
	assert(((*chunkLength) % 4) == 0);
	return { *chunkLength, parseIntToAscii(chunkHeader, sizeof(uint32_t)) };
}

void GltfModel::parseBin(Ifstream& file, ParseStruct& parseStruct) const
{
	ChunkHeader chunkHeader = parseChunkHeader(file);
	assert(chunkHeader.type == "BIN");

	parseStruct.buffer.data = (uint8_t*)malloc(chunkHeader.length);
	file.read(parseStruct.buffer.data, chunkHeader.length);
}

Vector<GltfModel::BufferView> GltfModel::parseBufferViews(const JsonParser::JsonObject& jsonObject) const
{
	Vector<BufferView> result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& bufferViewArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	for (uint8_t* it : bufferViewArray)
	{
		BufferView bufferView;
		JsonParser::Object* object = (JsonParser::Object*)it;
		assert(object->type == JsonParser::ObjectType::OBJECT);

		auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
		for (uint8_t* it : properties)
		{
			JsonParser::JsonObject* bufferViewJsonObject = (JsonParser::JsonObject*) it;
			assert(bufferViewJsonObject->property.type == JsonParser::ObjectType::INTEGER);
			if (bufferViewJsonObject->name == "buffer")
			{
				bufferView.buffer = (Buffer*) (intptr_t) std::get<int32_t>(bufferViewJsonObject->property.jsonUnion);
			}
			else if(bufferViewJsonObject->name == "byteLength")
			{
				bufferView.byteLength = std::get<int32_t>(bufferViewJsonObject->property.jsonUnion);
			}
			else if (bufferViewJsonObject->name == "byteOffset")
			{
				bufferView.byteOffset = std::get<int32_t>(bufferViewJsonObject->property.jsonUnion);
			}
			else if (bufferViewJsonObject->name == "byteStride")
			{
				bufferView.byteStride = std::get<int32_t>(bufferViewJsonObject->property.jsonUnion);
			}
			else if (bufferViewJsonObject->name == "target")
			{
				bufferView.target = std::get<int32_t>(bufferViewJsonObject->property.jsonUnion);
			}
		}

		result.push_back(std::move(bufferView));
	}

	return result;
}

Vector<GltfModel::Accessor> GltfModel::parseAccessors(const JsonParser::JsonObject& jsonObject) const
{
	Vector<Accessor> result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& bufferViewArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	for (uint8_t* it : bufferViewArray)
	{
		Accessor accessor;
		JsonParser::Object* object = (JsonParser::Object*) it;
		assert(object->type == JsonParser::ObjectType::OBJECT);

		auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
		for (uint8_t* it : properties)
		{
			JsonParser::JsonObject* accessorJsonObject = (JsonParser::JsonObject*) it;
			if (accessorJsonObject->name == "bufferView")
			{
				assert(accessorJsonObject->property.type == JsonParser::ObjectType::INTEGER);
				accessor.bufferView = (BufferView*) (intptr_t) std::get<int32_t>(accessorJsonObject->property.jsonUnion);
			}
			else if (accessorJsonObject->name == "byteOffset")
			{
				assert(accessorJsonObject->property.type == JsonParser::ObjectType::INTEGER);
				accessor.byteOffset = std::get<int32_t>(accessorJsonObject->property.jsonUnion);
			}
			else if (accessorJsonObject->name == "componentType")
			{
				assert(accessorJsonObject->property.type == JsonParser::ObjectType::INTEGER);
				accessor.componentType = (ComponentType) std::get<int32_t>(accessorJsonObject->property.jsonUnion);
			}
			else if (accessorJsonObject->name == "count")
			{
				assert(accessorJsonObject->property.type == JsonParser::ObjectType::INTEGER);
				accessor.count = std::get<int32_t>(accessorJsonObject->property.jsonUnion);
			}
			else if (accessorJsonObject->name == "type")
			{
				assert(accessorJsonObject->property.type == JsonParser::ObjectType::STRING);
				String& type = std::get<String>(accessorJsonObject->property.jsonUnion);
				accessor.type = utils::getFromString<Type>(type);
			}
		}

		result.push_back(std::move(accessor));
	}

	return result;
}

GltfModel::Buffer GltfModel::parseBuffer(const JsonParser::JsonObject& jsonObject) const
{
	Buffer result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& bufferViewArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	assert(bufferViewArray.size() == 1);

	JsonParser::Object* object = (JsonParser::Object*) bufferViewArray[0];
	assert(object->type == JsonParser::ObjectType::OBJECT);
	auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
	assert(properties.size() == 1);

	JsonParser::JsonObject* bufferJsonObject = (JsonParser::JsonObject*) properties[0];
	assert(bufferJsonObject->name == "byteLength");
	assert(bufferJsonObject->property.type == JsonParser::ObjectType::INTEGER);
	result.byteLength = std::get<int32_t>(bufferJsonObject->property.jsonUnion);

	return result;
}

Vector<GltfModel::Node> GltfModel::parseNodes(const JsonParser::JsonObject& jsonObject) const
{
	Vector<Node> result;

	assert(jsonObject.property.type == JsonParser::ObjectType::ARRAY);
	auto& nodeArray = std::get<Vector<uint8_t*>>(jsonObject.property.jsonUnion);
	for (uint8_t* it : nodeArray)
	{
		Node node;
		JsonParser::Object* object = (JsonParser::Object*) it;
		assert(object->type == JsonParser::ObjectType::OBJECT);

		auto& properties = std::get<Vector<uint8_t*>>(object->jsonUnion);
		Vector3f translation;
		Vector3f scale = { 1.0f, 1.0f, 1.0f };
		Quaternion rotation;
		for (uint8_t* it : properties)
		{
			JsonParser::JsonObject* nodeJsonObject = (JsonParser::JsonObject*) it;
			if (nodeJsonObject->name == "matrix")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& matrixArray = std::get<Vector<uint8_t*>>(nodeJsonObject->property.jsonUnion);
				assert(matrixArray.size() == 16);
				Mat4x4 localTransform;
				for (uint32_t i = 0; i < matrixArray.size(); ++i)
				{
					localTransform.matrix[i] = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[i]);
				}
			}
			else if (nodeJsonObject->name == "translation")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& matrixArray = std::get<Vector<uint8_t*>>(nodeJsonObject->property.jsonUnion);
				assert(matrixArray.size() == 3);
				translation.x = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[0]);
				translation.y = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[1]);
				translation.z = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[2]);
			}
			else if (nodeJsonObject->name == "scale")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& matrixArray = std::get<Vector<uint8_t*>>(nodeJsonObject->property.jsonUnion);
				assert(matrixArray.size() == 3);
				scale.x = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[0]);
				scale.y = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[1]);
				scale.z = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[2]);
			}
			else if (nodeJsonObject->name == "rotation")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& matrixArray = std::get<Vector<uint8_t*>>(nodeJsonObject->property.jsonUnion);
				assert(matrixArray.size() == 4);
				rotation.v.x = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[0]);
				rotation.v.y = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[1]);
				rotation.v.z = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[2]);
				rotation.w = JsonParser::getFloatFromNumericObject((JsonParser::Object*) matrixArray[3]);
			}
			else if (nodeJsonObject->name == "children")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::ARRAY);
				auto& matrixArray = std::get<Vector<uint8_t*>>(nodeJsonObject->property.jsonUnion);
				for (uint8_t* it : matrixArray)
				{
					JsonParser::Object* element = (JsonParser::Object*) it;
					assert(element->type == JsonParser::ObjectType::INTEGER);
					node.children.push_back(std::get<int32_t>(element->jsonUnion));
				}
			}
			else if (nodeJsonObject->name == "name")
			{
				assert(nodeJsonObject->property.type == JsonParser::ObjectType::STRING);
				new (&node.name) String(std::move(std::get<String>(nodeJsonObject->property.jsonUnion)));
			}
		}

		node.pos = translation;
		node.rot = rotation;
		node.scl = scale;

		result.push_back(std::move(node));
	}

	return result;
}

uint32_t GltfModel::getSizeInBytes(ComponentType componentType)
{
	switch (componentType)
	{
		case ComponentType::BYTE:
		case ComponentType::UBYTE:
		{
			return 1;
		}
		case ComponentType::SHORT:
		case ComponentType::USHORT:
		{
			return 2;
		}
		case ComponentType::UINT:
		case ComponentType::FLOAT:
		{
			return 4;
		}
		default:
		{
			InvalidCodePath;
			return 0;
		}
	}
}

uint32_t GltfModel::getNComponents(Type type)
{
	switch (type)
	{
		case Type::SCALAR:
		{
			return 1;
		}
		case Type::VEC2:
		{
			return 2;
		}
		case Type::VEC3:
		{
			return 3;
		}
		case Type::VEC4:
		case Type::MAT2:
		{
			return 4;
		}
		case Type::MAT3:
		{
			return 9;
		}
		case Type::MAT4:
		{
			return 16;
		}
		default:
		{
			InvalidCodePath;
			return 0;
		}
	}
}

void GltfModel::parseObjects(const JsonParser::JsonObject& jsonObject, ParseStruct& parseStruct) const
{
	if (jsonObject.name == "bufferViews")
	{
		parseStruct.bufferViews = parseBufferViews(jsonObject);
	}
	else if (jsonObject.name == "accessors")
	{
		parseStruct.accessors = parseAccessors(jsonObject);
	}
	else if (jsonObject.name == "buffers")
	{
		parseStruct.buffer = parseBuffer(jsonObject);
	}
	else if (jsonObject.name == "nodes")
	{
		parseStruct.nodes = parseNodes(jsonObject);
	}
}

void GltfModel::fixPointersInParseStruct(ParseStruct& parseStruct) const
{
	for (auto it = parseStruct.bufferViews.begin(); it != parseStruct.bufferViews.end(); ++it)
	{
		assert(it->buffer == (Buffer*)(intptr_t)0);
		it->buffer = &parseStruct.buffer;
	}

	for (auto it = parseStruct.accessors.begin(); it != parseStruct.accessors.end(); ++it)
	{
		it->bufferView = &parseStruct.bufferViews[(uint32_t)(intptr_t)it->bufferView];
	}
}
