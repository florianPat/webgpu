#pragma once

#include "IWindow.h"
#include "X11.h"

class Keyboard
{
    bool keyCodes[255] = { 0 };
public:
    static constexpr uint32_t EVENT_MASKS = KeyPressMask | KeyReleaseMask | KeymapStateMask;
public:
    bool isKeyPressed(char keyCode) const;
    inline void setKey(char keyCode, bool isDown);
};

class Mouse
{
    uint32_t buttons = 0;
public:
    Vector2i pos;
public:
    static constexpr uint32_t EVENT_MASKS = PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask;
public:
    enum class Button
    {
        left = 1,
        middle = 2,
        right = 4
    };
    inline bool isButtonPressed(Button mouseButton) const;
    inline void setButton(uint32_t button);
};

class LinuxWindow : public IWindow
{
    Display* display = nullptr;
    Screen* screen = nullptr;
    WindowHandle windowHandle = 0;

    Keyboard keyboard;
    Mouse mouse;
public:
    LinuxWindow(int32_t width, int32_t height);
    LinuxWindow(const LinuxWindow& other) = delete;
    LinuxWindow& operator=(const LinuxWindow& rhs) = delete;
    LinuxWindow(LinuxWindow&& rhs) = delete;
    LinuxWindow& operator=(LinuxWindow&& rhs) = delete;
    bool processEvents();

    void close();
    const Keyboard& getKeyboard() const;
    const Mouse& getMouse() const;
private:
    void deactivate();
    void finishInit();
    void finishDeinit();
    void toggleFullscreen();
};
