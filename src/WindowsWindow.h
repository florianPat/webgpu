#pragma once

#include "Sound.h"
#include "Audio.h"
#include "Vector2.h"
#include "IWindow.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

class Keyboard
{
	bool keyCodes[255] = { 0 };
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
	enum class Button
	{
		left = MK_LBUTTON,
		middle = MK_MBUTTON,
		right = MK_RBUTTON
	};
	inline bool isButtonPressed(Button mouseButton) const;
	inline void setButton(uint32_t wParam);
};

class WindowsWindow : public IWindow
{
    HWND windowHandle;
	bool hasCursor = true;

	Keyboard keyboard;
	Mouse mouse;

    Audio audio;

	WINDOWPLACEMENT previousWindowPos = { sizeof(previousWindowPos) };
public:
    WindowsWindow(HINSTANCE hInstance, int32_t width, int32_t height);
    WindowsWindow(const WindowsWindow& other) = delete;
    WindowsWindow(WindowsWindow&& other) = delete;
    WindowsWindow& operator=(const WindowsWindow& rhs) = delete;
    WindowsWindow& operator=(WindowsWindow&& rhs) = delete;
	bool processEvents();

	void close();
	const Keyboard& getKeyboard() const;
	const Mouse& getMouse() const;
    Audio& getAudio();
	void setWindowText(const char* text) const;
private:
	void deactivate();
	static LRESULT CALLBACK windowProcInit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void finishInit();
	void finishDeinit();
	void toggleFullscreen();
};
