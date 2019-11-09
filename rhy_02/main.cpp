#include "pch.h"



constexpr size_t _console_window_size_x = 400;
constexpr size_t _console_window_size_y = 800;
constexpr size_t _console_buffer_size_x = 40;
constexpr size_t _program_rendering_fps = 0;

constexpr std::string_view _path_test = "../../../../FMOD Studio API Windows/api/core/examples/media/standrews.wav";
constexpr std::string_view _path_song = "697873717765.mp3";





void Initialize();
void Update();
void Render();
void SetConsoleState(size_t x, size_t y, LONG_PTR new_style);

TimePoint_t g_lastKeyUpdateTime;

int main()
{
	SetConsoleState(_console_window_size_x, _console_window_size_y, WS_BORDER | WS_DLGFRAME | WS_MINIMIZEBOX | WS_SYSMENU);
	g_cdb = new ConsoleDoubleBuffer(_console_buffer_size_x);
	g_soundDevice.Initialize();
	Initialize();

	constexpr Time_t _TimePerFPS = _program_rendering_fps ? Time_t(Time_t(std::chrono::seconds(1)) / _program_rendering_fps) : Time_t(0);
	TimePoint_t lastRenderTime(Time_t(0));
	while (!GetAsyncKeyState(VK_ESCAPE) || !GetAsyncKeyState(VK_BACK))
	{
		HWND __dummy(nullptr);
		g_inputDevice.BeginFrame(__dummy);
		{
			Update();

			TimePoint_t thisTime(Clock_t::now());
			if (thisTime - lastRenderTime > _TimePerFPS)
			{
				lastRenderTime = thisTime;
				Render();
			}
		}
		g_inputDevice.EndFrame();
		g_lastKeyUpdateTime = Clock_t::now();
	}
	g_soundDevice.Release();

	delete g_cdb;
	return 0;
}



#include "RhythmGame.h"
inline RhythmGame* g_rhy;
void Initialize()
{
	constexpr size_t readybpm = 198;
	constexpr size_t bpm = 198;
	constexpr Time_t readyunit = std::chrono::duration_cast<Time_t>(1s * (60.0 / readybpm));
	constexpr Time_t unit = Time_t(60s) / bpm;
	constexpr Time_t unitoffset = Time_t(0); Time_t(730ms);
	std::list<Time_t> note[4];
	auto lamb = [&](size_t lane, double _1)
	{
		//note[lane].push_back(readyunit * 4 + unitoffset + std::chrono::duration_cast<Time_t>(unit * _1));
		note[lane].push_back(std::chrono::duration_cast<Time_t>(unit * _1 + 700ms));
	};
	#include "../rhy_01/temp pattern.inl"
	

	g_rhy = new RhythmGame;
	g_rhy->Initialize(SoundSample(g_soundDevice, _path_song), 198, std::move(note[0]), std::move(note[1]), std::move(note[2]), std::move(note[3]));
}

void Update()
{
	g_rhy->Update(g_lastKeyUpdateTime);
}

void Render()
{
	g_cdb->Clear();
	g_cdb->Begin();
	g_rhy->Render();
	g_cdb->End();
	g_cdb->Flipping();
}



void SetConsoleState(size_t x, size_t y, LONG_PTR new_style)
{
	SetLastError(NO_ERROR);
	HWND hwnd_console = GetConsoleWindow();
	SetWindowLongPtr(hwnd_console, GWL_STYLE, new_style);
	SetWindowPos(hwnd_console, 0, 0, 0, x, y, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME);
	ShowWindow(hwnd_console, SW_SHOW);

	_CONSOLE_CURSOR_INFO cursorInfo{ 1, false };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}
