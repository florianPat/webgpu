#pragma once

#include "View.h"

class IGraphics
{
public:
    //NOTE: These a really just meant to be use by the TouchInput.
    uint32_t renderWidth = 0, renderHeight = 0;
    //NOTE: This is just meant to be modified by the Window (but friend setters... I do not know)
    uint32_t screenWidth = 0, screenHeight = 0;
public:
    IGraphics(uint32_t renderWidth, uint32_t renderHeight) : renderWidth(renderWidth), renderHeight(renderHeight)
    {}
    IGraphics(const IGraphics& other) = delete;
    IGraphics& operator=(const IGraphics& rhs) = delete;
    IGraphics(IGraphics&& other) = delete;
    IGraphics& operator=(IGraphics&& rhs) = delete;

    //NOTE: This is a blueprint to implement.
    // But not through virtual function calls, just by deriving and making the methods
    /*
    bool startGfx();
    void stopGfx();
    void checkIfToRecover();
    void recover();

    void setupGfxGpu();

    void clear();
    void bindOtherOrthoProj(const Mat4x4& otherOrthoProj);
    void unbindOtherOrthoProj();
    void draw(const Sprite& sprite);
    void draw(const RectangleShape& rect);
    void draw(const CircleShape& circle);
    void flush();
    void render();
    View& getDefaultView();
    int32_t getWidth();
    int32_t getHeight();
     */
};