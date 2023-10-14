#pragma once

#include "Types.h"
//#include "Delegate.h"
//#define WINDOWS_LEAN_AND_MEAN
//#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#undef assert

#define CONS

#ifdef DEBUG
#ifdef CONS
// TODO: Add platform agnostic message box!
#define assert(exp) if(!((exp) && true)) do { printf("ASSERT: %s\n", #exp); \
		abort(); } while(false)
#else
#define assert(exp) if(!((exp) && true)) do { OutputDebugStringA("ASSERT: "); OutputDebugStringA(#exp); OutputDebugStringA("\n"); \
											  DebugBreak(); } while(false)
#endif
#else
#define assert(exp)
#endif

#define InvalidCodePath assert(!"InvalidCodePath")

#define arrayCount(x) sizeof(x) / sizeof(x[0])

#define Kilobyte(x) x * 1024ll
#define Megabyte(x) Kilobyte(x) * 1024ll
#define Gigabyte(x) Megabyte(x) * 1024ll

static constexpr float PIf = 3.1415927f;

static constexpr float PiOver180 = PIf / 180;
static constexpr float _180OverPi = 180 / PIf;

/*#define GET_DELEGATE_FROM(c, m) Delegate<void(EventData*)>::from<c, m>(this)
#define GET_DELEGATE_WITH_PARAM_FORM(param, c, m) Delegate<param>::from<c, m>(this)
#define GET_COMPONENT(actor, componentType) actor->getComponent<componentType>(componentType::ID)*/

namespace utils
{
	uint32_t getGUID();
	float lerp(float v0, float v1, float t);
	float degreesToRadians(float degree);
	float radiansToDegrees(float radians);
	inline void log(const char* string)
	{
#ifdef DEBUG
#ifdef CONS
		printf("%s\n", string);
#else
		OutputDebugStringA(string);
		OutputDebugStringA("\n");
#endif
#else
		printf("%s\n", string);
#endif
	}
	inline void logBreak(const char* string)
	{
		log(string);
		InvalidCodePath;
	}
	template<typename... Args>
	void logF(const char* string, Args&&... args)
    {
		char buffer[256];
		snprintf((char* const)&buffer, 256, string, args...);
		log(buffer);
    }
    template<typename... Args>
	void logFBreak(const char* string, Args&&... args)
    {
		char buffer[256];
		snprintf((char* const)&buffer, 256, string, args...);
		logBreak(buffer);
    }
	constexpr uint32_t getIntFromChars(char first, char second)
	{
		return ((uint32_t)first) | ((uint32_t)second << 8);
	}
}