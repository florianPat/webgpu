#pragma once

#include "Ifstream.h"
#include "Vector.h"
#include "Vector3.h"
#include "Vector2.h"
#include "VulkanBuffer.h"
#include "Model.h"

struct ObjModel : public Model
{
	ObjModel(const String& filename);
	uint64_t getSize() const { return nVertices * sizeof(Vertex) + nIndices * sizeof(uint16_t) + sizeof(ObjModel); }
private:
	float findNextNumber(String& line);
	void parseVertices(const String& filename);
};