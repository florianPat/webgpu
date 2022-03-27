#include "MeshGltfModelPerMaterialFactory.h"
#include "Globals.h"
#include "Window.h"

MeshGltfModelPerMaterialFactory::MeshGltfModelPerMaterialFactory(const String& filename, Vector<String>&& animationFilenames, Vector<String>&& meshPartExclusions) : MeshGltfModel(), modelsPerMaterial(2), filename(filename)
{
	for (auto it = animationFilenames.begin(); it != animationFilenames.end(); ++it)
	{
		animations.emplace(*it, SkeletalAnimation(uniformBuffer, Vector<SkeletalAnimation::Channel>(), inverseBindMatrices, jointChildrenIndicies));
	}
	parseVertices<MeshGltfModelPerMaterialFactory>(filename);
}

Vector<MeshGltfModelPerMaterialFactory::Tuple>& MeshGltfModelPerMaterialFactory::getMeshModels()
{
	return modelsPerMaterial;
}

Vector<uint32_t> MeshGltfModelPerMaterialFactory::decompileVertexBuffer(ParseStruct& parseStruct)
{
	Vector<uint32_t> skipIndexes;
	uint32_t currentSkipIndex = 0;

	uint32_t i = 0;
	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		const Mesh& mesh = *it;

		if (!meshPartExclusions.empty())
		{
			for (auto it = meshPartExclusions.begin(); it != meshPartExclusions.end(); ++it)
			{
				if (mesh.name == *it)
				{
					skipIndexes.push_back(i);
				}
			}

			if (skipIndexes.size() && skipIndexes.back() == i)
			{
				continue;
			}
		}
	}

	i = 0;
	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		if (skipIndexes.size() > currentSkipIndex && skipIndexes[currentSkipIndex] == i)
		{
			currentSkipIndex++;
			continue;
		}

		const Mesh& mesh = *it;

		if (modelsPerMaterial.size() <= mesh.material->texture)
		{
			// NOTE: This does not work because of unordered_map trying to free memory not allocated before because of move assignment in insert...
			// modelsPerMaterial.resize(mesh.material->texture + 1);
			for (uint32_t i = modelsPerMaterial.size(); i <= mesh.material->texture; ++i)
			{
				modelsPerMaterial.push_back(std::move(Tuple()));
			}
		}
		MeshGltfModel& textureModel = modelsPerMaterial[mesh.material->texture].model;
		const uint32_t meshVertices = mesh.position->count;
		textureModel.nVertices = meshVertices;
		new (&textureModel.vertexBuffer) VulkanBuffer(sizeof(Vertex) * meshVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, Globals::window->getGfx());
		Vertex* vertices = (Vertex*)textureModel.vertexBuffer.map(0, VK_WHOLE_SIZE);

		assert(mesh.position->componentType == ComponentType::FLOAT);
		assert(mesh.position->type == Type::VEC3);
		uint32_t positionStride = mesh.position->bufferView->byteStride == 0 ? getSizeInBytes(mesh.position->componentType) * getNComponents(mesh.position->type)
			: mesh.position->bufferView->byteStride;
		uint8_t* positionData = &mesh.position->bufferView->buffer->data[mesh.position->byteOffset + mesh.position->bufferView->byteOffset];

		assert(mesh.texCoord->type == Type::VEC2);
		uint32_t texCoordStride = mesh.texCoord->bufferView->byteStride == 0 ? getSizeInBytes(mesh.texCoord->componentType) * getNComponents(mesh.texCoord->type)
			: mesh.texCoord->bufferView->byteStride;
		uint8_t* texCoordData = &mesh.texCoord->bufferView->buffer->data[mesh.texCoord->byteOffset + mesh.texCoord->bufferView->byteOffset];

		uint32_t jointStride = 0;
		uint8_t* jointData = nullptr;
		uint32_t weightStride = 0;
		uint8_t* weightData = nullptr;
		if (mesh.joints != nullptr && mesh.weights != nullptr)
		{
			assert(mesh.joints->componentType == ComponentType::USHORT);
			assert(mesh.joints->type == Type::VEC4);
			jointStride = mesh.joints->bufferView->byteStride == 0 ? getSizeInBytes(mesh.joints->componentType) * getNComponents(mesh.joints->type)
				: mesh.joints->bufferView->byteStride;
			jointData = &mesh.joints->bufferView->buffer->data[mesh.joints->byteOffset + mesh.joints->bufferView->byteOffset];

			assert(mesh.weights->componentType == ComponentType::FLOAT);
			assert(mesh.weights->type == Type::VEC4);
			weightStride = mesh.weights->bufferView->byteStride == 0 ? getSizeInBytes(mesh.weights->componentType) * getNComponents(mesh.weights->type)
				: mesh.weights->bufferView->byteStride;
			weightData = &mesh.weights->bufferView->buffer->data[mesh.weights->byteOffset + mesh.weights->bufferView->byteOffset];
		}

		for (uint32_t i = 0; i < meshVertices; ++i)
		{
			float* positions = (float*)positionData;
			float x = *(positions + 0);
			float y = *(positions + 1);
			float z = *(positions + 2);
			vertices->pos = { x, y, z };
			positionData += positionStride;

			float u = 0.0f, v = 0.0f;
			switch (mesh.texCoord->componentType)
			{
			case ComponentType::FLOAT:
			{
				float* texCoords = (float*)texCoordData;
				u = *(texCoords + 0);
				v = *(texCoords + 1);
				break;
			}
			case ComponentType::USHORT:
			{
				uint16_t* texCoords = (uint16_t*)texCoordData;
				u = (*(texCoords + 0)) / 65535.0f;
				v = (*(texCoords + 1)) / 65535.0f;
				break;
			}
			case ComponentType::UBYTE:
			{
				uint8_t* texCoords = (uint8_t*)texCoordData;
				u = (*(texCoords + 0)) / 255.0f;
				v = (*(texCoords + 1)) / 255.0f;
				break;
			}
			default:
			{
				InvalidCodePath;
				break;
			}
			}
			vertices->texCoord = { u, v };
			texCoordData += texCoordStride;

			if (jointData != nullptr)
			{
				assert(weightData != nullptr);
				// NOTE: This works because there is only one skin in this mesh. If there a more than one, this breaks!! RISKIY!!
				uint16_t* jointIndicies = (uint16_t*)jointData;
				float* weights = (float*)weightData;
				for (uint32_t j = 0; j < 4; ++j)
				{
					vertices->jointIndices[j] = (uint32_t)jointIndicies[j];
					vertices->weights[j] = weights[j];
				}

				jointData += jointStride;
				weightData += weightStride;
			}
			else
			{
				for (uint32_t j = 0; j < 4; ++j)
				{
					vertices->jointIndices[j] = 0;
					vertices->weights[j] = 0;
				}
				vertices->weights[0] = 1.0f;
			}

			++vertices;
		}

		textureModel.vertexBuffer.unmap(0, VK_WHOLE_SIZE);
	}

	return skipIndexes;
}

void MeshGltfModelPerMaterialFactory::decompileIndexBuffer(ParseStruct& parseStruct, const Vector<uint32_t>& meshSkipIndexes)
{
	uint32_t currentSkipIndex = 0;
	uint32_t i = 0;
	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		if (meshSkipIndexes.size() > currentSkipIndex && meshSkipIndexes[currentSkipIndex] == i)
		{
			++currentSkipIndex;
			continue;
		}

		const Mesh& mesh = *it;

		MeshGltfModel& textureModel = modelsPerMaterial[mesh.material->texture].model;
		const uint32_t meshIndicies = mesh.indicies->count;
		textureModel.nIndices = meshIndicies;
		new (&textureModel.indexBuffer) VulkanBuffer(sizeof(uint16_t) * meshIndicies, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Globals::window->getGfx());
		uint16_t* indicies = (uint16_t*)textureModel.indexBuffer.map(0, VK_WHOLE_SIZE);

		assert(mesh.indicies->componentType == ComponentType::USHORT);
		assert(mesh.indicies->type == Type::SCALAR);
		uint32_t indicesStride = mesh.indicies->bufferView->byteStride == 0 ? getSizeInBytes(mesh.indicies->componentType) * getNComponents(mesh.indicies->type)
			: mesh.indicies->bufferView->byteStride;
		uint8_t* indiciesData = &mesh.indicies->bufferView->buffer->data[mesh.indicies->byteOffset + mesh.indicies->bufferView->byteOffset];
		assert(nIndices % 3 == 0);
		for (uint32_t i = 0; i < meshIndicies;)
		{
			*(indicies + 0) = *((uint16_t*)indiciesData);
			indiciesData += indicesStride;

			*(indicies + 2) = *((uint16_t*)indiciesData);
			indiciesData += indicesStride;

			*(indicies + 1) = *((uint16_t*)indiciesData);
			indiciesData += indicesStride;

			indicies += 3;
			i += 3;
		}

		textureModel.indexBuffer.unmap(0, VK_WHOLE_SIZE);
	}
}

void MeshGltfModelPerMaterialFactory::decompileTextures(ParseStruct& parseStruct, const Vector<uint32_t>& meshSkipIndexes, const String& filename)
{
	uint32_t currentSkipIndex = 0;
	uint32_t i = 0;
	for (auto it = parseStruct.meshes.begin(); it != parseStruct.meshes.end(); ++it, ++i)
	{
		if (meshSkipIndexes.size() > currentSkipIndex && meshSkipIndexes[currentSkipIndex] == i)
		{
			++currentSkipIndex;
			continue;
		}

		const Mesh& mesh = *it;
		// TODO: Load the texture directly from the glb file and not, like now, from a seperate texture file
		uint32_t lastSlashPos = filename.find_last_of('/');
		assert(lastSlashPos != String::npos);
		LongString textureFilename = filename.substr(0, lastSlashPos + 1);
		textureFilename += mesh.material->name;
		// TODO: Load from texture object of glb
		textureFilename += ".png";
		modelsPerMaterial[mesh.material->texture].texture = Globals::window->getAssetManager()->getOrAddRes<Texture>(textureFilename);
	}
}

void MeshGltfModelPerMaterialFactory::decompileObjects(ParseStruct& parseStruct, const String& filename)
{
	GltfModel::decompileObjects((GltfModel::ParseStruct&)parseStruct, filename);

	Vector<uint32_t> meshSkipIndexes = decompileVertexBuffer(parseStruct);
	decompileIndexBuffer(parseStruct, meshSkipIndexes);
	decompileTextures(parseStruct, meshSkipIndexes, filename);

	// inverseBindMatrices = decompileSkinBuffer(parseStruct);
	// decompileAnimation(parseStruct);
}
