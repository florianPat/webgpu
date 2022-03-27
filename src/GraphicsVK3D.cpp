#include "GraphicsVK3D.h"
#include "Graphics.h"

GraphicsVK3D::GraphicsVK3D(GraphicsVKIniter& gfxInit) 
	: GraphicsVK(gfxInit),
	uniformBuffer(sizeof(Mat4x4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, gfxInit),
	instanceBuffer(sizeof(InstancedInfo) * 64, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, gfxInit),
	sampler(gfxInit.createSampler()), skyboxShader("shaders/SkyBox", gfxInit.device),
	skyboxModel(Model::skyboxCube()),
	noSkinUniformBuffer(sizeof(Mat4x4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, gfxInit),
	terrainShader("shaders/Terrain", gfxInit.device),
	camera(Mat4x4::persProj(0.1f, 1000.0f, (float)gfxInit.renderWidth / gfxInit.renderHeight, 90.0f))
{
	new (&shader) VulkanShader("shaders/Model", gfxInit.device);

	Mat4x4* mappedData = (Mat4x4*) uniformBuffer.map(0, sizeof(Mat4x4));
	new (mappedData) Mat4x4(camera.getViewPersMat());
	uniformBuffer.unmap(0, sizeof(Mat4x4));

	mappedData = (Mat4x4*)noSkinUniformBuffer.map(0, sizeof(Mat4x4));
	new (mappedData) Mat4x4(Mat4x4::identity());
	noSkinUniformBuffer.unmap(0, sizeof(Mat4x4));

	setupVertexLayoutForModelShader(shader);

	skyboxShader.vertexLayout.addAttribute(3, VulkanVertexLayout::Type::FLOAT);
	skyboxShader.vertexLayout.set(VK_VERTEX_INPUT_RATE_VERTEX);

	setupVertexLayoutForModelShader(terrainShader);

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[3] = {};
	descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings[0].descriptorCount = 1;
	descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBindings[0].binding = 0;
	descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetLayoutBindings[1].descriptorCount = 1;
	descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBindings[1].binding = 1;
	descriptorSetLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings[2].descriptorCount = 1;
	descriptorSetLayoutBindings[2].pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBindings[2].binding = 2;
	new (&descriptorSetLayout) VulkanDescriptorSetLayout(descriptorSetLayoutBindings, arrayCount(descriptorSetLayoutBindings),
		gfxInit.device);
	VkDescriptorBufferInfo descriptorBufferInfo = {};
	descriptorBufferInfo.buffer = uniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = uniformBuffer.getSize();
	descriptorSetLayout.updateUniformBuffer(0, &descriptorBufferInfo);

	pipeline = gfxInit.createWindowPipeline(shader, descriptorSetLayout, enableAlpha, VK_NULL_HANDLE);
	skyboxPipeline = gfxInit.createWindowPipeline(skyboxShader, descriptorSetLayout, false, VK_NULL_HANDLE);
	terrainPipeline = gfxInit.createWindowPipeline(terrainShader, descriptorSetLayout, false, VK_NULL_HANDLE);
}

GraphicsVK3D::~GraphicsVK3D()
{
	if (terrainPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(gfxInit.device, terrainPipeline, nullptr);
		terrainPipeline = VK_NULL_HANDLE;
	}
	if (skyboxPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(gfxInit.device, skyboxPipeline, nullptr);
		skyboxPipeline = VK_NULL_HANDLE;
	}
	if (pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(gfxInit.device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}

	skyboxModel.~Model();
	uniformBuffer.~VulkanBuffer();
	instanceBuffer.~VulkanBuffer();
	shader.~VulkanShader();
	skyboxShader.~VulkanShader();
	terrainShader.~VulkanShader();
	descriptorSetLayout.~VulkanDescriptorSetLayout();
	noSkinUniformBuffer.~VulkanBuffer();
	camera.~Camera();

	if (sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(gfxInit.device, sampler, nullptr);
		sampler = VK_NULL_HANDLE;
	}
}

void GraphicsVK3D::bindOtherFramebuffer(FramebufferDrawStruct&& stateIn)
{
	state = stateIn;
}

void GraphicsVK3D::unbindOtherFramebuffer()
{
	state = gfxInit.getCurrentFramebufferStruct(pipeline);
}

void GraphicsVK3D::render()
{
	state = gfxInit.getNextFramebufferStruct(pipeline);
}

void GraphicsVK3D::draw(Model& model, const Mat4x4& mvPersProj, const VulkanTexture* texture)
{
	draw(model, mvPersProj, texture, noSkinUniformBuffer);
}

void GraphicsVK3D::draw(Model& model, const Mat4x4& mvPersProj, const VulkanTexture* texture, VulkanBuffer& skinUniformBuffer)
{
	// TODO: This is so slow. SO SLOW!!
	state.currentCommandBuffer->begin();
	uniformBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(Mat4x4), &mvPersProj);
	state.currentCommandBuffer->end();
	state.currentCommandBuffer->submit(gfxInit.queue, state.currentFence->fence);
	state.currentFence->wait();
	state.currentFence->reset();

	draw(model, Vector3f(), Vector3f(), Vector3f(1.0f, 1.0f, 1.0f), texture, skinUniformBuffer);

	state.currentCommandBuffer->begin();
	uniformBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(Mat4x4), &camera.getViewPersMat());
	state.currentCommandBuffer->end();
	state.currentCommandBuffer->submit(gfxInit.queue, state.currentFence->fence);
	state.currentFence->wait();
	state.currentFence->reset();
}

void GraphicsVK3D::draw(Model& model, const Vector3f& pos, const Vector3f& rot, const Vector3f& scl, const VulkanTexture* texture)
{	
	draw(model, pos, rot, scl, texture, noSkinUniformBuffer);
}

void GraphicsVK3D::draw(Model& model, const Vector3f& pos, const Vector3f& rot, const Vector3f& scl, const VulkanTexture* texture, VulkanBuffer& skinUniformBuffer)
{
	state.currentCommandBuffer->begin();

	VkDescriptorBufferInfo descriptorBufferInfo = {};
	descriptorBufferInfo.buffer = skinUniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = skinUniformBuffer.getSize();
	descriptorSetLayout.updateUniformBuffer(2, &descriptorBufferInfo);

	InstancedInfo instanceInfo = { pos, rot, scl };
	instanceBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(InstancedInfo), &instanceInfo);
	descriptorSetLayout.updateSamplerImage(1, texture->getTextureView(), sampler);

	state.currentCommandBuffer->beginRenderPass(state.currentRenderPassBeginInfo);

	vkCmdBindPipeline(state.currentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.currentPipeline);
	descriptorSetLayout.bind(state.currentCommandBuffer->commandBuffer);
	model.vertexBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 0);
	instanceBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 1);
	model.indexBuffer.bindIndexBuffer(state.currentCommandBuffer->commandBuffer);

	vkCmdDrawIndexed(state.currentCommandBuffer->commandBuffer, model.nIndices, 1, 0, 0, 0);

	state.currentCommandBuffer->endRenderPass();

	state.currentCommandBuffer->end();
	state.currentCommandBuffer->submit(gfxInit.queue, state.currentFence->fence);
	state.currentFence->wait();
	state.currentFence->reset();
}

void GraphicsVK3D::drawSkybox(const VulkanTexture* cubeMapTexture)
{
	Vector3f from;
	Vector3f to = (camera.rot * Vector3f{ 0.0f, 0.0f, 1.0f }).getNormalized();
	Mat4x4 mvPersMat = camera.persProj * Mat4x4::lookAt(from, to);

	state.currentCommandBuffer->begin();

	uniformBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(Mat4x4), &mvPersMat);
	descriptorSetLayout.updateSamplerImage(1, cubeMapTexture->getTextureView(), sampler);

	state.currentCommandBuffer->beginRenderPass(state.currentRenderPassBeginInfo);

	vkCmdBindPipeline(state.currentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);
	descriptorSetLayout.bind(state.currentCommandBuffer->commandBuffer);
	skyboxModel.vertexBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 0);
	skyboxModel.indexBuffer.bindIndexBuffer(state.currentCommandBuffer->commandBuffer);

	vkCmdDrawIndexed(state.currentCommandBuffer->commandBuffer, skyboxModel.nIndices, 1, 0, 0, 0);

	state.currentCommandBuffer->endRenderPass();
	
	uniformBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(Mat4x4), &camera.getViewPersMat());
	state.currentCommandBuffer->end();

	state.currentCommandBuffer->submit(gfxInit.queue, state.currentFence->fence);

	state.currentFence->wait();
	state.currentFence->reset();
}

void GraphicsVK3D::drawInstanced(Model& model, const Vector<InstancedInfo>& instancedInfos, const VulkanTexture* texture)
{
	drawInstanced(model, instancedInfos, texture, noSkinUniformBuffer);
}

void GraphicsVK3D::drawInstanced(Model& model, const Vector<struct InstancedInfo>& instancedInfos, const VulkanTexture* texture, VulkanBuffer& skinUniformBuffer)
{
	state.currentCommandBuffer->begin();

	VkDescriptorBufferInfo descriptorBufferInfo = {};
	descriptorBufferInfo.buffer = skinUniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = skinUniformBuffer.getSize();
	descriptorSetLayout.updateUniformBuffer(2, &descriptorBufferInfo);

	instanceBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(InstancedInfo) * instancedInfos.size(), instancedInfos.data());
	descriptorSetLayout.updateSamplerImage(1, texture->getTextureView(), sampler);

	state.currentCommandBuffer->beginRenderPass(state.currentRenderPassBeginInfo);

	vkCmdBindPipeline(state.currentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.currentPipeline);
	descriptorSetLayout.bind(state.currentCommandBuffer->commandBuffer);
	model.vertexBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 0);
	instanceBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 1);
	model.indexBuffer.bindIndexBuffer(state.currentCommandBuffer->commandBuffer);

	vkCmdDrawIndexed(state.currentCommandBuffer->commandBuffer, model.nIndices, instancedInfos.size(), 0, 0, 0);

	state.currentCommandBuffer->endRenderPass();
	state.currentCommandBuffer->end();

	state.currentCommandBuffer->submit(gfxInit.queue, state.currentFence->fence);

	state.currentFence->wait();
	state.currentFence->reset();
}

void GraphicsVK3D::drawTerrain(Model& model, const Vector3f& pos, const VulkanTexture* texture)
{
	state.currentCommandBuffer->begin();

	InstancedInfo instanceInfo = { pos, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f, 1.0f } };
	instanceBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(InstancedInfo), &instanceInfo);
	descriptorSetLayout.updateSamplerImage(1, texture->getTextureView(), sampler);

	state.currentCommandBuffer->beginRenderPass(state.currentRenderPassBeginInfo);

	vkCmdBindPipeline(state.currentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipeline);
	descriptorSetLayout.bind(state.currentCommandBuffer->commandBuffer);
	model.vertexBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 0);
	instanceBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 1);
	model.indexBuffer.bindIndexBuffer(state.currentCommandBuffer->commandBuffer);

	vkCmdDrawIndexed(state.currentCommandBuffer->commandBuffer, model.nIndices, 1, 0, 0, 0);

	state.currentCommandBuffer->endRenderPass();

	state.currentCommandBuffer->end();
	state.currentCommandBuffer->submit(gfxInit.queue, state.currentFence->fence);
	state.currentFence->wait();
	state.currentFence->reset();
}

void GraphicsVK3D::setupVertexLayoutForModelShader(VulkanShader& shader)
{
	shader.vertexLayout.addAttribute(3, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(2, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(3, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(4, VulkanVertexLayout::Type::UINT32);
	shader.vertexLayout.addAttribute(4, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.set(VK_VERTEX_INPUT_RATE_VERTEX);
	shader.vertexLayout.addAttribute(3, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(3, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(3, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.set(VK_VERTEX_INPUT_RATE_INSTANCE);
}
