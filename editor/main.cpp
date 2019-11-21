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


SoundSample ss;
POINT pos{ 0, 0 };
MyDC homeDC, backDC;
void Init()
{
	g_soundDevice.Initialize();
	ss = g_soundDevice.CreateSoundSample("handclap.wav");
	ss.play().detach();


	homeDC.Initialize(WNDLM.WindowHandle());
	backDC.Initialize(homeDC, 1600, 1000);

	homeDC.SetPenColor(128, 128, 0);
	homeDC.SetBrushColor(0, 0, 255);
	backDC.SetPenColor(0, 255, 0);
	backDC.SetBrushColor(32, 0, 0);


	MyDC_FillRect(homeDC, 1500, 900, 1598, 998, (HBRUSH)GetStockObject(GRAY_BRUSH));
	MyDC_FillRect(backDC, 1500, 900, 1598, 998, (HBRUSH)GetStockObject(GRAY_BRUSH));
}

void Update()
{
	HWND handle = WNDLM.WindowHandle();
	g_inputDevice.BeginFrame(handle);

	if (g_inputDevice.IsKeyDown('1'))	{ backDC.SetPenColor(0, 0, 0); backDC.SetBrushColor(255, 255, 255); }
	if (g_inputDevice.IsKeyDown('2'))	{ backDC.SetPenColor(255, 0, 0); backDC.SetBrushColor(0, 255, 0); }
	if (g_inputDevice.IsKeyDown('3'))	{ backDC.SetBrushTransparent(false); }
	if (g_inputDevice.IsKeyDown('4'))	{ backDC.SetBrushTransparent(true); }

	if (g_inputDevice.IsKeyDown('A'))	pos.x -= 5;
	if (g_inputDevice.IsKeyDown('D'))	pos.x += 5;
	if (g_inputDevice.IsKeyDown('W'))	pos.y -= 5;
	if (g_inputDevice.IsKeyDown('S'))	pos.y += 5;


	if (g_inputDevice.IsKeyDown(VK_RBUTTON))	backDC.LinePoint(g_inputDevice.MousePos().x, g_inputDevice.MousePos().y);
	if (g_inputDevice.IsKeyDown(VK_LBUTTON))	backDC.LineLink(g_inputDevice.MousePos().x, g_inputDevice.MousePos().y);


	if (g_inputDevice.IsKeyDown(VK_RETURN)) { MyDC_FillRect(backDC, 0, 0, 1600, 1000, (HBRUSH)GetStockObject(WHITE_BRUSH)); }



	g_inputDevice.EndFrame();
}

void Render()
{
	backDC.DrawRectBySize(pos.x, pos.y, 10, 10);
	homeDC.Paste(backDC, 0, 0, 0, 0, 500, 500);
}

void Release()
{
	homeDC.Release();
	backDC.Release();


	ss.release();
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
