#include "pch.h"


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
	WNDLM.WndResize(1600, 1000);

	ShowWindow(WNDLM.WindowHandle(), SW_RESTORE);

    return (int)WNDLM.Loop();
}





MyRenderTarget editScreenDCRenderTarget;

#include "SheetEditor_IO.h"
SheetEditor_IO editorIO;

void Init()
{
	// Sound Device
	g_soundDevice.Initialize();

	// Main DC
	constexpr size_t _backgroundWidth = 1600;
	constexpr size_t _backgroundHeight = 1000;
	constexpr BYTE _backgroundBrightness = 127;
	g_dc_window.Initialize(WNDLM.WindowHandle());

	MyRenderTarget forBackbuffer;
	forBackbuffer.Allocation(g_dc_window, _backgroundWidth, _backgroundHeight);
	g_dc_backbuffer.Initialize(g_dc_window);
	g_dc_backbuffer.SetRenderTarget(std::move(forBackbuffer));
	g_dc_backbuffer.SetPenColor(_backgroundBrightness, _backgroundBrightness, _backgroundBrightness);
	g_dc_backbuffer.SetBrushColor(_backgroundBrightness, _backgroundBrightness, _backgroundBrightness);
	g_dc_backbuffer.DrawRectByRect(0, 0, _backgroundWidth, _backgroundHeight);

	g_dc_rendertarget1.Initialize(g_dc_window);
	g_dc_rendertarget2.Initialize(g_dc_window);
	

	// Edit
	editScreenDCRenderTarget.Allocation(g_dc_window, 400, 1000);


	editorIO.Initialize(100, 48);
}

#include <string>
void Update()
{
	g_soundDevice.Update();

	HWND handle = WNDLM.WindowHandle();
	g_inputDevice.BeginFrame(handle);

	//if (g_inputDevice.IsKeyDown('1'))	{ backDC.SetPenColor(0, 0, 0); backDC.SetBrushColor(255, 255, 255); }
	//if (g_inputDevice.IsKeyDown('2'))	{ backDC.SetPenColor(255, 0, 0); backDC.SetBrushColor(0, 255, 0); }
	//if (g_inputDevice.IsKeyDown('3'))	{ backDC.SetBrushTransparent(false); }
	//if (g_inputDevice.IsKeyDown('4'))	{ backDC.SetBrushTransparent(true); }
	//
	//if (g_inputDevice.IsKeyDown('A'))	pos.x -= 5;
	//if (g_inputDevice.IsKeyDown('D'))	pos.x += 5;
	//if (g_inputDevice.IsKeyDown('W'))	pos.y -= 5;
	//if (g_inputDevice.IsKeyDown('S'))	pos.y += 5;
	//
	//
	//if (g_inputDevice.IsKeyDown(VK_RBUTTON))	backDC.LinePoint(g_inputDevice.MousePos().x, g_inputDevice.MousePos().y);
	//if (g_inputDevice.IsKeyDown(VK_LBUTTON))	backDC.LineLink(g_inputDevice.MousePos().x, g_inputDevice.MousePos().y);
	//
	//
	//if (g_inputDevice.IsKeyDown(VK_RETURN)) { MyDC_FillRect(backDC, 0, 0, 1600, 1000, (HBRUSH)GetStockObject(WHITE_BRUSH)); }
	editorIO.Update();


	g_inputDevice.EndFrame();
}

void Render()
{
	// edit screen
	{
		//g_dc_rendertarget1.SetRenderTarget(editScreenDCRenderTarget);
		//DC_FillRect(g_dc_rendertarget1, 0, 0, 400, 1000, (HBRUSH)GetStockObject(BLACK_BRUSH));
		//
		//g_dc_rendertarget1.SetBrushTransparent(true);
		//for (size_t i = 0; i < 5; i++)
		//{
		//	BYTE c = (i == 0 || i == 4) ? 127 : 31;
		//	g_dc_rendertarget1.SetPenColor(c, c, c);
		//
		//	int y = i * 200 + 100;
		//	g_dc_rendertarget1.SingleLine(0, y, 400, y);
		//}
		//for (size_t i = 0; i < 5; i++)
		//{
		//	BYTE c = (i == 0 || i == 4) ? 255 : 127;
		//	g_dc_rendertarget1.SetPenColor(c, c, c);
		//
		//	int x = i * 100;
		//	g_dc_rendertarget1.SingleLine(x, 0, x, 1000);
		//}
		//g_dc_backbuffer.Paste(g_dc_rendertarget1, 0, 0, 1200 - 1, 0, 400, 1000);
	}


	editorIO.Render();
	g_dc_rendertarget1.SetRenderTarget(editorIO._get_result());
	g_dc_backbuffer.Paste(g_dc_rendertarget1, 0, 0, 0, 0, 1600, 1000);
	g_dc_rendertarget1.DetachRenderTarget();

	g_dc_window.Paste(g_dc_backbuffer, 0, 0, 0, 0, 1600, 1000);
}

void Release()
{
	editorIO.Release();

	g_dc_rendertarget1.Release();
	g_dc_rendertarget2.Release();
	g_dc_backbuffer.Release();
	g_dc_window.Release();

	g_soundDevice.Release();
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	g_inputDevice.MsgProc(message, wParam, lParam);

	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// 신경쓰고싶지않음
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
