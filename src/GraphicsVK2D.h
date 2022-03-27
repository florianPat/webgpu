#pragma once

#include "Mat4x4.h"
#include "RectangleShape.h"
#include "UniquePtr.h"
#include "CircleShape.h"
#include "VulkanBuffer.h"
#include "GraphicsVK.h"
#include "Sprite.h"
#include "VulkanTexture.h"

//TODO: Can you multithread that?
class GraphicsVK2D : public GraphicsVK
{
    struct Vertex
    {
        Vector2f position;
        Vector2f tex;
        float colorR = 0.0f, colorG = 0.0f, colorB = 0.0f, colorA = 0.0f;
		float mvMatrix[6] = { 0.0f };
    };
private:
	View::ViewportType viewportType;
	View view;
	static constexpr int32_t NUM_SPRITES_TO_BATCH = 32;
	static constexpr int32_t NUM_VERTICES_TO_BATCH = NUM_SPRITES_TO_BATCH * 4;
	int32_t nSpritesBatched = 0;
	UniquePtr<Vertex[]> vertices;
	VkImageView currentBoundTexture = VK_NULL_HANDLE;
	VulkanTexture blackTexture;
	VkSampler sampler = VK_NULL_HANDLE;
	Mat4x4 orthoProj;

	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;
	VulkanBuffer uniformBuffer;

	static constexpr VkBool32 enableAlpha = VK_TRUE;
public:
	static constexpr uint32_t id = 0;
public:
    GraphicsVK2D(GraphicsVKIniter& gfxInit, View::ViewportType viewportType);
	~GraphicsVK2D();

	void bindOtherFramebuffer(FramebufferDrawStruct&& stateIn);
	void unbindOtherFramebuffer();
    void bindOtherOrthoProj(const Mat4x4& otherOrthoProj);
    void unbindOtherOrthoProj();
    void draw(const Sprite& sprite);
    void draw(const RectangleShape& rect);
    void draw(const CircleShape& circle);
    void flush();
    void render() override;

	View& getDefaultView() { return view; }
private:
    int32_t nVerticesBatched() const;
};
