#pragma once

#include "String.h"

void clearErrors();
void checkErrors(const String& func, int32_t line, const char* inFunc);

#ifdef _DEBUG
#define CallGL(x) do { clearErrors(); x; checkErrors(#x, __LINE__, __FILE__); } while(false)
#else
#define CallGL(x) x
#endif
