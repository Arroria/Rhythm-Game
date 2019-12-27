#pragma once
#include "windefinclude.h"

#include <chrono>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if (p) { delete (p); (p) = nullptr; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = nullptr; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p); (p) = nullptr; } }
#endif

#include <string_view>
class WindowLoopManager
{
public:
	using Clock_t = std::chrono::steady_clock;
	using ClockTime_t = Clock_t::duration;
	using ClockTimePoint_t = Clock_t::time_point;

	WindowLoopManager();
	~WindowLoopManager();



	void Initialize(const HINSTANCE& hInstance);
	HRESULT AllocWindow(const std::string_view& windowTitle, DWORD windowStyle, LRESULT(CALLBACK* wndProc)(HWND, UINT, WPARAM, LPARAM));
	void SetFunction(void(*initialize)(), void(*update)(), void(*render)(), void(*release)()) { m_initialize = initialize; m_update = update; m_render = render; m_release = release; }
	void SetFPSLimite(size_t updateFPS, size_t renderFPS) { m_updateFPSLimite = updateFPS; m_renderFPSLimite = renderFPS; }
	
	void WndMove(const int& xPos, const int& yPos);
	void WndResize(const unsigned int& width, const unsigned int& height);


	WPARAM Loop();


	HWND WindowHandle() const { return m_hWnd; }
	const ClockTime_t& DeltatimeByUpdate() const { return m_deltatime_update; }
	const ClockTime_t& DeltatimeByRender() const { return m_deltatime_render; }


private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	DWORD m_wndStyle;


	void(*m_initialize)();
	void(*m_update)();
	void(*m_render)();
	void(*m_release)();

	size_t m_updateFPSLimite;
	size_t m_renderFPSLimite;
	ClockTime_t m_deltatime_update;
	ClockTime_t m_deltatime_render;
};


