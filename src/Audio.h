#pragma once

#include <dsound.h>
#include "Sound.h"

class Audio
{
    Array<Sound*, 32> sounds;
    Vector<Iterator<Sound*>> deleteList;

    LPDIRECTSOUNDBUFFER secondaryBuffer = nullptr;
    uint32_t bytesPerSample = sizeof(int16_t) * 2;
    uint32_t samplesPerSecound;
    uint32_t sampleIndex = 0;
    uint32_t bufferSize;
public:
    Audio(uint32_t samplesPerSecond = 48000, uint32_t bufferSize = 48000 * sizeof(int16_t) * 2);
    Audio(const Audio& other) = delete;
    Audio& operator=(const Audio& rhs) = delete;
    Audio(Audio&& other) = delete;
    Audio& operator=(Audio&& rhs) = delete;
    bool startSnd(HWND windowHandle);
    void stopSnd();
    void startStream();
    void stopStream();
    //TODO: Think about doing this thread safe!
    void start(Sound* sound);
    void stop(Sound* sound);
    void clear();
    // TOOD: Threading!!
    void update();
};
