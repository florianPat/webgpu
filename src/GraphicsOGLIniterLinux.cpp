#include "GraphicsOGLIniterLinux.h"
#include <GL/glx.h>

GraphicsOGLIniterLinux::GraphicsOGLIniterLinux(uint32_t renderWidth, uint32_t renderHeight)
    : GraphicsOGLIniter(renderWidth, renderHeight)
{
}

bool GraphicsOGLIniterLinux::startGfx(WindowHandle *windowHandle)
{
    GLint majorGLX = 0, minorGLX = 0;

    return true;
}

void GraphicsOGLIniterLinux::stopGfx()
{

}

void GraphicsOGLIniterLinux::render()
{

}
