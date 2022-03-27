#include "LinuxWindow.h"
#include "EventManager.h"
#include "EventResumeApp.h"
#include "EventStopApp.h"

LinuxWindow::LinuxWindow(int32_t width, int32_t height) : IWindow(width, height)
{
    Globals::window = this;

    display = XOpenDisplay(nullptr);
    if (display == nullptr)
    {
        utils::logBreak("Could not open display");
    }
    screen = DefaultScreenOfDisplay(display);
    int32_t screenId = DefaultScreen(display);
    windowHandle = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, width, height, 1,
                                              BlackPixel(display, screenId), WhitePixel(display, screenId));

    XSelectInput(display, windowHandle, Keyboard::EVENT_MASKS | Mouse::EVENT_MASKS | ExposureMask);

    XStoreName(display, windowHandle, "3D Engine");

    XClearWindow(display, windowHandle);
    XMapRaised(display, windowHandle);

    clock = Clock();

    nativeThreadQueue.startThreads();

    if ((!gfx.startGfx(&windowHandle)) || (!startFont()))
    {
        deactivate();
        close();
        return;
    }

    finishInit();
    return;
}

bool LinuxWindow::processEvents()
{
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

    return running;
}

void LinuxWindow::close()
{
    assert(Globals::eventManager != nullptr);
    Globals::eventManager->TriggerEvent<EventStopApp>();
    nativeThreadQueue.flushFrom(threadQueueEventBeginning);

    nativeThreadQueue.endThreads();

    finishDeinit();
    deactivate();

    running = false;
    XDestroyWindow(display, windowHandle);
    XFree(screen);
    XCloseDisplay(display);
}

const Keyboard &LinuxWindow::getKeyboard() const
{
    return keyboard;
}

const Mouse &LinuxWindow::getMouse() const
{
    return mouse;
}

void LinuxWindow::deactivate()
{
    assetManager.clear();
    gfx.stopGfx();
    stopFont();
}

void LinuxWindow::finishInit()
{
    if(Globals::eventManager != nullptr)
    {
        Globals::eventManager->TriggerEvent<EventResumeApp>();
        nativeThreadQueue.flushFrom(threadQueueEventBeginning);
    }

    //audio.startStream();

    initFinished = true;
    clock.restart();
}

void LinuxWindow::finishDeinit()
{
    //audio.stopStream();

    initFinished = false;
}

void LinuxWindow::toggleFullscreen()
{

}

bool Keyboard::isKeyPressed(char keyCode) const
{
    return keyCodes[keyCode];
}

inline void Keyboard::setKey(char keyCode, bool isDown)
{
    if (isDown)
    {
        keyCodes[keyCode] = true;
    }
    else
    {
        keyCodes[keyCode] = false;
    }
}

inline bool Mouse::isButtonPressed(Button mouseButton) const
{
    return buttons & (uint32_t)mouseButton;
}

inline void Mouse::setButton(uint32_t button)
{
    buttons |= button;
}

