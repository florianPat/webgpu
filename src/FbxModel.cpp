#if 0
#include "FbxModel.h"
#include <fbxsdk/fileio/fbxiosettings.h>
#include "Globals.h"
#include "Window.h"

#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libfbxsdk.lib")
#pragma comment(lib, "libxml2-md.lib")

FbxManager* FbxModel::fbxManager = nullptr;

void FbxModel::initFbxSdk()
{
	fbxManager = FbxManager::Create();

	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ios);
    (*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
    (*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
    (*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, false);
    (*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, false);
    (*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, false);
    (*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
    (*(fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    (*(fbxManager->GetIOSettings())).SetBoolProp(EXP_FBX_EMBEDDED, true);
}

void FbxModel::deinitFbxSdk()
{
    fbxManager->Destroy();
}

FbxModel::FbxModel(const String& filename)
{
    parseVertices(filename);
}

void FbxModel::parseVertices(const String& filename)
{
    FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "Manager");
    assert(fbxImporter != nullptr);
    bool success = fbxImporter->Initialize(filename.c_str(), -1, fbxManager->GetIOSettings());
    assert(success);

    FbxScene* scene = FbxScene::Create(fbxManager, "FbxScene");
    assert(scene);
    success = fbxImporter->Import(scene);
    assert(success);

    fbxImporter->Destroy();
    fbxImporter = nullptr;

    FbxNode* rootNode = scene->GetRootNode();
    traverseTree(rootNode);

    new (&vertexBuffer) VulkanBuffer(sizeof(Vertex) * nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        Globals::window->getGfx());
    vertices = (Vertex*)vertexBuffer.map(0, VK_WHOLE_SIZE);
    new (&indexBuffer) VulkanBuffer(sizeof(uint16_t) * nIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        Globals::window->getGfx());
    indices = (uint16_t*)indexBuffer.map(0, VK_WHOLE_SIZE);

    traverseTree(rootNode, nullptr);

    uint32_t nAnimStacks = scene->GetSrcObjectCount<FbxAnimStack>();
    for (uint32_t i = 0; i < nAnimStacks; ++i)
    {
        vertexIndexOffset = 0;

        FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
        
        traverseTree(rootNode, animStack);

        animations.push_back(SkeletalAnimation(std::move(joints)));
        joints = Vector<SkeletalAnimation::Joint>();
    }

    vertexBuffer.unmap(0, VK_WHOLE_SIZE);
    indexBuffer.unmap(0, VK_WHOLE_SIZE);

    vertices = nullptr;
    indices = nullptr;

    scene->Destroy();
}

void FbxModel::traverseTree(FbxNode* node, FbxAnimStack* animStack)
{
    uint32_t childCount = node->GetChildCount();
    for (uint32_t i = 0; i < childCount; ++i)
    {
        FbxNode* childNode = node->GetChild(i);

        if (childNode->GetNodeAttribute() != nullptr)
        {
            switch (childNode->GetNodeAttribute()->GetAttributeType())
            {
                case FbxNodeAttribute::EType::eMesh:
                {
                    if (animStack == nullptr)
                    {
                        processMesh(childNode->GetMesh());
                    }
                    else
                    {
                        processAnimation(childNode, animStack);
                    }
                    
                    break;
                }
            }
        }

        traverseTree(childNode, animStack);
    }
}

void FbxModel::processMesh(FbxMesh* mesh)
{
    assert(mesh);

    uint32_t nControlPoints = mesh->GetControlPointsCount();

    if (vertices == nullptr)
    {
        nVertices += nControlPoints;
        int32_t polygonCount = mesh->GetPolygonCount();
        for (int32_t i = 0; i < polygonCount; ++i)
        {
            uint32_t nVerticesInPolygon = mesh->GetPolygonSize(i);
            if (nVerticesInPolygon == 4)
            {
                nIndices += 6;
            }
            else if (nVerticesInPolygon == 3)
            {
                nIndices += nVerticesInPolygon;
            }
            else
            {
                InvalidCodePath;
            }
        }
        return;
    }

    FbxVector4* fbxVertices = mesh->GetControlPoints();
    assert(fbxVertices);

    FbxStringList uvSetNames;
    mesh->GetUVSetNames(uvSetNames);

    int32_t polygonCount = mesh->GetPolygonCount();
    for (int32_t polygonI = 0; polygonI < polygonCount; ++polygonI)
    {
        int32_t nVerticesInPolygon = mesh->GetPolygonSize(polygonI);
        assert(nVerticesInPolygon == 3 || nVerticesInPolygon == 4);
        for (int32_t vertexI = 0; vertexI < nVerticesInPolygon; ++vertexI)
        {
            int32_t vertexIndex = mesh->GetPolygonVertex(polygonI, vertexI);
            assert(vertexIndex != -1 && vertexIndex < (int32_t)nControlPoints);

            Vector3f pos;
            pos.x = (float)-fbxVertices[vertexIndex].mData[0];
            pos.y = (float)fbxVertices[vertexIndex].mData[1];
            pos.z = (float)fbxVertices[vertexIndex].mData[2];

            Vector2f texCoord;
            FbxVector2 fbxTexCoord;
            bool unmapped = false;
            bool result = mesh->GetPolygonVertexUV(polygonI, vertexI, uvSetNames.GetItemAt(0)->mString, fbxTexCoord, unmapped);
            assert(result);
            if (!unmapped)
            {
                texCoord.x = (float)fbxTexCoord[0];
                texCoord.y = (float)(1.0f - fbxTexCoord[1]);
            }
            else
            {
                InvalidCodePath;
            }
            
            FbxVector4 fbxNormal;
            Vector3f normal;
            result = mesh->GetPolygonVertexNormal(polygonI, vertexI, fbxNormal);
            assert(result);
            normal.x = (float)fbxNormal[0];
            normal.y = (float)fbxNormal[1];
            normal.z = (float)fbxNormal[2];

            vertexIndex += vertexIndexOffset;

            vertices[vertexIndex].pos = pos;
            vertices[vertexIndex].texCoord = texCoord;
            vertices[vertexIndex].normal = normal;

            if (nVerticesInPolygon == 4 && vertexI == 3)
            {
                indices[polyVertexI++] = (uint16_t)(mesh->GetPolygonVertex(polygonI, 2) + vertexIndexOffset);
                indices[polyVertexI++] = (uint16_t)vertexIndex;
                indices[polyVertexI++] = (uint16_t)(mesh->GetPolygonVertex(polygonI, 0) + vertexIndexOffset);
            }
            else
            {
                assert(nVerticesInPolygon % 3 == 0 || (nVerticesInPolygon == 4 && vertexI != 3));
                indices[polyVertexI++] = (uint16_t)vertexIndex;
            }
        }
    }

    vertexIndexOffset += nControlPoints;
}

void FbxModel::processAnimation(FbxNode* node, FbxAnimStack* animStack)
{
    assert(node);
    FbxMesh* mesh = node->GetMesh();

    uint32_t defomerCount = mesh->GetDeformerCount();
    for (uint32_t i = 0; i < defomerCount; ++i)
    {
        auto* skin = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
        if (skin)
        {
            uint32_t nClusters = skin->GetClusterCount();
            for (uint32_t j = 0; j < nClusters; ++j)
            {
                FbxCluster* cluster = skin->GetCluster(j);

                FbxAMatrix linkMatrix;
                cluster->GetTransformLinkMatrix(linkMatrix);
                FbxAMatrix localMatrix = cluster->GetLink()->EvaluateLocalTransform();
                uint32_t nIndices = cluster->GetControlPointIndicesCount();
                for (uint32_t k = 0; k < nIndices; ++k)
                {
                    float weight = (float)cluster->GetControlPointWeights()[k];
                    uint16_t index = (uint16_t)cluster->GetControlPointIndices()[k];
                    Vertex& vertex = vertices[index + vertexIndexOffset];
                    for (uint32_t k = 0; k < 4; ++k)
                    {
                        if (weight > vertex.weights[k])
                        {
                            for (uint32_t l = k + 1; l < 4; ++l)
                            {
                                vertex.weights[l] = vertex.weights[l - 1];
                                vertex.jointIndices[l] = vertex.jointIndices[l - 1];
                            }
                            vertex.weights[k] = weight;
                            vertex.jointIndices[k] = joints.size();
                            break;
                        }
                    }
                }

                FbxNode* link = cluster->GetLink();
                assert(link->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton);
                FbxTimeSpan animationTimeSpan = animStack->GetLocalTimeSpan();
                uint32_t start = (uint32_t)animationTimeSpan.GetStart().GetMilliSeconds();
                uint32_t end = (uint32_t)animationTimeSpan.GetStop().GetMilliSeconds();

                Vector<SkeletalAnimation::KeyFrame> keyFrames;
                for (uint32_t k = start; k < end; k += KEY_FRAME_INTERVAL_MICROSECONDS)
                {
                    FbxTime currentTime;
                    currentTime.SetMilliSeconds(k);

                    FbxAMatrix globalTransform = link->EvaluateGlobalTransform(currentTime);
                    
                    FbxVector4 fbxPos = globalTransform.GetT();
                    Vector3f pos;
                    pos.x = (float)fbxPos.mData[0];
                    pos.y = (float)fbxPos.mData[1];
                    pos.z = (float)fbxPos.mData[2];

                    FbxQuaternion fbxRot = globalTransform.GetQ();
                    Quaternion rot(Vector3f((float)fbxRot.mData[0], (float)fbxRot.mData[1], (float)fbxRot.mData[2]), 0.0f);
                    rot.w = (float)fbxRot.mData[3];

                    keyFrames.push_back(SkeletalAnimation::KeyFrame{ Time::milliseconds(k - start).asMicroseconds(), pos, rot });
                }

                SkeletalAnimation::Joint joint;
                joint.keyFrames = std::move(keyFrames);
                joints.push_back(std::move(joint));
            }
        }
    }

    vertexIndexOffset += mesh->GetControlPointsCount();
}
#endif