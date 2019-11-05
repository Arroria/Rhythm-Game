#pragma once
#include <Windows.h>
#include <string_view>

#include "SoundDevice.h"
inline SoundDevice g_soundDevice;


#include "InputDevice/InputDevice.h"
#if _DEBUG
#pragma comment (lib, "InputDevice/Debug/x64/InputDevice.lib")
#else
#pragma comment (lib, "InputDevice/Release/x64/InputDevice.lib")
#endif
inline InputDevice g_inputDevice;


#include <chrono>
using Clock_t = std::chrono::steady_clock;
using ClockTime_t = Clock_t::duration;
using Time_t = std::chrono::nanoseconds;
using TimePoint_t = Clock_t::time_point;

#include <iostream>
inline void gotoxy(int x, int y) { SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ (short)x, (short)y }); }
