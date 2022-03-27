#pragma once

class GraphicsOGL
{
protected:
    class GraphicsOGLIniter& gfxInit;
public:
    GraphicsOGL(GraphicsOGLIniter& gfxInit) : gfxInit(gfxInit) {}
    GraphicsOGL(const GraphicsOGL& other) = delete;
    GraphicsOGL& operator=(const GraphicsOGL& rhs) = delete;
    GraphicsOGL(GraphicsOGL&& other) = delete;
    GraphicsOGL& operator=(GraphicsOGL&& rhs) = delete;
    virtual ~GraphicsOGL() {}
    virtual void render() {}
};
