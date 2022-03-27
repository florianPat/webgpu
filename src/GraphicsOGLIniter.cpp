#include "GraphicsOGLIniter.h"
#include "GLUtils.h"

GraphicsOGL *GraphicsOGLIniter::getRenderer(uint32_t rendererId)
{
    return (GraphicsOGL*)(renderers.begin() + rendererOffsets[rendererId]);
}

GraphicsOGLIniter::GraphicsOGLIniter(uint32_t renderWidth, uint32_t renderHeight) : IGraphics(renderWidth, renderHeight),
                                                                     view(),
                                                                     renderers(0)
{
}

void GraphicsOGLIniter::clear()
{
    /*CallGL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    CallGL(glClear(GL_COLOR_BUFFER_BIT));*/
}
