#pragma once

#include "GraphicsOGLIniter.h"
#include <GLES2/gl2.h>
#include "android_native_app_glue.h"

class GraphicsOGLIniterAndroid : public GraphicsOGLIniter
{
protected:
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    bool glContextLost = false;
    static constexpr EGLint DISPLAY_ATTRIBS[] = {
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };
private:
    bool initDisplay(EGLConfig& config, ANativeWindow* nativeWindow);
    bool initSurface(EGLConfig& config, ANativeWindow* nativeWindow);
    bool initContext(EGLConfig& config);
public:
    GraphicsOGLIniterAndroid(uint32_t renderWidth, uint32_t renderHeight, View::ViewportType viewportType);

    bool startGfx(ANativeWindow* nativeWindow);
    void stopGfx();
    bool checkIfToRecover();
    void recover(ANativeWindow* nativeWindow);
    void render();

    bool isRecovered() const;
};
