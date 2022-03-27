#pragma once

#include "IGraphics.h"
#include "GraphicsOGL.h"
#include "VariableVector.h"

class GraphicsOGLIniter : public IGraphics
{
protected:
    View::ViewportType viewportType;
    View view;
    uint32_t rendererOffsets[4] = { 0 };
    VariableVector<GraphicsOGL> renderers;
public:
    GraphicsOGLIniter(uint32_t renderWidth, uint32_t renderHeight);

    void clear();
    template <typename T, typename... Args>
    void addRenderer(Args&&... args);
    template <typename T>
    T* getRenderer();
    GraphicsOGL* getRenderer(uint32_t rendererId);

    View& getDefaultView() { return view; }
};

template<typename T, typename... Args>
inline void GraphicsOGLIniter::addRenderer(Args&&... args)
{
    rendererOffsets[T::id] = renderers.getOffsetToEnd() + sizeof(uint32_t);
    renderers.push_back<T>(*this, std::forward<Args>(args)...);
    T* renderer = (T*)(renderers.begin() + rendererOffsets[T::id]);
    renderer->unbindOtherFramebuffer();
}

template<typename T>
inline T* GraphicsOGLIniter::getRenderer()
{
    return (T*)(renderers.begin() + rendererOffsets[T::id]);
}
