#pragma once
#include "WindowLoopManager.h"
inline WindowLoopManager g_wndlm = WindowLoopManager();
#define WNDLM (g_wndlm)



#include "InputDevice/InputDevice.h"
#if _DEBUG
#pragma comment (lib, "InputDevice/Debug/x64/InputDevice.lib")
#else
#pragma comment (lib, "InputDevice/Release/x64/InputDevice.lib")
#endif
inline InputDevice g_inputDevice;



#include "SoundDevice.h"
inline SoundDevice g_soundDevice;



#include "mywinapi.h"

