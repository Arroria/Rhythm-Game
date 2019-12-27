#include "pch.h"

#include "WindowLoopManager.h"
WindowLoopManager g_wndlm;
#define WNDLM (g_wndlm)

#include "WindowsRenderDevice.h"
WindowsRenderDevice g_wndRendev;
#define WNDRD (g_wndRendev)


constexpr size_t _window_width = 1600;
constexpr size_t _window_height = 1000;


void Init();
void Update();
void Render();
void Release();

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WNDLM.Initialize(hInstance);
	WNDLM.AllocWindow("Editor", WS_BORDER | WS_DLGFRAME | WS_MINIMIZEBOX | WS_SYSMENU, WndProc);
	WNDLM.SetFunction(Init, Update, Render, Release);
	WNDLM.SetFPSLimite(0, 60);

	WNDLM.WndMove(10, 10);
	WNDLM.WndResize(_window_width, _window_height);

	ShowWindow(WNDLM.WindowHandle(), SW_RESTORE);

	return (int)WNDLM.Loop();
}


void Init()
{
	WNDRD.Initialize(WNDLM.WindowHandle(), _window_width, _window_height);
}
void Update()
{
}
void Render()
{
	WNDRD.DrawOnMainBuffer();
	WNDRD.SetBrushColor(255, 255, 255);
	WNDRD.Fill(0, 0, _window_width, _window_height);
	
	WNDRD.SingleLine(0, 0, _window_width, _window_height);

	WNDRD.Clipping();
}
void Release()
{
	WNDRD.Release();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
