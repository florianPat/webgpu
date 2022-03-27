#include "Utils.h"
#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu_cpp.h>
#else
#include <dawn/webgpu.h>
#include "X11.h"
#endif

const char* WGSL_SHADER = R"(
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

struct AppState {
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

int main(int argc, const char **argv)
{
	WGPUInstance instance;

	WGPUInstanceDescriptor instanceDescriptor = { 0 };
	instanceDescriptor.nextInChain = nullptr;
	instance = wgpuCreateInstance(&instanceDescriptor);
	if (instance == nullptr)
	{
		utils::logBreak("wgpu instance could not be created!");
		return 0;
	}

	WGPUSurface surface;
#ifdef __EMSCRIPTEN__
	WGPUSurfaceDescriptorFromCanvasHTMLSelector surfaceDescritporCanvas = { 0 };
	surfaceDescritporCanvas.chain.next = nullptr;
	surfaceDescritporCanvas.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
	surfaceDescritporCanvas.selector = "#webgpu-canvas";

	WGPUSurfaceDescriptor surfaceDescriptor = { 0 };
	surfaceDescriptor.nextInChain = (const WGPUChainedStruct*)&surfaceDescritporXlib;
	surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
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

	WGPUSurfaceDescriptorFromXlibWindow surfaceDescritporXlib = { 0 };
	surfaceDescritporXlib.chain.next = nullptr;
	surfaceDescritporXlib.chain.sType = WGPUSType_SurfaceDescriptorFromXlibWindow;
	surfaceDescritporXlib.display = display;
	surfaceDescritporXlib.window = windowHandle;

	WGPUSurfaceDescriptor surfaceDescriptor = { 0 };
	surfaceDescriptor.nextInChain = (const WGPUChainedStruct*)&surfaceDescritporXlib;
	surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
#endif
	if (surface == nullptr)
	{
		utils::logBreak("wgpu surface could not be created!");
	}

	WGPUAdapter adapter;

	WGPURequestAdapterOptions requestAdapterOptions = { 0 };
	requestAdapterOptions.nextInChain = nullptr;
	requestAdapterOptions.compatibleSurface = surface;
	requestAdapterOptions.forceFallbackAdapter = false;
	requestAdapterOptions.powerPreference = WGPUPowerPreference_Undefined;
	wgpuInstanceRequestAdapter(instance, &requestAdapterOptions, requestAdapterCallback, (void*) &adapter);
	if (adapter == nullptr)
	{
		utils::logBreak("wgpu adapter could not be created!");
	}

	WGPUDevice device;
	
	WGPUDeviceDescriptor deviceDescriptor = { 0 };	
	deviceDescriptor.nextInChain = nullptr;

	WGPURequiredLimits requiredLimits = { 0 };
	requiredLimits.nextInChain = nullptr;
	requiredLimits.limits.maxBindGroups = 1;
	deviceDescriptor.requiredLimits = &requiredLimits;
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

	WGPUShaderModuleDescriptor shaderSource = { 0 };
	shaderSource.label = "Shader Source";
	WGPUShaderModuleWGSLDescriptor wgslDescriptor = { 0 };
	wgslDescriptor.chain.next = nullptr;
	wgslDescriptor.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
	wgslDescriptor.source = WGSL_SHADER;
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
	colorTargetState.format = WGPUTextureFormat_BGRA8Unorm;
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

	WGPUSwapChain swapChain;

	WGPUSwapChainDescriptor swapChainDescriptor = { 0 };
	swapChainDescriptor.usage = WGPUTextureUsage_RenderAttachment;
	swapChainDescriptor.format = WGPUTextureFormat_BGRA8Unorm;
	swapChainDescriptor.width = winWidth;
	swapChainDescriptor.height = winHeight;
	swapChainDescriptor.presentMode = WGPUPresentMode_Fifo;

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

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(loop_iteration, appState, -1, 0);
#else
    while (1) {
        loop_iteration(appState);
    }
#endif
    return 0;
}

#ifdef __EMSCRIPTEN__
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
#ifndef __EMSCRIPTEN__
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
	renderPassColorAttachment.clearColor = { 0.0, 0.0, 0.0, 1.0 };
	renderPassColorAttachment.clearValue = { 0.0, 0.0, 0.0, 1.0 };

	renderPass = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptor);
	if (renderPass == nullptr)
	{
		utils::logBreak("wgpu render pass encoder could not be created!");
	}

	wgpuRenderPassEncoderSetPipeline(renderPass, appState->pipeline);
	wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
	wgpuRenderPassEncoderEnd(renderPass);

	WGPUCommandBufferDescriptor commandBufferDescriptor = { 0 };
	commandEncoderDescriptor.label = nullptr;
	WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, &commandBufferDescriptor);
	if (commandBuffer == nullptr)
	{
		utils::logBreak("wgpu command buffer could not be retreived!");
	}
	wgpuQueueSubmit(appState->queue, 1, &commandBuffer);

#ifndef __EMSCRIPTEN__
	wgpuSwapChainPresent(appState->swapChain);
#endif
}
