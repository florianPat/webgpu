#include "WindowsWindow.h"
#include "Utils.h"
#include "Types.h"
#include "Utils.h"
#include "Ifstream.h"
#include "Font.h"
#include "Globals.h"
#include "EventDestroyApp.h"
#include "EventStopApp.h"
#include "EventResumeApp.h"
#include "EventManager.h"
#include "FbxModel.h"

LRESULT CALLBACK WindowsWindow::windowProcInit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		// extract ptr to window class from creation data
		const CREATESTRUCTW* const create = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowsWindow* wnd = reinterpret_cast<WindowsWindow*>(create->lpCreateParams);
		// sanity check
		assert(wnd != nullptr);
		// set WinAPI-managed user data to store ptr to window class
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);

		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)&WindowsWindow::windowProc);

		return true;
	}
	else
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowsWindow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WindowsWindow* const window = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	LRESULT result = 0;

	switch (message)
	{
		case WM_ACTIVATEAPP:
		{
			if (wParam)
			{
				//NOTE: Acitve
			}
			else
			{
				//NOTE: Inactive
			}
			break;
		}
		case WM_CLOSE:
		{
			assert(Globals::eventManager != nullptr);
			Globals::eventManager->TriggerEvent<EventStopApp>();
			window->nativeThreadQueue.flushFrom(window->threadQueueEventBeginning);

			window->nativeThreadQueue.endThreads();

			window->finishDeinit();
			window->deactivate();

			window->running = false;
			window->windowHandle = 0;
			DestroyWindow(hwnd);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		case WM_QUIT:
		{
			break;
		}
		case WM_SETCURSOR:
		{
			if (window->hasCursor)
				result = DefWindowProcA(hwnd, message, wParam, lParam);
			else
				SetCursor(0);
			break;
		}
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			char keyCode = (char)wParam;
			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (1 << 31)) == 0);

			if (wasDown != isDown)
			{
				window->keyboard.setKey(keyCode, isDown);
			}

			if (isDown)
			{
				bool altKeyIsDown = (lParam & (1 << 29)); //Get back 1 if Alt is down
				if ((keyCode == VK_F4) && altKeyIsDown)
				{
					window->running = false;
				}
				if (keyCode == VK_RETURN && altKeyIsDown)
				{
					if (hwnd)
						window->toggleFullscreen();
				}
			}
			break;
		}
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			POINTS p = MAKEPOINTS(lParam);

			if ((uint32_t)p.x <= window->gfx.screenHeight)
				window->mouse.pos.x = p.x;
			if ((uint32_t)p.y <= window->gfx.screenHeight)
				window->mouse.pos.y = p.y;

			window->mouse.setButton((uint32_t)wParam);
			break;
		}
		case WM_CHAR:
		{
			//TODO: Implement this, I need a queue!
			//keyboard.char((uint8_t)message.wParam);
			break;
		}
		default:
		{
			result = DefWindowProcA(hwnd, message, wParam, lParam);
			break;
		}
	}

	return result;
}

WindowsWindow::WindowsWindow(HINSTANCE hInstance, int32_t width, int32_t height) : IWindow(width, height)
{
    Globals::window = this;

    WNDCLASSA windowClass = { 0 };
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.hInstance = GetModuleHandle(0);
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.lpszClassName = "WindowWindowClass";
    windowClass.lpfnWndProc = &windowProcInit;

    if (RegisterClassA(&windowClass))
    {
        RECT windowRect = {};
        windowRect.left = 0;
        windowRect.bottom = 0;
        windowRect.right = width;
        windowRect.top = height;
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);
        windowHandle = CreateWindowA(windowClass.lpszClassName, "3d-Engine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.top - windowRect.bottom, 0, 0, GetModuleHandle(0), this);

        if (windowHandle)
        {
            clock = Clock();

            nativeThreadQueue.startThreads();

            if ((!gfx.startGfx(hInstance, windowHandle)) || (!audio.startSnd()) || (!startFont()))
            {
                    deactivate();
                    close();
                    return;
            }

            finishInit();
            return;
        }
        else
        {
            InvalidCodePath;
        }
    }
    else
    {
        InvalidCodePath;
    }

    processEvents();
}

bool WindowsWindow::processEvents()
{
    MSG message;

    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);

        if (message.message == WM_CLOSE)
        {
           return false;
        }
        else if (!running)
        {
           PostMessageA(windowHandle, WM_CLOSE, 0, 0);
        }
    }

    return true;
}

void WindowsWindow::close()
{
    PostQuitMessage(0);
}

void WindowsWindow::deactivate()
{
    //TODO: I should not have to do this!
    assetManager.clear();
    gfx.stopGfx();
    audio.stopSnd();
    stopFont();
}

const Keyboard& WindowsWindow::getKeyboard() const
{
	return keyboard;
}

const Mouse& WindowsWindow::getMouse() const
{
	return mouse;
}

Audio& WindowsWindow::getAudio()
{
    return audio;
}

void WindowsWindow::finishInit()
{
    if(Globals::eventManager != nullptr)
    {
        Globals::eventManager->TriggerEvent<EventResumeApp>();
        nativeThreadQueue.flushFrom(threadQueueEventBeginning);
    }

    audio.startStream();

    initFinished = true;
    clock.restart();
}

void WindowsWindow::finishDeinit()
{
    audio.stopStream();

    initFinished = false;
}

void WindowsWindow::setWindowText(const char* text) const
{
	SetWindowTextA(windowHandle, text);
}

void WindowsWindow::toggleFullscreen()
{
	DWORD dwStyle = GetWindowLong(windowHandle, GWL_STYLE);
	if (dwStyle & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(windowHandle, &previousWindowPos) &&
			GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY),
				&mi))
		{
			SetWindowLong(windowHandle, GWL_STYLE,
				dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(windowHandle, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(windowHandle, GWL_STYLE,
			dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(windowHandle, &previousWindowPos);
		SetWindowPos(windowHandle, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
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
	return (buttons & (uint32_t)mouseButton);
}

inline void Mouse::setButton(uint32_t wParam)
{
	buttons = 0;
	buttons |= wParam;
}
