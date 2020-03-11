#pragma once

constexpr size_t _window_width = 1600;
constexpr size_t _window_height = 1000;

#include "WindowLoopManager.h"
inline WindowLoopManager g_wndlm;
#define WNDLM (g_wndlm)

#include "WindowsRenderDevice.h"
inline WindowsRenderDevice g_wndRendev;
#define WNDRD (g_wndRendev)

#include "InputDevice/InputDevice.h"
#if _DEBUG
#pragma comment (lib, "InputDevice/Debug/x64/InputDevice.lib")
#else
#pragma comment (lib, "InputDevice/Release/x64/InputDevice.lib")
#endif
inline InputDevice g_inputDevice;

#include "SoundDevice.h"
inline SoundDevice g_soundDevice;

#include "Console.h"
#include <iostream>

