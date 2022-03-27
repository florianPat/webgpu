#pragma once

#include "MeshGltfModel.h"

// NOTE: The deriving is not really cool because all the instance properties and methods still exist... Are there traits in C++? Check!
struct MeshGltfModelPerMaterialFactory : public MeshGltfModel
{
	struct Tuple
	{
		MeshGltfModel model;
		Texture* texture;
	};
private:
	Vector<Tuple> modelsPerMaterial;
	String filename;
public:
	MeshGltfModelPerMaterialFactory(const String& filename, Vector<String>&& animationFilenames = {}, Vector<String>&& meshPartExclusions = {});
	Vector<Tuple>& getMeshModels();
protected:
	Vector<uint32_t> decompileVertexBuffer(ParseStruct& parseStruct);
	void decompileIndexBuffer(ParseStruct& parseStruct, const Vector<uint32_t>& meshSkipIndexes);
	void decompileTextures(ParseStruct& parseStruct, const Vector<uint32_t>& meshSkipIndexes, const String& filename);

	friend class GltfModel;
	void decompileObjects(ParseStruct& parseStruct, const String& filename);
};