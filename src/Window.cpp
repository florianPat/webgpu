#include "Window.h"
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

LRESULT CALLBACK Window::windowProcInit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		// extract ptr to window class from creation data
		const CREATESTRUCTW* const create = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* wnd = reinterpret_cast<Window*>(create->lpCreateParams);
		// sanity check
		assert(wnd != nullptr);
		// set WinAPI-managed user data to store ptr to window class
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);

		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)&Window::windowProc);

		return true;
	}
	else
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* const window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	LRESULT result = 0;

	switch (message)
	{
		case WM_ACTIVATEAPP:
		case WM_ACTIVATE:
		{
			if (wParam)
			{
				//NOTE: Active

				if (window->shouldHideAndClipCursor)
				{
					window->hideCursor();
					window->clipCursor();
				}
				else
				{
					window->unclipCursor();
					window->showCursor();
				}
			}
			else
			{
				//NOTE: Inactive

				if (window->shouldHideAndClipCursor)
				{
					window->unclipCursor();
					window->showCursor();
				}
			}
			break;
		}
		case WM_MOUSEACTIVATE:
		{
			window->hideCursor();
			window->clipCursor();
			break;
		}
		// NOTE: SetFocus und KillFocus to recives / lose keyboard focus (display or hide caret)
		case WM_SETFOCUS:
		{
			break;
		}
		case WM_KILLFOCUS:
		{
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
		{
			POINTS p = MAKEPOINTS(lParam);

			if ((uint32_t)p.x <= window->gfx.screenHeight)
				window->mouse.pos.x = p.x;
			if ((uint32_t)p.y <= window->gfx.screenHeight)
				window->mouse.pos.y = p.y;
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			window->mouse.setButton((uint32_t)wParam);
			break;
		}
		case WM_LBUTTONUP:
		{
			window->mouse.unsetButton((uint32_t)Mouse::Button::left);
			window->mouse.setReleasedButton((uint32_t)Mouse::Button::left);
			break;
		}
		case WM_MBUTTONUP:
		{
			window->mouse.unsetButton((uint32_t)Mouse::Button::middle);
			window->mouse.setReleasedButton((uint32_t)Mouse::Button::middle);
			break;
		}
		case WM_RBUTTONUP:
		{
			window->mouse.unsetButton((uint32_t)Mouse::Button::right);
			window->mouse.setReleasedButton((uint32_t)Mouse::Button::right);
			break;
		}
		case WM_CHAR:
		{
			window->keyboard.character((uint8_t)wParam);
			break;
		}
		case WM_INPUT:
		{
			uint32_t size = 0;
			uint32_t result = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
			if (result == (uint32_t)-1)
			{
				utils::log("Raw input error");
			}
			uint8_t* data = (uint8_t*)_malloca(size);
			result = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));
			assert(result == size);
			const RAWINPUT& rawInput = (const RAWINPUT&)*data;
			if (rawInput.header.dwType == RIM_TYPEMOUSE && (rawInput.data.mouse.lLastX != 0 || rawInput.data.mouse.lLastY != 0))
			{
				// TODO: Add this to a message queue
				uint32_t x = (uint32_t)rawInput.data.mouse.lLastX;
				uint32_t y = (uint32_t)rawInput.data.mouse.lLastY;
				window->mouse.rawDelta.x = x;
				window->mouse.rawDelta.y = y;
			}
			_freea(data);
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

Window::Window(HINSTANCE hInstance, int32_t width, int32_t height) : clock(),
                                                                     assetManager(),
                                                                     gfx(width, height)
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
		windowRect.top = 0;
		windowRect.right = width;
		windowRect.bottom = height;
		uint32_t windowStyle = WS_OVERLAPPED | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;
		AdjustWindowRect(&windowRect, windowStyle, false);
		windowHandle = CreateWindowA(windowClass.lpszClassName, "3d-Engine", windowStyle | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0, 0, GetModuleHandle(0), this);

		if (windowHandle)
		{
			clock = Clock();

			nativeThreadQueue.startThreads();

			if ((!gfx.startGfx(hInstance, windowHandle)) || (!audio.startSnd(windowHandle)) || (!startFont()) || (!enableRawInput()))
			{
				utils::logBreak("initialization of gfx or audio or font failed!");
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

bool Window::processEvents()
{
	// TODO: Move this into a queue?
	resetRawInput();
	
	mouse.clearReleasedButton();

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

void Window::close()
{
	PostQuitMessage(0);
}

AssetManager * Window::getAssetManager()
{
    return &assetManager;
}

Clock & Window::getClock()
{
    return clock;
}

void Window::deactivate()
{
	//TODO: I should not have to do this!
	assetManager.clear();
    gfx.stopGfx();
    audio.stopSnd();
    stopFont();
}

bool Window::startFont()
{
    int32_t error = FT_Init_FreeType(&fontLibrary);
    if(error)
        utils::log("could not init freetype library");

    return (error == 0);
}

void Window::stopFont()
{
    FT_Done_FreeType(fontLibrary);
    fontLibrary = nullptr;
}

FT_Library Window::getFontLibrary()
{
    return fontLibrary;
}

GraphicsIniter& Window::getGfx()
{
    return gfx;
}

Keyboard& Window::getKeyboard()
{
	return keyboard;
}

const Mouse& Window::getMouse() const
{
	return mouse;
}

NativeThreadQueue& Window::getNativeThreadQueue()
{
    return nativeThreadQueue;
}

Audio& Window::getAudio()
{
    return audio;
}

void Window::finishInit()
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

void Window::finishDeinit()
{
    audio.stopStream();

    initFinished = false;
}

void Window::callToGetEventJobsBeginning()
{
    threadQueueEventBeginning = nativeThreadQueue.getSize();
}

void Window::setWindowText(const char* text) const
{
	SetWindowTextA(windowHandle, text);
}

void Window::hideAndClipCursor()
{
	shouldHideAndClipCursor = true;
	hideCursor();
	clipCursor();
}

void Window::showAndUnclipCursor()
{
	shouldHideAndClipCursor = false;
	unclipCursor();
	showCursor();
}

String Window::pasteTextFromClipboard() const
{
	ShortString result;

	if (!IsClipboardFormatAvailable(CF_TEXT))
	{
		utils::logBreak("Clipboard format cf_text not available!");
	}
	if (!OpenClipboard(windowHandle))
	{
		utils::logBreak("Could not open clipboard!");
	}
	HGLOBAL hgbl = GetClipboardData(CF_TEXT);
	if (hgbl != 0)
	{
		LPSTR string = (LPSTR) GlobalLock(hgbl);
		if (string != 0)
		{
			new (&result) String(std::move(String::createIneffectivlyFrom(string)));
		}
		GlobalUnlock(hgbl);
	}
	CloseClipboard();

	return result;
}

void Window::hideCursor()
{
	while (ShowCursor(false) >= 0);
}

void Window::showCursor()
{
	while (ShowCursor(true) < 0);
}

void Window::clipCursor()
{
	RECT rect;
	GetClientRect(windowHandle, &rect);
	MapWindowPoints(windowHandle, nullptr, (POINT*)&rect, 2);
	ClipCursor(&rect);
}

void Window::unclipCursor()
{
	ClipCursor(nullptr);
}

void Window::toggleFullscreen()
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

bool Window::enableRawInput() const
{
	RAWINPUTDEVICE rawInputDevice;
	rawInputDevice.usUsagePage = 0x01; // mouse page
	rawInputDevice.usUsage = 0x02; // mouse usage
	rawInputDevice.dwFlags = 0;
	rawInputDevice.hwndTarget = windowHandle;
	return RegisterRawInputDevices(&rawInputDevice, 1, sizeof(rawInputDevice));
}

void Window::resetRawInput()
{
	mouse.rawDelta = { 0, 0 };
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

inline void Keyboard::character(uint8_t character)
{
	characterQueue[characterQueueNextWrite++] = character;
}

bool Keyboard::hasNextCharacterInQueue() const
{
	return characterQueueNextRead != characterQueueNextWrite;
}

char Keyboard::getNextCharacterFromQueue()
{
	assert(hasNextCharacterInQueue());
	return characterQueue[characterQueueNextRead++];
}

bool Mouse::isButtonPressed(Button mouseButton) const
{
	return (buttons & (uint32_t)mouseButton);
}

bool Mouse::isButtonReleased(Button mouseButton) const
{
	return (releasedButton & (uint32_t)mouseButton);
}

inline void Mouse::setButton(uint32_t wParam)
{
	buttons |= wParam;
}

inline void Mouse::unsetButton(uint32_t wParam)
{
	buttons &= ~wParam;
}

inline void Mouse::setReleasedButton(uint32_t wParam)
{
	releasedButton |= wParam;
}

inline void Mouse::clearReleasedButton()
{
	releasedButton = 0;
}
