#pragma once

#include "Model.h"

class Terrain : public Model
{
	HeapArray<float> heights;
	uint32_t vertexCount;
	float size;
public:
	Vector3f pos;
public:
	Terrain(const String& heightmapFile, uint32_t vertexCount, float size, uint32_t maxHeight, const Vector3f& pos);
	float getHeight(float worldX, float worldZ);
};