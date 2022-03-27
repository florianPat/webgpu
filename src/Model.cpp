#include "Model.h"
#include "Globals.h"
#include "Window.h"

void Model::reloadFromFile(const String& filename)
{
	vertexBuffer.~VulkanBuffer();
}

Model Model::cube()
{
	Model result;

	result.nVertices = 8;
	new (&result.vertexBuffer) VulkanBuffer(sizeof(Vertex) * result.nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		Globals::window->getGfx());
	result.nIndices = 36;
	new (&result.indexBuffer) VulkanBuffer(sizeof(Vertex) * result.nIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Globals::window->getGfx());

	Vertex* vertices = (Vertex*) result.vertexBuffer.map(0, VK_WHOLE_SIZE);
	vertices[0].pos = { -1.0f, -1.0f, -1.0f };
	vertices[0].texCoord = Vector2f(0.0f, 1.0f);
	vertices[0].normal = Vector3f();
	vertices[0].weights[0] = 1.0f;
	vertices[1].pos = { 1.0f, -1.0f, -1.0f };
	vertices[1].texCoord = Vector2f(1.0f, 1.0f);
	vertices[1].normal = Vector3f();
	vertices[1].weights[0] = 1.0f;
	vertices[2].pos = { 1.0f, 1.0f, -1.0f };
	vertices[2].texCoord = Vector2f(1.0f, 0.0f);
	vertices[2].normal = Vector3f();
	vertices[2].weights[0] = 1.0f;
	vertices[3].pos = { -1.0f, 1.0f, -1.0f };
	vertices[3].texCoord = Vector2f(0.0f, 0.0f);
	vertices[3].normal = Vector3f();
	vertices[3].weights[0] = 1.0f;
	vertices[4].pos = { -1.0f, -1.0f, 1.0f };
	vertices[4].texCoord = Vector2f(0.0f, 1.0f);
	vertices[4].normal = Vector3f();
	vertices[4].weights[0] = 1.0f;
	vertices[5].pos = { 1.0f, -1.0f, 1.0f };
	vertices[5].texCoord = Vector2f(0.0f, 0.0f);
	vertices[5].normal = Vector3f();
	vertices[5].weights[0] = 1.0f;
	vertices[6].pos = { 1.0f, 1.0f, 1.0f };
	vertices[6].texCoord = Vector2f(1.0f, 0.0f);
	vertices[6].normal = Vector3f();
	vertices[6].weights[0] = 1.0f;
	vertices[7].pos = { -1.0f, 1.0f, 1.0f };
	vertices[7].texCoord = Vector2f(0.0f, 0.0f);
	vertices[7].normal = Vector3f();
	vertices[7].weights[0] = 1.0f;
	result.vertexBuffer.unmap(0, VK_WHOLE_SIZE);

	uint16_t* indices = (uint16_t*)result.indexBuffer.map(0, VK_WHOLE_SIZE);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 3;
	indices[5] = 0;

	indices[6] = 5;
	indices[7] = 4;
	indices[8] = 7;
	indices[9] = 7;
	indices[10] = 6;
	indices[11] = 5;

	indices[12] = 1;
	indices[13] = 5;
	indices[14] = 6;
	indices[15] = 6;
	indices[16] = 2;
	indices[17] = 1;

	indices[18] = 4;
	indices[19] = 0;
	indices[20] = 3;
	indices[21] = 3;
	indices[22] = 7;
	indices[23] = 4;

	indices[24] = 3;
	indices[25] = 2;
	indices[26] = 6;
	indices[27] = 6;
	indices[28] = 7;
	indices[29] = 3;

	indices[30] = 1;
	indices[31] = 0;
	indices[32] = 4;
	indices[33] = 4;
	indices[34] = 5;
	indices[35] = 1;
	result.indexBuffer.unmap(0, VK_WHOLE_SIZE);

	return result;
}

Model Model::skyboxCube()
{
	Model result;

	result.nVertices = 8;
	new (&result.vertexBuffer) VulkanBuffer(sizeof(Vector3f) * result.nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		Globals::window->getGfx());
	result.nIndices = 36;
	new (&result.indexBuffer) VulkanBuffer(sizeof(Vector3f) * result.nIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Globals::window->getGfx());

	Vector3f* vertices = (Vector3f*)result.vertexBuffer.map(0, VK_WHOLE_SIZE);
	vertices[0] = { -1.0f, -1.0f, -1.0f };
	vertices[1] = { 1.0f, -1.0f, -1.0f };
	vertices[2] = { 1.0f, 1.0f, -1.0f };
	vertices[3] = { -1.0f, 1.0f, -1.0f };
	vertices[4] = { -1.0f, -1.0f, 1.0f };
	vertices[5] = { 1.0f, -1.0f, 1.0f };
	vertices[6] = { 1.0f, 1.0f, 1.0f };
	vertices[7] = { -1.0f, 1.0f, 1.0f };
	result.vertexBuffer.unmap(0, VK_WHOLE_SIZE);

	uint16_t* indices = (uint16_t*)result.indexBuffer.map(0, VK_WHOLE_SIZE);
	indices[0] = 2;
	indices[1] = 1;
	indices[2] = 0;
	indices[3] = 0;
	indices[4] = 3;
	indices[5] = 2;

	indices[6] = 7;
	indices[7] = 4;
	indices[8] = 5;
	indices[9] = 5;
	indices[10] = 6;
	indices[11] = 7;

	indices[12] = 6;
	indices[13] = 5;
	indices[14] = 1;
	indices[15] = 1;
	indices[16] = 2;
	indices[17] = 6;

	indices[18] = 3;
	indices[19] = 0;
	indices[20] = 4;
	indices[21] = 4;
	indices[22] = 7;
	indices[23] = 3;

	indices[24] = 6;
	indices[25] = 2;
	indices[26] = 3;
	indices[27] = 3;
	indices[28] = 7;
	indices[29] = 6;

	indices[30] = 4;
	indices[31] = 0;
	indices[32] = 1;
	indices[33] = 1;
	indices[34] = 5;
	indices[35] = 4;
	result.indexBuffer.unmap(0, VK_WHOLE_SIZE);

	return result;
}

Model Model::plane()
{
	Model result;

	result.nVertices = 4;
	new (&result.vertexBuffer) VulkanBuffer(sizeof(Vertex) * result.nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		Globals::window->getGfx());
	result.nIndices = 6;
	new (&result.indexBuffer) VulkanBuffer(sizeof(Vertex) * result.nIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Globals::window->getGfx());

	Vertex* vertices = (Vertex*)result.vertexBuffer.map(0, VK_WHOLE_SIZE);
	vertices[0].pos = { -1.0f, 0.0f, -1.0f };
	vertices[0].texCoord = Vector2f(0.0f, 0.0f);
	vertices[0].normal = Vector3f();
	vertices[0].weights[0] = 1.0f;
	vertices[1].pos = { 1.0f, 0.0f, -1.0f };
	vertices[1].texCoord = Vector2f(1.0f, 0.0f);
	vertices[1].normal = Vector3f();
	vertices[1].weights[0] = 1.0f;
	vertices[2].pos = { 1.0f, 0.0f, 1.0f };
	vertices[2].texCoord = Vector2f(1.0f, 1.0f);
	vertices[2].normal = Vector3f();
	vertices[2].weights[0] = 1.0f;
	vertices[3].pos = { -1.0f, 0.0f, 1.0f };
	vertices[3].texCoord = Vector2f(0.0f, 1.0f);
	vertices[3].normal = Vector3f();
	vertices[3].weights[0] = 1.0f;
	result.vertexBuffer.unmap(0, VK_WHOLE_SIZE);

	uint16_t* indices = (uint16_t*)result.indexBuffer.map(0, VK_WHOLE_SIZE);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 3;
	indices[5] = 0;
	result.indexBuffer.unmap(0, VK_WHOLE_SIZE);

	return result;
}
