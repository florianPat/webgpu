#if 0
#pragma once

#include <fbxsdk.h>
#include "String.h"
#include "Model.h"
#include "Quaternion.h"
#include "SkeletalAnimation.h"

class FbxModel : public Model
{
	static FbxManager* fbxManager;
	static constexpr uint32_t KEY_FRAME_INTERVAL_MICROSECONDS = 10;

	//TODO: Do that without these vars?
	Vertex* vertices = nullptr;
	uint16_t* indices = nullptr;
	uint32_t vertexIndexOffset = 0;
	uint32_t polyVertexI = 0;
	Vector<SkeletalAnimation::Joint> joints;
public:
	Vector<SkeletalAnimation> animations;
public:
	static void initFbxSdk();
	static void deinitFbxSdk();
public:
	FbxModel(const String& filename);
	uint64_t getSize() const { return nVertices * sizeof(Vertex) + nIndices * sizeof(uint16_t) + sizeof(FbxModel); }
private:
	void parseVertices(const String& filename);
	void traverseTree(FbxNode* node, FbxAnimStack* animStack = nullptr);
	void processMesh(FbxMesh* mesh);
	void processAnimation(FbxNode* node, FbxAnimStack* animStack);
};
#endif