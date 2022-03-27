#pragma once

#include "GraphicsVK.h"
#include "Model.h"
#include "Camera.h"
#include "SkeletalAnimation.h"

class GraphicsVK3D : public GraphicsVK
{
	VulkanBuffer uniformBuffer;
	VulkanBuffer instanceBuffer;
	VkSampler sampler = VK_NULL_HANDLE;

	VulkanShader skyboxShader;
	static constexpr VkBool32 enableAlpha = VK_FALSE;

	VkPipeline skyboxPipeline = VK_NULL_HANDLE;
	Model skyboxModel;
	VulkanBuffer noSkinUniformBuffer;

	VulkanShader terrainShader;
	VkPipeline terrainPipeline = VK_NULL_HANDLE;
public:
	static constexpr uint32_t id = 1;
	Camera camera;
public:
	GraphicsVK3D(GraphicsVKIniter& gfxInit);
	~GraphicsVK3D();

	void bindOtherFramebuffer(FramebufferDrawStruct&& stateIn);
	void unbindOtherFramebuffer();
	void render() override;
	void draw(Model& model, const Mat4x4& mvPersProj, const VulkanTexture* texture);
	void draw(Model& model, const Mat4x4& mvPersProj, const VulkanTexture* texture, VulkanBuffer& skinUniformBuffer);
	void draw(Model& model, const Vector3f& pos, const Vector3f& rot, const Vector3f& scl, const VulkanTexture* texture);
	void draw(Model& model, const Vector3f& pos, const Vector3f& rot, const Vector3f& scl, const VulkanTexture* texture,
		VulkanBuffer& skinUniformBuffer);
	void drawSkybox(const VulkanTexture* cubeMapTexture);
	void drawInstanced(Model& model, const Vector<struct InstancedInfo>& instancedInfos, const VulkanTexture* texture);
	void drawInstanced(Model& model, const Vector<struct InstancedInfo>& instancedInfos, const VulkanTexture* texture, VulkanBuffer& skinUniformBuffer);
	void drawTerrain(Model& model, const Vector3f& pos, const VulkanTexture* texture);
private:
	void setupVertexLayoutForModelShader(VulkanShader& shader);
};