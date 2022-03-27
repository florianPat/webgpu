#include "Terrain.h"
#include "Globals.h"
#include "Window.h"

Terrain::Terrain(const String& heightmapFile, uint32_t vertexCount, float size, uint32_t maxHeight, const Vector3f& pos) : Model(), heights(vertexCount* vertexCount),
	vertexCount(vertexCount), size(size), pos(pos)
{
	// TODO: Load height from heightmapfile
	srand(122);

	nVertices = vertexCount * vertexCount;
	new (&vertexBuffer) VulkanBuffer(sizeof(Vertex) * nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		Globals::window->getGfx());
	nIndices = 6 * (vertexCount - 1) * (vertexCount - 1);
	new (&indexBuffer) VulkanBuffer(sizeof(Vertex) * nIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Globals::window->getGfx());

	Vertex* vertices = (Vertex*)vertexBuffer.map(0, VK_WHOLE_SIZE);
	for (uint32_t x = 0; x < vertexCount; ++x)
	{
		for (uint32_t z = 0; z < vertexCount; ++z)
		{
			vertices[z + x * vertexCount].pos.x = (float)x / (vertexCount - 1.0f) * size;
			float height = (float)(rand() % maxHeight);
			heights[x * vertexCount + z] = height;
			vertices[z + x * vertexCount].pos.y = height;
			vertices[z + x * vertexCount].pos.z = (float)z / (vertexCount - 1.0f) * size;
			vertices[z + x * vertexCount].normal = Vector3f();
			vertices[z + x * vertexCount].texCoord = Vector2f(x / (vertexCount - 1.0f), z / (vertexCount - 1.0f));
			vertices[z + x * vertexCount].weights[0] = 1.0f;
		}
	}
	vertexBuffer.unmap(0, VK_WHOLE_SIZE);

	uint16_t* indices = (uint16_t*)indexBuffer.map(0, VK_WHOLE_SIZE);
	uint32_t pointer = 0;
	for (uint32_t x = 0; x < vertexCount - 1; ++x)
	{
		for (uint32_t z = 0; z < vertexCount - 1; ++z)
		{
			uint16_t topLeft = (uint16_t)(x * vertexCount + z);
			uint16_t topRight = topLeft + 1;
			uint16_t bottomLeft = (uint16_t)((x + 1) * vertexCount + z);
			uint16_t bottomRight = bottomLeft + 1;
			indices[pointer++] = topLeft;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = topRight;
			indices[pointer++] = topRight;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = bottomRight;
		}
	}
	indexBuffer.unmap(0, VK_WHOLE_SIZE);
}

float Terrain::getHeight(float worldX, float worldZ)
{
	float terrainX = worldX - pos.x;
	float terrainZ = worldZ - pos.z;
	float gridSquareSize = size / (vertexCount - 1.0f);
	int32_t gridX = (int32_t) floorf(terrainX / gridSquareSize);
	int32_t gridZ = (int32_t) floorf(terrainZ / gridSquareSize);

	if (gridX < 0 || gridX >= (int32_t)vertexCount || gridZ < 0 || gridZ >= (int32_t)vertexCount)
	{
		return 0.0f;
	}

	float xCoord = (float)((int32_t)terrainX % (int32_t)gridSquareSize) / gridSquareSize;
	float zCoord = (float)((int32_t)terrainZ % (int32_t)gridSquareSize) / gridSquareSize;
	float heightTopLeft = heights[gridX * vertexCount + gridZ];
	float heightTopRight = heights[gridX * vertexCount + gridZ + 1];
	float heightBottomLeft = heights[(gridX + 1) * vertexCount + gridZ];
	float heightBottomRight = heights[(gridX + 1) * vertexCount + gridZ + 1];
	float topLerp = utils::lerp(heightTopLeft, heightTopRight, xCoord);
	float bottomLerp = utils::lerp(heightBottomLeft, heightBottomRight, zCoord);
	return utils::lerp(topLerp, bottomLerp, xCoord);
}
