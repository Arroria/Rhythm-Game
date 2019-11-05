#include "pch.h"
#include "RhytheGame.h"

#include <iostream>
#include <chrono>


inline TimePoint_t g_lastKeyUpdateTime;

void Initialize();
void Update();
void Render();
int main()
{
	g_soundDevice.Initialize();
	Initialize();

	constexpr size_t _FPS = 60;
	constexpr Time_t _TimePerFPS = Time_t(std::chrono::seconds(1)) / _FPS;
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
	return 0;
}


constexpr std::string_view _path_test = "../../../../FMOD Studio API Windows/api/core/examples/media/standrews.wav";
constexpr std::string_view _path_song = "697873717765.mp3";

#include "RhytheGame.h"
inline RhytheGame* g_rhy;
void Initialize()
{
	using namespace std::chrono_literals;
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

	g_rhy = new RhytheGame;
	g_rhy->Initialize(SoundSample(g_soundDevice, _path_song), std::move(note[0]), std::move(note[1]), std::move(note[2]), std::move(note[3]));
}

void Update()
{
	g_rhy->Update(g_lastKeyUpdateTime);
}

void Render()
{
	g_rhy->Render();
}
