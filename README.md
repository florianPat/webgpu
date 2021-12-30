# WebGPU

Use Vulkan/D3D12 like API for the web with Emscripten and the WebGpu bindings.

## Building

You can build the project using Emscripten's CMake wrapper.
1. First, install cmake and emscripten
2. Then, run:
```
mkdir emcmake-build
emcmake cmake ..
cmake --build .
```
3. After the build, you can then run a webserver in the build directory and view the application in
a browser that supports WebGPU (e.g. Chrome Canary with enabled feature flag):
```
php -S localhost:8000
# navigate to localhost:8000
```
