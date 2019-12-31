#pragma once

constexpr size_t _window_width = 1600;
constexpr size_t _window_height = 1000;

#include "WindowLoopManager.h"
inline WindowLoopManager g_wndlm;
#define WNDLM (g_wndlm)

#include "WindowsRenderDevice.h"
inline WindowsRenderDevice g_wndRendev;
#define WNDRD (g_wndRendev)

#include "Console.h"
#include <iostream>

