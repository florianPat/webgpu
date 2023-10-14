#include "Utils.h"
#include <cstdlib>

#ifdef EMSCRIPTEN
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
    #include <emscripten/html5_webgpu.h>
#else
    #include <dawn/webgpu.h>
    #include <dawn/dawn_proc.h>
    #include <dawn/native/DawnNative.h>
    #include "X11.h"
    #include <EGL/egl.h>
#endif

struct AppState
{
	WGPUSwapChain swapChain;
	WGPUDevice device;
	WGPUQueue queue;
    WGPUCommandEncoder encoder;
};

int winWidth = 640;
int winHeight = 480;

void loop_iteration(void *_appState);

static void requestAdapterCallback(WGPURequestAdapterStatus status,
                              WGPUAdapter received, const char *message,
                              void *userdata) {
    if (status != WGPURequestAdapterStatus::WGPURequestAdapterStatus_Success) {
        utils::logBreak("Could not get adapter!");
    }
    *(WGPUAdapter *)userdata = received;
}

static void requestDeviceCallback(WGPURequestDeviceStatus status,
                             WGPUDevice received, const char *message,
                             void *userdata) {
    if (status != WGPURequestDeviceStatus::WGPURequestDeviceStatus_Success) {
        utils::logBreak("Could not get the device!");
    }
    *(WGPUDevice *)userdata = received;
}

static void handleDeviceLost(WGPUDeviceLostReason reason, char const * message, void * userdata)
{
	utils::logFBreak("DEVICE LOST (%d): %s\n", reason, message);
}

static void handleUncapturedError(WGPUErrorType type, char const * message, void * userdata)
{
	utils::logFBreak("UNCAPTURED ERROR (%d): %s\n", type, message);
}

static void printDeviceError(WGPUErrorType errorType, const char* message, void*) {
    const char* errorTypeName = "";
    switch (errorType) {
        case WGPUErrorType_Validation:
            errorTypeName = "Validation";
            break;
        case WGPUErrorType_OutOfMemory:
            errorTypeName = "Out of memory";
            break;
        case WGPUErrorType_Unknown:
            errorTypeName = "Unknown";
            break;
        case WGPUErrorType_DeviceLost:
            errorTypeName = "Device lost";
            break;
        default:
            InvalidCodePath;
            return;
    }
    utils::logFBreak("%s error: %s", errorTypeName, message);
}

#ifndef EMSCRIPTEN
struct EglSwapBuffersInfo
{
    EGLDisplay display;
    EGLSurface surface;
};

static void presentCallback(void* userData)
{
    EglSwapBuffersInfo* info = (EglSwapBuffersInfo*) userData;
    eglSwapBuffers(info->display, info->surface);
}
#endif

int main(int argc, const char **argv)
{
#ifdef EMSCRIPTEN
    WGPUInstance instance;
    auto device = emscripten_webgpu_get_device();

	WGPUSurfaceDescriptorFromCanvasHTMLSelector surfaceDescritporCanvas = { 0 };
	surfaceDescritporCanvas.chain.next = nullptr;
	surfaceDescritporCanvas.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
	surfaceDescritporCanvas.selector = "#webgpu-canvas";

	WGPUSurfaceDescriptor surfaceDescriptor = { 0 };
	surfaceDescriptor.nextInChain = (const WGPUChainedStruct*)&surfaceDescritporCanvas;
#else
	Display* display = XOpenDisplay(nullptr);
    if (display == nullptr)
    {
        utils::logBreak("Could not open display");
    }
    Screen* screen = DefaultScreenOfDisplay(display);
    int32_t screenId = DefaultScreen(display);
    WindowHandle windowHandle = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, winWidth, winHeight, 1,
                                              BlackPixel(display, screenId), WhitePixel(display, screenId));

    XSelectInput(display, windowHandle, KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | ExposureMask);

    XStoreName(display, windowHandle, "3D Engine");

    XClearWindow(display, windowHandle);
    XMapRaised(display, windowHandle);

    EGLDisplay eglDisplay = eglGetDisplay((EGLNativeDisplayType) display);
    if (eglDisplay == EGL_NO_DISPLAY)
    {
        utils::logBreak("egl display could not be retreived!");
    }
    EGLint eglVersionMajor, eglVersionMinor;
    if (!eglInitialize(eglDisplay, &eglVersionMajor, &eglVersionMinor)) {
        utils::logBreak("Unable to initialize EGL");
    }

    EGLint eglConfigConstraints[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_CONFIG_CAVEAT, EGL_NONE,
        EGL_NONE
    };

    EGLConfig eglConf;
    EGLint numConfig;
    if (!eglChooseConfig(eglDisplay, eglConfigConstraints, &eglConf, 1, &numConfig)) {
        utils::logFBreak("Failed to choose config (eglError: %s)", eglGetError());
    }

    if (numConfig != 1) {
        utils::logFBreak("Didn't get exactly one config, but %d", numConfig);
    }

    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, eglConf, windowHandle, nullptr);
    if (eglSurface == EGL_NO_SURFACE) {
        utils::logFBreak("Unable to create EGL surface (eglError: %s)", eglGetError());
    }

    //// egl-contexts collect all state descriptions needed required for operation
    EGLint ctxattr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    EGLContext eglContext = eglCreateContext(eglDisplay, eglConf, EGL_NO_CONTEXT, ctxattr);
    if (eglContext == EGL_NO_CONTEXT) {
        utils::logFBreak("Unable to create EGL context (eglError: %s)", eglGetError());
    }

    //// associate the egl-context with the egl-surface
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    EGLint queriedRenderBuffer;
    if (!eglQueryContext(eglDisplay, eglContext, EGL_RENDER_BUFFER, &queriedRenderBuffer)) {
        utils::logFBreak("Failed to query EGL_RENDER_BUFFER: %d", eglGetError());
    }

    if (!eglSwapInterval(eglDisplay, 1)) {
        utils::logF("eglSwapInterval failed: %d", eglGetError());
    } else {
        utils::log("Set swap interval");
    }

    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    WGPUInstanceDescriptor instanceDescriptor = {};
    WGPUInstance instance = wgpuCreateInstance(&instanceDescriptor);

    if(instance == nullptr)
    {
        utils::logBreak("wgpu instance could not be created!");
    }

    WGPUSurfaceDescriptorFromXlibWindow surfaceDescritporXlib = { 0 };
	surfaceDescritporXlib.display = display;
	surfaceDescritporXlib.window = windowHandle;
    surfaceDescritporXlib.chain.next = nullptr;
    surfaceDescritporXlib.chain.sType = WGPUSType::WGPUSType_SurfaceDescriptorFromXlibWindow;

	WGPUSurfaceDescriptor surfaceDescriptor = { 0 };
    surfaceDescriptor.nextInChain = (WGPUChainedStruct*) &surfaceDescritporXlib;
#endif

    WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
	if (surface == nullptr)
	{
		utils::logBreak("wgpu surface could not be created!");
	}

    WGPUAdapter adapter;
    WGPURequestAdapterOptions requestAdapterOptions = {};
    requestAdapterOptions.compatibleSurface = surface;
    requestAdapterOptions.compatibilityMode = true;
    requestAdapterOptions.powerPreference = WGPUPowerPreference::WGPUPowerPreference_Undefined;
    requestAdapterOptions.forceFallbackAdapter = false;
    requestAdapterOptions.backendType = WGPUBackendType::WGPUBackendType_OpenGLES;
    wgpuInstanceRequestAdapter(instance, &requestAdapterOptions, requestAdapterCallback, (void*) &adapter);

    if (adapter == nullptr)
    {
        utils::logBreak("wgpu adapter could not be created!");
    }

    WGPUDevice device;
    WGPUDeviceDescriptor deviceDescriptor = { 0 };
    deviceDescriptor.nextInChain = nullptr;
    deviceDescriptor.requiredFeaturesCount = 0;
    deviceDescriptor.requiredLimits = nullptr;
    deviceDescriptor.deviceLostCallback = handleDeviceLost;
    wgpuAdapterRequestDevice(adapter, &deviceDescriptor, requestDeviceCallback, (void*) &device);

    if (device == nullptr)
    {
        utils::logBreak("wgpu device could not be created!");
    }

    wgpuDeviceSetUncapturedErrorCallback(device, handleUncapturedError, nullptr);
	wgpuDeviceSetDeviceLostCallback(device, handleDeviceLost, nullptr);

	WGPUQueue queue = wgpuDeviceGetQueue(device);
	if (queue == nullptr)
	{
		utils::logBreak("wgpu queue could not be created!");
	}

	WGPUSwapChain swapChain;

    WGPUSwapChainDescriptor swapChainDescriptor = { 0 };
    swapChainDescriptor.usage = WGPUTextureUsage::WGPUTextureUsage_RenderAttachment;
    swapChainDescriptor.presentMode = WGPUPresentMode::WGPUPresentMode_Fifo;
    // NOTE: Dawn does not implement wgpuSurfaceGetPreferredFormat yet...
#if defined(EMSCRIPTEN)
    WGPUTextureFormat preferredFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
    swapChainDescriptor.format = preferredFormat;
#else
    swapChainDescriptor.format = WGPUTextureFormat_BGRA8Unorm;
#endif
    swapChainDescriptor.height = winHeight;
    swapChainDescriptor.width = winWidth;
    swapChain = wgpuDeviceCreateSwapChain(device, surface, &swapChainDescriptor);

   if (swapChain == nullptr)
   {
       utils::logBreak("wgpu swap chain could not be created!");
   }

   WGPUCommandEncoderDescriptor commandEncoderDescriptor = { 0 };
   WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &commandEncoderDescriptor);

	AppState* appState = (AppState*) malloc(sizeof(AppState));
	appState->device = device;
	appState->queue = queue;
	appState->swapChain = swapChain;
    appState->encoder = encoder;

#ifdef EMSCRIPTEN
    emscripten_set_main_loop_arg(loop_iteration, appState, -1, 0);
#else
    while (1) {
        loop_iteration(appState);
    }
#endif
    return 0;
}

#ifdef EMSCRIPTEN
int mouseMoveCallback(int type, const EmscriptenMouseEvent *event, void *_app_state)
{
    return true;
}

int mouseWheelCallback(int type, const EmscriptenWheelEvent *event, void *_app_state)
{
    return true;
}
#endif

void loop_iteration(void *_appState)
{
    AppState *appState = reinterpret_cast<AppState *>(_appState);
#ifndef EMSCRIPTEN
    // TODO: Events
/*
	while (XPending(display) > 0)
    {
        XEvent xEvent;
        XNextEvent(display, &xEvent);

        switch(xEvent.type)
        {
            case KeymapNotify:
            {
                XRefreshKeyboardMapping(&xEvent.xmapping);
                break;
            }
            case KeyPress:
            {
                keyboard.setKey(xEvent.xkey.keycode, true);
                break;
            }
            case KeyRelease:
            {
                keyboard.setKey(xEvent.xkey.keycode, false);
                break;
            }
            case ButtonPress:
            {
                mouse.setButton(xEvent.xbutton.button);
                break;
            }
            case ButtonRelease:
            {
                mouse.setButton(~xEvent.xbutton.button);
                break;
            }
            case MotionNotify:
            {
                mouse.pos.x = xEvent.xmotion.x;
                mouse.pos.y = xEvent.xmotion.y;
                break;
            }
            case EnterNotify:
            {
                break;
            }
            case LeaveNotify:
            {
                break;
            }
            case Expose:
            {
                XWindowAttributes attributes;
                XGetWindowAttributes(display, windowHandle, &attributes);
                uint32_t newWidth = attributes.width;
                uint32_t newHeight = attributes.height;
                break;
            }
        }
    }
*/
#else
    emscripten_set_mousemove_callback("#webgpu-canvas", appState, true, mouseMoveCallback);
    emscripten_set_wheel_callback("#webgpu-canvas", appState, true, mouseWheelCallback);
#endif

	WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(appState->swapChain);
	if (nextTexture == nullptr)
	{
		utils::logBreak("wgpu next texture view could not be retrieved!");
	}

    WGPURenderPassDescriptor renderPassDescriptor = { 0 };
    WGPURenderPassColorAttachment renderPassColorAttachment = { 0 };
    renderPassColorAttachment.view = nextTexture;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = WGPULoadOp::WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp::WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{ 0.0f, 0.0f, 1.0f, 1.0f };

    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &renderPassColorAttachment;
    renderPassDescriptor.depthStencilAttachment = nullptr;
    renderPassDescriptor.timestampWriteCount = 0;
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(appState->encoder, &renderPassDescriptor);
    wgpuRenderPassEncoderEnd(renderPass);

#ifndef EMSCRIPTEN
	wgpuSwapChainPresent(appState->swapChain);
#endif
}
