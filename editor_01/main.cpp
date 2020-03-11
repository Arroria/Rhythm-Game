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

	g_soundDevice.Initialize();

	if (BNS_Load("./_gamedata/test.txt", bnsData))
	{
		if (BNS_Save("./_gamedata/test_out.txt", bnsData))
		{
		}
		editor.InjectBNS(bnsData);
		editor.Initialize();
		using namespace std::chrono_literals;
		//editor.LoadMusic("697873717765.mp3", 600ms);
	}
	else
		std::terminate();
}

void Update()
{
	HWND windowHandle = WNDLM.WindowHandle();
	g_inputDevice.BeginFrame(windowHandle);
	g_soundDevice.Update();



	// editor
	const POINT mousePosition = g_inputDevice.MousePos();
	const int mouseWheelDelta = g_inputDevice.MouseWheel() / 120;
	if (editor.MouseInNoteScreen(mousePosition))
	{
		int laneIndex = NULL, beatIndex = NULL;
		if (editor.MouseToNoteScreenNotePosition(mousePosition, laneIndex, beatIndex))
		{
			if (g_inputDevice.IsKeyDown(VK_LBUTTON))
				editor.FixNote(laneIndex, beatIndex, !editor.GetNote(laneIndex, beatIndex));
		}
	}
	else if (editor.MouseInPreviewScreen(mousePosition))
	{
		editor.PreviewScreenScroll(mouseWheelDelta);

		float barPosition = NULL;
		if (editor.MouseToPreviewScreenBarPositon(mousePosition, barPosition))
		{
			if (g_inputDevice.IsKeyDown(VK_LBUTTON) || g_inputDevice.IsKeyPressed('Q'))
				editor.SetEditorCursor_ByBar(barPosition);
		}
	}
	if (g_inputDevice.IsKeyDown(VK_SPACE))
	{
		editor.SetMusicSyncMode(!editor.IsMusicSyncMode());
	}
	editor.Update();


	g_inputDevice.EndFrame();
}

void Render()
{
	// grid
	static WNDRD_RenderTarget window_grid_rt;
	if (!window_grid_rt.Created())
	{
		if (!window_grid_rt.Create(WNDRD, 1600, 1000))
			std::terminate();
		WNDRD.DrawOnRenderTarget(window_grid_rt);
		WNDRD.SetBrushColor(WNDRD.DefaultTransparentColor());
		WNDRD.FillSize(0, 0, 1600, 1000);
		for (int x = 0; x < 1600; x++)
		{
			for (int y = 0; y < 1000; y++)
			{
				int xData = x % 100;
				int yData = y % 100;

				if (xData == 99 || yData == 99)
				{
					int colorPower = (std::min(xData, yData) + 1) * (255. / 100);
					WNDRD.DrawPixel(x, y, RGB(0, 0, colorPower));
					continue;
				}

				if (!xData || !yData)
				{
					int colorPower = (100 - std::max(xData, yData)) * (255. / 100);
					WNDRD.DrawPixel(x, y, RGB(0, colorPower, 0));
					continue;
				}
			}
		}
	}
	WNDRD.DrawOnScreen();
	WNDRD.SetBrushColor(RGB(0, 0, 0));
	WNDRD.FillSize(0, 0, 1600, 1000);
	WNDRD.LinkCopyRenderTarget(window_grid_rt);
	WNDRD.CopyTransparentSize(0, 0, 1600, 1000, 0, 0);
	WNDRD.UnlinkCopyRenderTarget();

	editor.Render();

	WNDRD.Clipping();
}

void Release()
{
	editor.Release();
	WNDRD.Release();

	g_soundDevice.Release();
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
