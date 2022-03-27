#include "IWindow.h"

IWindow::IWindow(int32_t width, int32_t height) : clock(),
                                                  assetManager(),
                                                  gfx(width, height)
{
}

AssetManager* IWindow::getAssetManager()
{
    return &assetManager;
}

Clock& IWindow::getClock()
{
    return clock;
}

bool IWindow::startFont()
{
    int32_t error = FT_Init_FreeType(&fontLibrary);
    if(error)
        utils::log("could not init freetype library");

    return (error == 0);
}

void IWindow::stopFont()
{
    FT_Done_FreeType(fontLibrary);
    fontLibrary = nullptr;
}

FT_Library IWindow::getFontLibrary()
{
    return fontLibrary;
}

GraphicsIniter& IWindow::getGfx()
{
    return gfx;
}


NativeThreadQueue& IWindow::getNativeThreadQueue()
{
    return nativeThreadQueue;
}


void IWindow::callToGetEventJobsBeginning()
{
    threadQueueEventBeginning = nativeThreadQueue.getSize();
}

