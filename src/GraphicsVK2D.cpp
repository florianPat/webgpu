#include "GraphicsVK2D.h"

GraphicsVK2D::GraphicsVK2D(GraphicsVKIniter& gfxInit, View::ViewportType viewportType)
    : GraphicsVK(gfxInit), viewportType(viewportType),
      view(gfxInit.renderWidth, gfxInit.renderHeight, gfxInit.screenWidth, gfxInit.screenHeight, viewportType),
      vertices(makeUnique<Vertex[]>(NUM_VERTICES_TO_BATCH)),
      orthoProj(view.getOrthoProj()), sampler(gfxInit.createSampler()),
      vertexBuffer(sizeof(Vertex)* NUM_VERTICES_TO_BATCH, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
          gfxInit),
      uniformBuffer(sizeof(orthoProj), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, gfxInit)
{
    new (&shader) VulkanShader("shaders/RectSprite", gfxInit.device);

    Mat4x4* mappedData = (Mat4x4*)uniformBuffer.map(0, sizeof(Mat4x4));
    new (mappedData) Mat4x4(orthoProj);
    uniformBuffer.unmap(0, sizeof(Mat4x4));

    Vector2f spriteVertices[] = { {0.0f, 0.0f},
                                  {1.0f, 0.0f},
                                  {1.0f, 1.0f},
                                  {0.0f, 1.0f} };
    for(int i = 0; i < NUM_VERTICES_TO_BATCH; i += 4)
    {
        vertices[i + 0].position = spriteVertices[0];
        vertices[i + 1].position = spriteVertices[1];
        vertices[i + 2].position = spriteVertices[2];
        vertices[i + 3].position = spriteVertices[3];
    }

    shader.vertexLayout.addAttribute(2, VulkanVertexLayout::Type::FLOAT);
    shader.vertexLayout.addAttribute(2, VulkanVertexLayout::Type::FLOAT);
    shader.vertexLayout.addAttribute(4, VulkanVertexLayout::Type::FLOAT);
    shader.vertexLayout.addAttribute(4, VulkanVertexLayout::Type::FLOAT);
    shader.vertexLayout.addAttribute(2, VulkanVertexLayout::Type::FLOAT);
    shader.vertexLayout.set(VK_VERTEX_INPUT_RATE_VERTEX);

    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {};
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
    new (&descriptorSetLayout) VulkanDescriptorSetLayout(descriptorSetLayoutBindings, arrayCount(descriptorSetLayoutBindings),
        gfxInit.device);
    VkDescriptorBufferInfo descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = uniformBuffer.buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = uniformBuffer.getSize();
    descriptorSetLayout.updateUniformBuffer(0, &descriptorBufferInfo);

    new (&indexBuffer) VulkanBuffer(sizeof(uint16_t) * 6 * NUM_SPRITES_TO_BATCH, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, gfxInit);
    uint16_t* indices = (uint16_t*) indexBuffer.map(0, VK_WHOLE_SIZE);
    for (uint16_t i = 0, counter = 0; i < NUM_SPRITES_TO_BATCH; ++i, counter += 6)
    {
        indices[counter + 0] = 0 + i * 4;
        indices[counter + 1] = 1 + i * 4;
        indices[counter + 2] = 2 + i * 4;
        indices[counter + 3] = 2 + i * 4;
        indices[counter + 4] = 3 + i * 4;
        indices[counter + 5] = 0 + i * 4;
    }
    indexBuffer.unmap(0, VK_WHOLE_SIZE);

    uint8_t buffer[] = { 255, 255, 255, 255 };
    new (&blackTexture) VulkanTexture(buffer, 1, 1);

    pipeline = gfxInit.createWindowPipeline(shader, descriptorSetLayout, enableAlpha, VK_NULL_HANDLE);
}

void GraphicsVK2D::render()
{
	flush();

    if (view.updated())
    {
        //TODO: Optimize
        orthoProj = view.getOrthoProj();
        state.currentCommandBuffer->begin();
        uniformBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(Mat4x4), &orthoProj);
        state.currentCommandBuffer->end();
    }

    state = gfxInit.getNextFramebufferStruct(pipeline);
}

void GraphicsVK2D::flush()
{
	if (nSpritesBatched != 0)
	{
        state.currentCommandBuffer->begin();

        vertexBuffer.cmdSubData(*state.currentCommandBuffer, 0, sizeof(Vertex) * nVerticesBatched(), vertices.get());

        state.currentCommandBuffer->beginRenderPass(state.currentRenderPassBeginInfo);

		vkCmdBindPipeline(state.currentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.currentPipeline);
		vertexBuffer.bindVertexBuffer(state.currentCommandBuffer->commandBuffer, 0);
		indexBuffer.bindIndexBuffer(state.currentCommandBuffer->commandBuffer);
		descriptorSetLayout.bind(state.currentCommandBuffer->commandBuffer);

		vkCmdDrawIndexed(state.currentCommandBuffer->commandBuffer, 6 * nSpritesBatched, 6, 0, 0, 0);

        state.currentCommandBuffer->endRenderPass();
        state.currentCommandBuffer->end();

        state.currentCommandBuffer->submit(gfxInit.queue, VK_NULL_HANDLE, VK_NULL_HANDLE, state.currentFence->fence);

		nSpritesBatched = 0;

		//NOTE: Need it here because changing state after this...
        state.currentFence->wait();
        state.currentFence->reset();
	}
}

GraphicsVK2D::~GraphicsVK2D()
{
    if (pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(gfxInit.device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }

	shader.~VulkanShader();
	vertexBuffer.~VulkanBuffer();
	indexBuffer.~VulkanBuffer();
	uniformBuffer.~VulkanBuffer();
	blackTexture.~VulkanTexture();
	descriptorSetLayout.~VulkanDescriptorSetLayout();

    if(sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(gfxInit.device, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }
}

void GraphicsVK2D::draw(const CircleShape& circle)
{
    //TODO: Implement!
}

void GraphicsVK2D::bindOtherOrthoProj(const Mat4x4& otherOrthoProj)
{
    Mat4x4* mappedData = (Mat4x4*) uniformBuffer.map(0, sizeof(Mat4x4));
    new (mappedData) Mat4x4(otherOrthoProj);
    uniformBuffer.unmap(0, sizeof(Mat4x4));
}

void GraphicsVK2D::unbindOtherOrthoProj()
{
    Mat4x4* mappedData = (Mat4x4*)uniformBuffer.map(0, sizeof(Mat4x4));
    new (mappedData) Mat4x4(orthoProj);
    uniformBuffer.unmap(0, sizeof(Mat4x4));
}

void GraphicsVK2D::bindOtherFramebuffer(FramebufferDrawStruct&& stateIn)
{
	flush();
    state = stateIn;
}

void GraphicsVK2D::unbindOtherFramebuffer()
{
	flush();
    state = gfxInit.getCurrentFramebufferStruct(pipeline);
}

void GraphicsVK2D::draw(const Sprite& sprite)
{
    assert(vertexBuffer);

    const Texture* texture = sprite.getTexture();
    assert(*texture);

    if(currentBoundTexture != texture->getTextureView())
    {
        if(nSpritesBatched != 0)
            flush();

        descriptorSetLayout.updateSamplerImage(1, texture->getTextureView(), sampler);

        currentBoundTexture = texture->getTextureView();
    }
    else if(nSpritesBatched >= NUM_SPRITES_TO_BATCH)
    {
        flush();
    }

    float texRectLeft = ((float)sprite.getTextureRect().left) / texture->getWidth();
    float texRectTop = ((float)sprite.getTextureRect().bottom) / texture->getHeight();
    float texRectRight = ((float)sprite.getTextureRect().getRight()) / texture->getWidth();
    float texRectBottom = ((float) sprite.getTextureRect().getTop()) / texture->getHeight();
    Vector2f texCoord[4] = { { texRectLeft, texRectBottom },
                             { texRectRight, texRectBottom },
                             { texRectRight, texRectTop },
                             { texRectLeft, texRectTop } };

    float red = sprite.color.r;
    float green = sprite.color.g;
    float blue = sprite.color.b;
    float alpha = sprite.color.a;

    Mat4x4 mv = sprite.getTransform();

    int32_t plusI = nVerticesBatched();

    for(int i = 0; i < 4; ++i)
    {
        vertices[i + plusI].tex = texCoord[i];
        vertices[i + plusI].colorR = red;
        vertices[i + plusI].colorG = green;
        vertices[i + plusI].colorB = blue;
        vertices[i + plusI].colorA = alpha;
        vertices[i + plusI].mvMatrix[0] = mv.matrix[0];
        vertices[i + plusI].mvMatrix[1] = mv.matrix[1];
        vertices[i + plusI].mvMatrix[2] = mv.matrix[4];
        vertices[i + plusI].mvMatrix[3] = mv.matrix[5];
        vertices[i + plusI].mvMatrix[4] = mv.matrix[12];
        vertices[i + plusI].mvMatrix[5] = mv.matrix[13];
    }

    ++nSpritesBatched;
}

void GraphicsVK2D::draw(const RectangleShape& rect)
{
    assert(vertexBuffer);

    const Texture* texture = &blackTexture;
    assert(*texture);

    if (currentBoundTexture != texture->getTextureView())
    {
        if (nSpritesBatched != 0)
            flush();

        descriptorSetLayout.updateSamplerImage(1, texture->getTextureView(), sampler);

        currentBoundTexture = texture->getTextureView();
    }
    else if(nSpritesBatched >= NUM_SPRITES_TO_BATCH)
    {
        flush();
    }

    float red = rect.fillColor.r;
    float green = rect.fillColor.g;
    float blue = rect.fillColor.b;
    float alpha = rect.fillColor.a;

    Mat4x4 mv = rect.getTransform();

    int32_t plusI = nVerticesBatched();

    for(int i = 0; i < 4; ++i)
    {
        vertices[i + plusI].tex = { 0.0f, 0.0f };
        vertices[i + plusI].colorR = red;
        vertices[i + plusI].colorG = green;
        vertices[i + plusI].colorB = blue;
        vertices[i + plusI].colorA = alpha;
        vertices[i + plusI].mvMatrix[0] = mv.matrix[0];
        vertices[i + plusI].mvMatrix[1] = mv.matrix[1];
        vertices[i + plusI].mvMatrix[2] = mv.matrix[4];
        vertices[i + plusI].mvMatrix[3] = mv.matrix[5];
        vertices[i + plusI].mvMatrix[4] = mv.matrix[12];
        vertices[i + plusI].mvMatrix[5] = mv.matrix[13];
    }

    ++nSpritesBatched;
}

int32_t GraphicsVK2D::nVerticesBatched() const
{
    return (nSpritesBatched * 4);
}
