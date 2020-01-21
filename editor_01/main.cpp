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
	WNDLM.SetFPSLimite(60, 60);

	WNDLM.WndMove(10, 10);
	WNDLM.WndResize(_window_width, _window_height);

	ShowWindow(WNDLM.WindowHandle(), SW_RESTORE);

	return (int)WNDLM.Loop();
}


#include "BeatNoteSheet.h"
BeatNoteSheet bnsData;
#include "BNS_Editor.h"
BNS_Editor editor;
void Init()
{
	Console::Create();
	WNDRD.Initialize(WNDLM.WindowHandle(), _window_width, _window_height);

	editor.Initialize();

	if (BNS_Load("./_gamedata/test.txt", bnsData))
	{
		if (BNS_Save("./_gamedata/test_out.txt", bnsData))
		{
			editor.InjectBNS(bnsData);
		}
	}
}

void Update()
{
	HWND windowHandle = WNDLM.WindowHandle();
	g_inputDevice.BeginFrame(windowHandle);
	
	
	editor.Update();


	g_inputDevice.EndFrame();
}

void Render()
{
	//WNDRD.DrawOnMainBuffer();
	//WNDRD.SetBrushColor(255, 255, 255);
	//WNDRD.Fill(0, 0, _window_width, _window_height);
	//
	//WNDRD.SingleLine(0, 0, _window_width, _window_height);
	//
	//
	//WNDRD.SetPenColor(0, 0, 0);
	//for (size_t lane = 0; lane < bnsData.m_beatData.size(); ++lane)
	//{
	//	auto& noteList = bnsData.m_beatData[lane];
	//	for (size_t beat = 0; beat < noteList.size(); ++beat)
	//	{
	//		if (noteList[beat])
	//			WNDRD.SingleLine(beat, lane * 10, beat, (lane + 1) * 10);
	//	}
	//}

	editor.Render();

	WNDRD.Clipping();
}

void Release()
{
	editor.Release();
	WNDRD.Release();

	Console::Release();
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
