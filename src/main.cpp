#include "Utils.h"
#include <cstdlib>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#else
    #include <dawn/webgpu.h>
    #include <dawn/webgpu_cpp.h>
    #include <dawn/dawn_proc.h>
    #include <dawn/native/DawnNative.h>
    #include <dawn/native/OpenGLBackend.h>
    #include "X11.h"
    #include <EGL/egl.h>
    #include <GLES3/gl3.h>
#endif

const char* WGSL_SHADER = R"(
@stage(vertex)
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(in_vertex_index) - 1);
    let y = f32(i32(in_vertex_index & 1u) * 2 - 1);
    return vec4<f32>(x, y, 0.0, 1.0);
}

@stage(fragment)
fn fs_main() -> @location(0) vec4<f32> {
    return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}
)";

const char* WGLS_WGPUNATIVE_SHADER = R"(
[[stage(vertex)]]
fn vs_main([[builtin(vertex_index)]] in_vertex_index: u32) -> [[builtin(position)]] vec4<f32> {
    let x = f32(i32(in_vertex_index) - 1);
    let y = f32(i32(in_vertex_index & 1u) * 2 - 1);
    return vec4<f32>(x, y, 0.0, 1.0);
}

[[stage(fragment)]]
fn fs_main() -> [[location(0)]] vec4<f32> {
    return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}   
)";

struct AppState
{
	WGPUSwapChain swapChain;
	WGPUDevice device;
	WGPUQueue queue;
	WGPURenderPipeline pipeline;
};

int winWidth = 640;
int winHeight = 480;

void loop_iteration(void *_appState);

static void requestAdapterCallback(WGPURequestAdapterStatus status,
                              WGPUAdapter received, const char *message,
                              void *userdata) {
  *(WGPUAdapter *)userdata = received;
}

static void requestDeviceCallback(WGPURequestDeviceStatus status,
                             WGPUDevice received, const char *message,
                             void *userdata) {
  *(WGPUDevice *)userdata = received;
}

static void handleDeviceLost(WGPUDeviceLostReason reason, char const * message, void * userdata)
{
	utils::logF("DEVICE LOST (%d): %s\n", reason, message);
}

static void handleUncapturedError(WGPUErrorType type, char const * message, void * userdata)
{
	utils::logF("UNCAPTURED ERROR (%d): %s\n", type, message);
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

#ifndef WGPUNATIVE
    auto dawnInstance = dawn::native::Instance();
    dawnInstance.DiscoverDefaultAdapters();
    dawn::native::opengl::PhysicalDeviceDiscoveryOptions adapterOptionsES(WGPUBackendType::WGPUBackendType_OpenGLES);
    adapterOptionsES.getProc = (void* (*)(const char*))eglGetProcAddress;
    bool result = dawnInstance.DiscoverAdapters(&adapterOptionsES);
    if(!result)
    {
        utils::logBreak("dawn could not discover adapters!");
    }

    // Get an adapter for the backend to use, and create the device.
    dawn::native::Adapter backendAdapter;
    {
        std::vector<dawn::native::Adapter> adapters = dawnInstance.GetAdapters();
        auto adapterIt = std::find_if(adapters.begin(), adapters.end(),
            [](const dawn::native::Adapter adapter) -> bool {
                wgpu::AdapterProperties properties = {};
                adapter.GetProperties(&properties);
                return properties.backendType != wgpu::BackendType::Null && properties.adapterType != wgpu::AdapterType::CPU;
            });
        assert(adapterIt != adapters.end());
        backendAdapter = *adapterIt;
    }
    WGPUDevice device = backendAdapter.CreateDevice();
    DawnProcTable backendProcs = dawn::native::GetProcs();

    dawnProcSetProcs(&backendProcs);

    WGPUInstance instance = dawnInstance.Get();
#else
    WGPUInstanceDescriptor instanceDescriptor = {};
    WGPUInstance instance = wgpuCreateInstance(&instanceDescriptor);
#endif
    if(instance == nullptr)
    {
        utils::logBreak("wgpu instance could not be created!");
    }

    WGPUSurfaceDescriptorFromXlibWindow surfaceDescritporXlib = { 0 };
	surfaceDescritporXlib.chain.next = nullptr;
	surfaceDescritporXlib.chain.sType = WGPUSType_SurfaceDescriptorFromXlibWindow;
	surfaceDescritporXlib.display = display;
	surfaceDescritporXlib.window = windowHandle;

	WGPUSurfaceDescriptor surfaceDescriptor = { 0 };
	surfaceDescriptor.nextInChain = (const WGPUChainedStruct*)&surfaceDescritporXlib;
#endif

    WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
	if (surface == nullptr)
	{
		utils::logBreak("wgpu surface could not be created!");
	}

#ifdef WGPUNATIVE
    WGPUAdapter adapter;
    WGPURequestAdapterOptions requestAdapterOptions = {};
    requestAdapterOptions.compatibleSurface = surface;

    /*WGPUAdapterExtras adapterExtras = {};
    adapterExtras.chain.next = nullptr;
    adapterExtras.chain.sType = (WGPUSType) WGPUSType_AdapterExtras;
    adapterExtras.backend = WGPUBackendType_OpenGL;
    requestAdapterOptions.nextInChain = (const WGPUChainedStruct*) &adapterExtras;*/
    wgpuInstanceRequestAdapter(instance, &requestAdapterOptions, requestAdapterCallback, (void*) &adapter);

    if (adapter == nullptr)
    {
        utils::logBreak("wgpu adapter could not be created!");
    }

    WGPUDevice device;

    WGPUDeviceDescriptor deviceDescriptor = {};
    WGPUDeviceExtras deviceExtras = {};
    deviceExtras.chain.next = nullptr;
    deviceExtras.chain.sType = (WGPUSType) WGPUSType_DeviceExtras;

    deviceExtras.label = "Device";
    deviceExtras.tracePath = nullptr;
    deviceDescriptor.nextInChain = (const WGPUChainedStruct*) &deviceExtras;

    WGPURequiredLimits requiredLimits = {};
    requiredLimits.nextInChain = nullptr;
    requiredLimits.limits.maxBindGroups = 1;
    deviceDescriptor.requiredLimits = &requiredLimits;
    wgpuAdapterRequestDevice(adapter, &deviceDescriptor, requestDeviceCallback, (void*) &device);
#endif
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

#if !defined(EMSCRIPTEN) && !defined(WGPUNATIVE)
    EglSwapBuffersInfo swapBuffersInfo = {};
    swapBuffersInfo.display = eglDisplay;
    swapBuffersInfo.surface = eglSurface;
    WGPUSwapChainDescriptor swapChainDescriptor = { 0 };
    swapChainDescriptor.usage = WGPUTextureUsage::WGPUTextureUsage_RenderAttachment;
    swapChainDescriptor.presentMode = WGPUPresentMode::WGPUPresentMode_Fifo;
    swapChainDescriptor.format = WGPUTextureFormat::WGPUTextureFormat_RGBA8UnormSrgb;
    swapChainDescriptor.height = winHeight;
    swapChainDescriptor.width = winWidth;
    swapChain = wgpuDeviceCreateSwapChain(device, nullptr, &swapChainDescriptor);
#else
#ifdef WGPUNATIVE
    WGPUTextureFormat preferredFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
#else
    WGPUTextureFormat preferredFormat = wgpuSurfaceGetPreferredFormat(surface, nullptr);
#endif

	WGPUSwapChainDescriptor swapChainDescriptor = { 0 };
	swapChainDescriptor.usage = WGPUTextureUsage_RenderAttachment;
	swapChainDescriptor.format = preferredFormat;
	swapChainDescriptor.width = winWidth;
	swapChainDescriptor.height = winHeight;
	swapChainDescriptor.presentMode = WGPUPresentMode_Fifo;
    swapChain = wgpuDeviceCreateSwapChain(device, surface, &swapChainDescriptor);
    
#endif
   if (swapChain == nullptr)
   {
       utils::logBreak("wgpu swap chain could not be created!");
   }
#if !defined(EMSCRIPTEN) && !defined(WGPUNATIVE)
   //wgpuSwapChainConfigure(swapChain, preferredFormat, WGPUTextureUsage_RenderAttachment, winWidth, winHeight);
#endif

	WGPUShaderModuleDescriptor shaderSource = { 0 };
	shaderSource.label = "Shader Source";
	WGPUShaderModuleWGSLDescriptor wgslDescriptor = { 0 };
	wgslDescriptor.chain.next = nullptr;
	wgslDescriptor.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#ifdef WGPUNATIVE
    wgslDescriptor.code = WGLS_WGPUNATIVE_SHADER;
#else
	// wgslDescriptor.source = WGSL_SHADER;
#endif
	shaderSource.nextInChain = (const WGPUChainedStruct*) &wgslDescriptor;

	WGPUShaderModule shader = wgpuDeviceCreateShaderModule(device, &shaderSource);
	if (shader == nullptr)
	{
		utils::logBreak("wgpu shader could not be created!");
	}

	WGPUPipelineLayout pipelineLayout;

	WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = { 0 };
	pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
	pipelineLayoutDescriptor.bindGroupLayouts = nullptr;
	pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDescriptor);
	if (pipelineLayout == nullptr)
	{
		utils::logBreak("wgpu pipeline layout could not be created!");
	}

	WGPURenderPipeline renderPipeline;

	WGPURenderPipelineDescriptor renderPipelineDescriptor = { 0 };
	renderPipelineDescriptor.label = "Render pipeline";
	renderPipelineDescriptor.layout = pipelineLayout;
	renderPipelineDescriptor.vertex.module = shader;
	renderPipelineDescriptor.vertex.entryPoint = "vs_main";
	renderPipelineDescriptor.vertex.bufferCount = 0;
	renderPipelineDescriptor.vertex.buffers = nullptr;
	renderPipelineDescriptor.vertex.nextInChain = nullptr;
	renderPipelineDescriptor.vertex.constantCount = 0;
	renderPipelineDescriptor.vertex.constants = nullptr;
	renderPipelineDescriptor.primitive.nextInChain = nullptr;
	renderPipelineDescriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
	renderPipelineDescriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
	renderPipelineDescriptor.primitive.frontFace = WGPUFrontFace_CCW;
	renderPipelineDescriptor.primitive.cullMode = WGPUCullMode_None;
	renderPipelineDescriptor.multisample.nextInChain = nullptr;
	renderPipelineDescriptor.multisample.mask = ~0;
	renderPipelineDescriptor.multisample.count = 1;
	renderPipelineDescriptor.multisample.alphaToCoverageEnabled = false;
	renderPipelineDescriptor.depthStencil = nullptr;
	
	WGPUFragmentState fragmentState = { 0 };
	fragmentState.module = shader;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;
	fragmentState.targetCount = 1;

	WGPUColorTargetState colorTargetState = { 0 };
	colorTargetState.format = WGPUTextureFormat::WGPUTextureFormat_RGBA8UnormSrgb;
	colorTargetState.writeMask = WGPUColorWriteMask_All;

	WGPUBlendState blendState = {};
	blendState.color.srcFactor = WGPUBlendFactor_One;
	blendState.color.dstFactor = WGPUBlendFactor_Zero;
	blendState.color.operation = WGPUBlendOperation_Add;
	blendState.alpha.srcFactor = WGPUBlendFactor_One;
	blendState.alpha.dstFactor = WGPUBlendFactor_Zero;
	blendState.alpha.operation = WGPUBlendOperation_Add;
	colorTargetState.blend = &blendState;

	fragmentState.targets = &colorTargetState;
	renderPipelineDescriptor.fragment = &fragmentState;

	renderPipeline = wgpuDeviceCreateRenderPipeline(device, &renderPipelineDescriptor);
	if (renderPipeline == nullptr)
	{
		utils::logBreak("wgpu render pipeline could not be created!");
	}

/*
    // Upload vertex data
    const std::vector<float> vertex_data = {
        1,  -1, 0, 1,  // position
        1,  0,  0, 1,  // color
        -1, -1, 0, 1,  // position
        0,  1,  0, 1,  // color
        0,  1,  0, 1,  // position
        0,  0,  1, 1,  // color
    };
    wgpu::BufferDescriptor buffer_desc;
    buffer_desc.mappedAtCreation = true;
    buffer_desc.size = vertex_data.size() * sizeof(float);
    buffer_desc.usage = wgpu::BufferUsage::Vertex;
    app_state->vertex_buf = app_state->device.CreateBuffer(&buffer_desc);
    std::memcpy(app_state->vertex_buf.GetMappedRange(), vertex_data.data(), buffer_desc.size);
    app_state->vertex_buf.Unmap();

    std::array<wgpu::VertexAttribute, 2> vertex_attributes;
    vertex_attributes[0].format = wgpu::VertexFormat::Float32x4;
    vertex_attributes[0].offset = 0;
    vertex_attributes[0].shaderLocation = 0;

    vertex_attributes[1].format = wgpu::VertexFormat::Float32x4;
    vertex_attributes[1].offset = 4 * 4;
    vertex_attributes[1].shaderLocation = 1;

    wgpu::VertexBufferLayout vertex_buf_layout;
    vertex_buf_layout.arrayStride = 2 * 4 * 4;
    vertex_buf_layout.attributeCount = vertex_attributes.size();
    vertex_buf_layout.attributes = vertex_attributes.data();

    wgpu::VertexState vertex_state;
    vertex_state.module = shader_module;
    vertex_state.entryPoint = "vertex_main";
    vertex_state.bufferCount = 1;
    vertex_state.buffers = &vertex_buf_layout;

    wgpu::ColorTargetState render_target_state;
    render_target_state.format = wgpu::TextureFormat::BGRA8Unorm;

    wgpu::FragmentState fragment_state;
    fragment_state.module = shader_module;
    fragment_state.entryPoint = "fragment_main";
    fragment_state.targetCount = 1;
    fragment_state.targets = &render_target_state;

    wgpu::BindGroupLayoutEntry view_param_layout_entry = {};
    view_param_layout_entry.binding = 0;
    view_param_layout_entry.buffer.hasDynamicOffset = false;
    view_param_layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;
    view_param_layout_entry.visibility = wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor view_params_bg_layout_desc = {};
    view_params_bg_layout_desc.entryCount = 1;
    view_params_bg_layout_desc.entries = &view_param_layout_entry;

    wgpu::BindGroupLayout view_params_bg_layout =
        app_state->device.CreateBindGroupLayout(&view_params_bg_layout_desc);

    wgpu::PipelineLayoutDescriptor pipeline_layout_desc = {};
    pipeline_layout_desc.bindGroupLayoutCount = 1;
    pipeline_layout_desc.bindGroupLayouts = &view_params_bg_layout;

    wgpu::PipelineLayout pipeline_layout =
        app_state->device.CreatePipelineLayout(&pipeline_layout_desc);

#ifndef __EMSCRIPTEN__
    wgpu::RenderPipelineDescriptor render_pipeline_desc;
#else
    // Emscripten is behind Dawn
    wgpu::RenderPipelineDescriptor2 render_pipeline_desc;
#endif
    render_pipeline_desc.vertex = vertex_state;
    render_pipeline_desc.fragment = &fragment_state;
    render_pipeline_desc.layout = pipeline_layout;
    // Default primitive state is what we want, triangle list, no indices

#ifndef __EMSCRIPTEN__
    app_state->render_pipeline = app_state->device.CreateRenderPipeline(&render_pipeline_desc);
#else
    // Emscripten is behind Dawn
    app_state->render_pipeline =
        app_state->device.CreateRenderPipeline2(&render_pipeline_desc);
#endif

    // Create the UBO for our bind group
    wgpu::BufferDescriptor ubo_buffer_desc;
    ubo_buffer_desc.mappedAtCreation = false;
    ubo_buffer_desc.size = 16 * sizeof(float);
    ubo_buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    app_state->view_param_buf = app_state->device.CreateBuffer(&ubo_buffer_desc);

    wgpu::BindGroupEntry view_param_bg_entry = {};
    view_param_bg_entry.binding = 0;
    view_param_bg_entry.buffer = app_state->view_param_buf;
    view_param_bg_entry.size = ubo_buffer_desc.size;

    wgpu::BindGroupDescriptor bind_group_desc = {};
    bind_group_desc.layout = view_params_bg_layout;
    bind_group_desc.entryCount = 1;
    bind_group_desc.entries = &view_param_bg_entry;

    app_state->bind_group = app_state->device.CreateBindGroup(&bind_group_desc);
*/
	AppState* appState = (AppState*) malloc(sizeof(AppState));
	appState->device = device;
	appState->pipeline = renderPipeline;
	appState->queue = queue;
	appState->swapChain = swapChain;

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
/*
    wgpu::Buffer upload_buf;
    if (app_state->camera_changed) {
        wgpu::BufferDescriptor upload_buffer_desc;
        upload_buffer_desc.mappedAtCreation = true;
        upload_buffer_desc.size = 16 * sizeof(float);
        upload_buffer_desc.usage = wgpu::BufferUsage::CopySrc;
        upload_buf = app_state->device.CreateBuffer(&upload_buffer_desc);

        const glm::mat4 proj_view = app_state->proj * app_state->camera.transform();

        std::memcpy(
            upload_buf.GetMappedRange(), glm::value_ptr(proj_view), 16 * sizeof(float));
        upload_buf.Unmap();
    }

    wgpu::RenderPassColorAttachment color_attachment;
    color_attachment.view = app_state->swap_chain.GetCurrentTextureView();
    color_attachment.clearColor.r = 0.f;
    color_attachment.clearColor.g = 0.f;
    color_attachment.clearColor.b = 0.f;
    color_attachment.clearColor.a = 1.f;
    color_attachment.loadOp = wgpu::LoadOp::Clear;
    color_attachment.storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassDescriptor pass_desc;
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &color_attachment;

    wgpu::CommandEncoder encoder = app_state->device.CreateCommandEncoder();
    if (app_state->camera_changed) {
        encoder.CopyBufferToBuffer(
            upload_buf, 0, app_state->view_param_buf, 0, 16 * sizeof(float));
    }

    wgpu::RenderPassEncoder render_pass_enc = encoder.BeginRenderPass(&pass_desc);
    render_pass_enc.SetPipeline(app_state->render_pipeline);
    render_pass_enc.SetVertexBuffer(0, app_state->vertex_buf);
    render_pass_enc.SetBindGroup(0, app_state->bind_group);
    render_pass_enc.Draw(3);
    render_pass_enc.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    // Here the # refers to the number of command buffers being submitted
    app_state->queue.Submit(1, &commands);
	*/

	WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(appState->swapChain);
	if (nextTexture == nullptr)
	{
		utils::logBreak("wgpu next texture view could not be retrieved!");
	}

	WGPUCommandEncoderDescriptor commandEncoderDescriptor = { 0 };
	commandEncoderDescriptor.label = "Command Encoder";
	WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(appState->device, &commandEncoderDescriptor);
	if (commandEncoder == nullptr)
	{
		utils::logBreak("wgpu command encoder could not be created!");
	}

	WGPURenderPassEncoder renderPass;

	WGPURenderPassDescriptor renderPassDescriptor = { 0 };
	renderPassDescriptor.colorAttachmentCount = 1;
	renderPassDescriptor.depthStencilAttachment = nullptr;

	WGPURenderPassColorAttachment renderPassColorAttachment = { 0 };
	renderPassColorAttachment.view = nextTexture;
	renderPassColorAttachment.resolveTarget = 0;
	renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
	renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
#ifndef EMSCRIPTEN
	renderPassColorAttachment.clearValue = { 0.0, 0.0, 0.0, 1.0 };
#else
    renderPassColorAttachment.clearColor = { 0.0, 0.0, 0.0, 1.0 };
#endif
    
    renderPassDescriptor.colorAttachments = &renderPassColorAttachment;
    renderPassDescriptor.depthStencilAttachment = nullptr;

	renderPass = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptor);
	if (renderPass == nullptr)
	{
		utils::logBreak("wgpu render pass encoder could not be created!");
	}

	wgpuRenderPassEncoderSetPipeline(renderPass, appState->pipeline);
	wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
#ifndef EMSCRIPTEN
    wgpuRenderPassEncoderEnd(renderPass);
#else
	wgpuRenderPassEncoderEndPass(renderPass);
#endif

    WGPUCommandBufferDescriptor commandBufferDescriptor = {};
	WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, &commandBufferDescriptor);
	if (commandBuffer == nullptr)
	{
		utils::logBreak("wgpu command buffer could not be retreived!");
	}
	wgpuQueueSubmit(appState->queue, 1, &commandBuffer);

#ifndef EMSCRIPTEN
	wgpuSwapChainPresent(appState->swapChain);
#endif
}
