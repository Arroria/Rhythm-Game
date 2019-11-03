#include <Windows.h>
#include <string_view>

#include "SoundDevice.h"
SoundDevice soundDevice;

#include "InputDevice/InputDevice.h"
#pragma comment (lib, "InputDevice/Debug/x64/InputDevice.lib")
InputDevice g_inputDevice;



#include "Lane.h"
#include "LaneVisualizer.h"

constexpr std::string_view _path = "../../../../FMOD Studio API Windows/api/core/examples/media/standrews.wav";

void main2();
int main()
{
	soundDevice.Initialize();
	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		if (GetAsyncKeyState(VK_RETURN))
		main2();
	}
	soundDevice.Release();
	return 0;
}

#include <iostream>
void main2()
{
	using namespace std::chrono_literals;
	
	
	
	constexpr size_t readybpm = 120;
	constexpr size_t bpm = 480;
	constexpr Time_t readyunit = std::chrono::duration_cast<Time_t>(1s * (60.0 / readybpm));
	constexpr Time_t unit = std::chrono::duration_cast<Time_t>(1s * (60.0 / bpm));
	constexpr Time_t unitoffset = 40ms;
		
	std::list<Time_t> note[4];
	std::list<Time_t> soundNote;
	{
		for (size_t i = 0; i < 4; i++)
			soundNote.push_back(readyunit * i);

		constexpr size_t count = 32;
		for (size_t i = 0; i < count; i++)
		{
			soundNote.push_back(readyunit * 4 + unit * i);
			
			if (i % 4 == 0)	note[0].push_back(readyunit * 4 + unit * i + unitoffset);
			if (i % 4 == 1)	note[1].push_back(readyunit * 4 + unit * i + unitoffset);
			if (i % 4 == 2)	note[2].push_back(readyunit * 4 + unit * i + unitoffset);
			if (i % 4 == 3)	note[3].push_back(readyunit * 4 + unit * i + unitoffset);
		}
	}
	Lane lane[4] = {
		{ std::move(note[0]), 33ms, 66ms, 100ms },
		{ std::move(note[1]), 33ms, 66ms, 100ms },
		{ std::move(note[2]), 33ms, 66ms, 100ms },
		{ std::move(note[3]), 33ms, 66ms, 100ms } };
	LaneVisualizer laneVis[4] = {
		{ &lane[0], 720 },
		{ &lane[1], 720 },
		{ &lane[2], 720 },
		{ &lane[3], 720 } };



	SoundSample sound1(soundDevice.CreateSoundSample(_path));
	const TimePoint_t start = Clock_t::now();
	while (soundNote.size() ||
		lane[0].size() ||
		lane[1].size() ||
		lane[2].size() ||
		lane[3].size())
	{
		HWND ___a(nullptr);
		g_inputDevice.BeginFrame(___a);


		const TimePoint_t now = Clock_t::now();
		const Time_t interval = std::chrono::duration_cast<Time_t>(now - start);
		if (soundNote.size())
		{
			if (soundNote.front() < interval)
			{
				sound1.play().detach();
				soundNote.pop_front();
			}
		}

		for (size_t i = 0; i < 4; i++)
		{
			gotoxy(0, 33 + i);
			lane[i].update(interval);
		}
		if (g_inputDevice.IsKeyDown('1'))	{ gotoxy(0, 33); lane[0].hit(interval); }
		if (g_inputDevice.IsKeyDown('2'))	{ gotoxy(0, 34); lane[1].hit(interval); }
		if (g_inputDevice.IsKeyDown('3'))	{ gotoxy(0, 35); lane[2].hit(interval); }
		if (g_inputDevice.IsKeyDown('4'))	{ gotoxy(0, 36); lane[3].hit(interval); }

		for (size_t i = 0; i < 4; i++)
			laneVis[i].Render(i, interval);

		g_inputDevice.EndFrame();
	}

	Sleep(1500);
}
