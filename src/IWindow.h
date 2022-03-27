#pragma once

#include "Clock.h"
#include "AssetManager.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "NativeThreadQueue.h"
#include "Graphics.h"

class IWindow
{
protected:
    bool initFinished = false;
    bool running = true;

    Clock clock;
    AssetManager assetManager;
    FT_Library fontLibrary = nullptr;
    GraphicsIniter gfx;

    NativeThreadQueue nativeThreadQueue;
    uint32_t threadQueueEventBeginning = 0;
public:
    IWindow(int32_t width, int32_t height);
    AssetManager* getAssetManager();
    Clock& getClock();
    FT_Library getFontLibrary();
    GraphicsIniter& getGfx();
    NativeThreadQueue& getNativeThreadQueue();
    void callToGetEventJobsBeginning();
protected:
    bool startFont();
    void stopFont();
};
