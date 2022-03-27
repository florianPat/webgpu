#include "ObjModel.h"
#include "Globals.h"
#include "Window.h"

ObjModel::ObjModel(const String& filename)
{
	parseVertices(filename);
}

float ObjModel::findNextNumber(String& line)
{
	uint32_t spacePos = line.find(' ', 0);
	uint32_t count = 0;
	if (spacePos != String::npos)
		count = spacePos;
	else
		count = line.size();
	assert(count > 0 && count <= line.size());

	float result = (float)atof(line.substr(0, count).c_str());

	if (count != line.size())
		line.erase(0, count + 1);

	return result;
}

void ObjModel::parseVertices(const String& filename)
{
	Ifstream file(filename);
	assert(file);

	LongString line = "";

	Vector<Vector3f> pos;
	Vector<Vector2f> texCoords;
	Vector<Vector3f> vertexNormals;
	Vector<uint16_t> indices;
	Vector<uint32_t> texCoordIndices;
	Vector<uint32_t> vertexNormalIndices;

	while (!file.eof())
	{
		file.getline(line);

		switch (line[0])
		{
		case 'v':
		{
			switch (line[1])
			{
				case ' ':
				{
					float coords[3] = {};
					line.erase(0, 2);
					for (uint32_t i = 0; i < 3; ++i)
					{
						coords[i] = findNextNumber(line);
					}

					pos.emplace_back(coords[0], coords[1], coords[2]);
					break;
				}
				case 't':
				{
					float coords[2] = {};
					line.erase(0, 3);
					for (uint32_t i = 0; i < 2; ++i)
					{
						coords[i] = findNextNumber(line);
					}

					texCoords.emplace_back(coords[0], coords[1]);
					break;
				}
				case 'n':
				{
					float normal[3];
					line.erase(0, 3);
					for (uint32_t i = 0; i < 3; ++i)
					{
						normal[i] = findNextNumber(line);
					}

					vertexNormals.emplace_back(normal[0], normal[1], normal[2]);
					break;
				}
				default:
				{
					InvalidCodePath;
					break;
				}
			}
			break;
		}
		case 'f':
		{
			line.erase(0, 2);

			while (!line.empty())
			{
				if (line.find('/') != String::npos)
				{
					for (uint32_t i = 0; i < 3; ++i)
					{
						if (i == 2 && line[0] == ' ')
						{
							line.erase(0, 1);
							break;
						}

						uint32_t nextSlashPos = 0;
						if (i == 2)
						{
							nextSlashPos = line.find(' ');
							if (nextSlashPos == String::npos)
								nextSlashPos = line.size();
						}
						else
							nextSlashPos = line.find('/');

						if (i == 1 && nextSlashPos == 0)
						{
							line.erase(0, 1);
							break;
						}

						assert(nextSlashPos != 0 && nextSlashPos != String::npos);
						uint16_t index = (uint16_t)atoi(line.substr(0, nextSlashPos).c_str());
						--index;
						if (i == 0)
						{
							indices.emplace_back(index);
						}
						else if (i == 1)
						{
							texCoordIndices.emplace_back(index);
						}
						else if (i == 2)
						{
							vertexNormalIndices.emplace_back(index);
						}

						if (nextSlashPos != line.size())
							line.erase(0, nextSlashPos + 1);
						else
							line.erase(0, line.size());
					}
				}
				else
				{
					for (uint32_t i = 0; i < 3; ++i)
					{
						int16_t index = (int16_t)findNextNumber(line);
						if (index < 0)
							index *= -1;
						--index;
						indices.emplace_back((uint16_t)index);
					}
				}
			}
			break;
		}
		case 's': //edge betwween with same number are smooth or not
		{
			break;
		}
		case 'u': //usemlt
		case 'm': //mtllib
		case '#': // comment
		case 'o': // name of model
		{
			break;
		}
		default:
		{
			InvalidCodePath;
			break;
		}
		}
	}

	assert(indices.size() % 3 == 0);

	nVertices = max(max(pos.size(), texCoords.size()), vertexNormals.size());
	new (&vertexBuffer) VulkanBuffer(sizeof(Vertex)* nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		Globals::window->getGfx());
	Vertex* vertices = (Vertex*)vertexBuffer.map(0, VK_WHOLE_SIZE);
	
	nIndices = indices.size();
	new (&indexBuffer) VulkanBuffer(sizeof(uint16_t)* nIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Globals::window->getGfx());
	uint16_t* indicesMapped = (uint16_t*)indexBuffer.map(0, VK_WHOLE_SIZE);

	for (uint32_t i = 0; i < indices.size(); ++i)
	{
		Vertex vertex;
		vertex.pos = pos[indices[i]];
		vertex.texCoord = texCoords[texCoordIndices[i]];
		vertex.normal = vertexNormals[vertexNormalIndices[i]];
		vertices[i] = vertex;

		indicesMapped[i] = (uint16_t)i;
	}

	vertexBuffer.unmap(0, VK_WHOLE_SIZE);
	indexBuffer.unmap(0, VK_WHOLE_SIZE);
}

