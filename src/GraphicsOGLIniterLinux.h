#pragma once

#include "GraphicsOGLIniter.h"
#include "X11.h"

class GraphicsOGLIniterLinux : public GraphicsOGLIniter
{
protected:
    //static constexpr GLint DISPLAY_ATTRIBS[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    //GLXContext glContext;
public:
    GraphicsOGLIniterLinux(uint32_t renderWidth, uint32_t renderHeight);

    bool startGfx(WindowHandle* WindowHandle);
    void stopGfx();
    void render();
};
