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
inline DC_Window g_dc_window;
inline DC_RenderTarget g_dc_backbuffer;
inline DC_RenderTarget g_dc_rendertarget1;
inline DC_RenderTarget g_dc_rendertarget2;
