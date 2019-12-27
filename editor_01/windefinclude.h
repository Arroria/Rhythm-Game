#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

constexpr bool _ISDEBUG() { return
#if _DEBUG
true
#else
false
#endif
;}
