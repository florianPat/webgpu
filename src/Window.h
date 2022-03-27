#pragma once

#include "Clock.h"
#include "AssetManager.h"
#include "Sound.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "NativeThreadQueue.h"
#include "Audio.h"
#include "Graphics.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

class Keyboard
{
	bool keyCodes[255] = { 0 };
	uint8_t characterQueue[255] = { 0 };
	uint8_t characterQueueNextRead = 0;
	uint8_t characterQueueNextWrite = 0;
public:
	bool isKeyPressed(char keyCode) const;
	inline void setKey(char keyCode, bool isDown);
	inline void character(uint8_t character);
	bool hasNextCharacterInQueue() const;
	char getNextCharacterFromQueue();
};

class Mouse
{
	uint32_t buttons = 0;
	uint32_t releasedButton = 0;
public:
	Vector2i pos;
	Vector2i rawDelta;
public:
	enum class Button
	{
		left = MK_LBUTTON,
		middle = MK_MBUTTON,
		right = MK_RBUTTON
	};
	bool isButtonPressed(Button mouseButton) const;
	bool isButtonReleased(Button mouseButton) const;
	inline void setButton(uint32_t wParam);
	inline void unsetButton(uint32_t wParam);
	inline void setReleasedButton(uint32_t wParam);
	inline void clearReleasedButton();
};

class Window
{
	HWND windowHandle;
	bool gainedFocus = false;
	bool resumed = false;
	bool validNativeWindow = false;
	bool initFinished = false;
	bool running = true;
	bool recreating = false;
	bool hasCursor = true;
	bool shouldHideAndClipCursor = false;

    Clock clock;
    AssetManager assetManager;
    FT_Library fontLibrary = nullptr;
	Keyboard keyboard;
	Mouse mouse;

    GraphicsIniter gfx;

    NativeThreadQueue nativeThreadQueue;
    uint32_t threadQueueEventBeginning = 0;

    Audio audio;

	WINDOWPLACEMENT previousWindowPos = { sizeof(previousWindowPos) };
public:
	Window(HINSTANCE hInstance, int32_t width, int32_t height);
	Window(const Window& other) = delete;
	Window(Window&& other) = delete;
	Window& operator=(const Window& rhs) = delete;
	Window& operator=(Window&& rhs) = delete;
	bool processEvents();

	void close();
	AssetManager* getAssetManager();
	Clock& getClock();
	FT_Library getFontLibrary();
    GraphicsIniter& getGfx();
	Keyboard& getKeyboard();
	const Mouse& getMouse() const;
    NativeThreadQueue& getNativeThreadQueue();
    Audio& getAudio();
    void callToGetEventJobsBeginning();
	void setWindowText(const char* text) const;
	void hideAndClipCursor();
	void showAndUnclipCursor();
	String pasteTextFromClipboard() const;
private:
	void deactivate();
	static LRESULT CALLBACK windowProcInit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool startFont();
	void stopFont();
	void finishInit();
	void finishDeinit();
	void toggleFullscreen();
	bool enableRawInput() const;
	void resetRawInput();
	void hideCursor();
	void showCursor();
	void clipCursor();
	void unclipCursor();
};