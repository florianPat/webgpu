#pragma once

#define Window WindowHandle
#define Time LinuxTime
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#undef Window
#undef Time
