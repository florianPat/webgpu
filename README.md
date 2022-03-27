# WebGPU

Use Vulkan/D3D12 like API for the web with emscripten and the webgpu bindings.

## Building

You can build the project using Emscripten's CMake wrapper.
1. First, install cmake and emscripten and gcc/clang
2. Then, run the following or build with the vscode "build emscripten" task:
```
mkdir emcmake-build && cd emcmake-build
emcmake cmake ..
cmake --build .
```
3. After the build, you can then run a webserver in the build directory and view the application in
a browser that supports WebGPU (e.g. Chrome Canary with enabled feature flag):
```
php -S localhost:8000
# navigate to localhost:8000
```

You can also build the project for native (linux only for now, windows soon).
1. First, install cmake and gcc/clang and [build dawn](https://dawn.googlesource.com/dawn/+/HEAD/docs/dawn/building.md)
2. Then run the following or run the "build native debug/release" vscode task:
```
mkdir -p native-build && cd native-build 
cmake .. -DDawn_DIR=<path to dawn build> && cmake --build .
```
3. Then run the app: ```./native-build/webgpu``` or launch with the vscode task
