#pragma once

#include "AnimationGltfModel.h"

class MeshGltfModel : public GltfModel
{
protected:
	struct Material
	{
		String name = "";
		uint32_t texture = 0;
		uint32_t texCoordIndex = 0;
	};
	struct Mesh
	{
		String name = "";
		Accessor* position = nullptr;
		Accessor* indicies = nullptr;
		Accessor* texCoord = nullptr;
		Accessor* joints = nullptr;
		Accessor* weights = nullptr;
		Material* material = nullptr;
	};
	struct Skin
	{
		Accessor* inversedBindMatrices = nullptr;
		Vector<Node*> joints;
	};
	struct ParseStruct : public GltfModel::ParseStruct
	{
		Vector<Mesh> meshes;
		Vector<Skin> skins;
		Vector<Material> materials;
	};
public:
	VulkanBuffer uniformBuffer;
	Vector<Mat4x4> inverseBindMatrices;
	Vector<Vector<uint32_t>> jointChildrenIndicies;
	NodeJointMap nodeJointMap;
	std::unordered_map<String, SkeletalAnimation> animations;
	Vector<String> meshPartExclusions;
public:
	MeshGltfModel() = default;
	MeshGltfModel(const String& filename, Vector<String>&& animationFilenames = {}, Vector<String>&& meshPartExclusions = {});
protected:
	Vector<Mesh> parseMeshes(const JsonParser::JsonObject& jsonObject) const;
	Vector<Skin> parseSkins(const JsonParser::JsonObject& jsonObject) const;
	Vector<Material> parseMaterials(const JsonParser::JsonObject& jsonObject) const;
	void computeGlobalJointTransform(Node* node, Vector<Node>& nodes) const;
	Vector<uint32_t> decompileVertexBuffer(ParseStruct& parseStruct);
	void decompileIndexBuffer(ParseStruct& parseStruct, const Vector<uint32_t>& meshSkipIndexes);
	Vector<Mat4x4> decompileSkinBuffer(ParseStruct& parseStruct);
	void createNodeJointMap(Skin& skin, intptr_t nodeStart);
	void decompileAnimation(ParseStruct& parseStruct);

	friend class GltfModel;
	void parseObjects(const JsonParser::JsonObject& jsonObject, ParseStruct& parseStruct) const;
	void fixPointersInParseStruct(ParseStruct& parseStruct) const;
	void decompileObjects(ParseStruct& parseStruct, const String& filename);
};