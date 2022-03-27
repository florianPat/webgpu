#pragma once

#include "Model.h"
#include "SkeletalAnimation.h"
#include "JsonParser.h"
#include <unordered_map>

class GltfModel : public Model
{
protected:
	enum class ComponentType
	{
		BYTE = 5120,
		UBYTE = 5121,
		SHORT = 5122,
		USHORT = 5123,
		UINT = 5125,
		FLOAT = 5126,
	};
	enum class Type
	{
		SCALAR = utils::getIntFromChars('S', 'R'),
		VEC2 = utils::getIntFromChars('V', '2'),
		VEC3 = utils::getIntFromChars('V', '3'),
		VEC4 = utils::getIntFromChars('V', '4'),
		MAT2 = utils::getIntFromChars('M', '2'),
		MAT3 = utils::getIntFromChars('M', '3'),
		MAT4 = utils::getIntFromChars('M', '4'),
	};
	struct Buffer
	{
		uint8_t* data = nullptr;
		uint32_t byteLength = 0;
	};
	struct BufferView
	{
		Buffer* buffer = 0;
		uint32_t byteLength = 0;
		uint32_t byteOffset = 0;
		uint32_t byteStride = 0;
		uint32_t target = 0;
	};
	struct Accessor
	{
		BufferView* bufferView = nullptr;
		uint32_t byteOffset = 0;
		ComponentType componentType = ComponentType::BYTE;
		uint32_t count = 0;
		Type type = Type::SCALAR;
	};
	struct Node
	{
		String name = "";
		Vector<int32_t> children;
		Vector3f pos;
		Quaternion rot;
		Vector3f scl;
		Mat4x4 transform;
	};
	struct ParseStruct
	{
		Vector<Accessor> accessors;
		Vector<BufferView> bufferViews;
		Buffer buffer;
		Vector<Node> nodes;
	};
	struct ChunkHeader
	{
		uint32_t length = 0;
		String type;
	};
	struct NodeJointMapValue
	{
		uint32_t jointIndex;
		String nodeName;
	};
	typedef std::unordered_map<uint32_t, NodeJointMapValue> NodeJointMap;
public:
	// NOTE: The template stuff and the child friend thing and this is neccessary to get "virtual functions" in constructors. But without the cost of virtual functions.
	// TODO: Can we do this better?
	//GltfModel(const String& filename) { parseVertices<GltfModel>(); }
	uint64_t getSize() const { return nVertices * sizeof(Vertex) + nIndices * sizeof(uint16_t) + sizeof(GltfModel); }
protected:
	template<typename T>
	void parseVertices(const String& filename);
private:
	ShortString parseIntToAscii(uint8_t* readBytes, uint32_t offset) const;
	void parseHeader(Ifstream& file) const;
	ChunkHeader parseChunkHeader(Ifstream& file) const;
	template<typename T>
	typename T::ParseStruct parseFile(Ifstream& file) const;
	template<typename T>
	typename T::ParseStruct parseJson(Ifstream& file) const;
	void parseBin(Ifstream& file, ParseStruct& parseStruct) const;
	Vector<BufferView> parseBufferViews(const JsonParser::JsonObject& jsonObject) const;
	Vector<Accessor> parseAccessors(const JsonParser::JsonObject& jsonObject) const;
	Buffer parseBuffer(const JsonParser::JsonObject& jsonObject) const;
	Vector<Node> parseNodes(const JsonParser::JsonObject& jsonObject) const;
protected:
	static uint32_t getSizeInBytes(ComponentType componentType);
	static uint32_t getNComponents(Type type);
protected:
	void parseObjects(const JsonParser::JsonObject& jsonObject, ParseStruct& parseStruct) const;
	void fixPointersInParseStruct(ParseStruct& parseStruct) const;
	void decompileObjects(ParseStruct& parseStruct, const String& filename) {};
};

template<typename T>
void GltfModel::parseVertices(const String& filename)
{
	Ifstream file(filename);
	assert(file);
	parseHeader(file);
	T::ParseStruct parseStruct = parseFile<T>(file);
	((T*)this)->fixPointersInParseStruct(parseStruct);
	((T*)this)->decompileObjects(parseStruct, filename);
}

template<typename T>
typename T::ParseStruct GltfModel::parseFile(Ifstream& file) const
{
	T::ParseStruct result = parseJson<T>(file);
	parseBin(file, (ParseStruct&)result);
	return result;
}

template<typename T>
typename T::ParseStruct GltfModel::parseJson(Ifstream& file) const
{
	T::ParseStruct result;

	ChunkHeader chunkHeader = parseChunkHeader(file);
	assert(chunkHeader.type == "JSON");

	LongString json(chunkHeader.length, 0);
	file.read(json.data(), chunkHeader.length);

	JsonParser jsonParser(json);

	for (auto it = jsonParser.jsonObjects.begin(); it != jsonParser.jsonObjects.end(); ++it)
	{
		((T*)this)->parseObjects(*it, result);
	}

	return result;
}
