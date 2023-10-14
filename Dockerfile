FROM ubuntu:23.04

RUN apt update && apt upgrade
RUN apt install git clang cmake ninja-build libx11-dev libxrandr libxrandr-dev libxinerama-dev \
    libxcursor-dev libxi-dev libgl1-mesa-dev libgl1-mesa mesa-utils libx11-xcb-dev

COPY . /app
WORKDIR /app
USER app

RUN git submodule init && git submodule update --recursive

RUN mkdir -p native-build-debug && \
    cmake -G ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CPP_COMPILER=/usr/bin/clang++ -S . -B ./native-build-debug && \
    cmake --build ./native-build-debug --target webgpu -j 6

RUN ./native-build-debug/webgpu
