#pragma once

#include "Vector3.h"
#include "VulkanBuffer.h"
#include "String.h"

struct Model
{
	struct Vertex
	{
		Vector3f pos;
		Vector2f texCoord;
		Vector3f normal;
		uint32_t jointIndices[4] = { 0, 0, 0, 0 };
		float weights[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	};

	uint32_t nVertices = 0;
	uint32_t nIndices = 0;
	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;
public:
	void reloadFromFile(const String& filename);
	uint64_t getSize() const { return nVertices * sizeof(Vertex) + nIndices * sizeof(uint16_t) + sizeof(Model); }
	static Model cube();
	static Model skyboxCube();
	static Model plane();
};