#include <Windows.h>
#include <string_view>

#include "SoundDevice.h"
SoundDevice soundDevice;

#include "InputDevice/InputDevice.h"
#pragma comment (lib, "InputDevice/Debug/x64/InputDevice.lib")
InputDevice g_inputDevice;



#include "Lane.h"
#include "GameVisualizer.h"

constexpr std::string_view _path_test = "../../../../FMOD Studio API Windows/api/core/examples/media/standrews.wav";
constexpr std::string_view _path_song = "697873717765.mp3";

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

	size_t score[6] = { NULL ,NULL ,NULL ,NULL ,NULL ,NULL };
	size_t combo[2] = { NULL, NULL };

	auto _combo_plus = [&combo]()
	{
		if (++combo[0] > combo[1])
		{
			combo[1] = combo[0];
			gotoxy(20, 18);
			std::cout << "Combo Max  : " << combo[1] << "     ";
		}
		gotoxy(20, 17);
		std::cout << "Combo      : " << combo[0] << "     ";
	};
	auto _combo_end = [&combo]()
	{
		combo[0] = 0;
		gotoxy(20, 17);
		std::cout << "Combo      : 0     ";
	};

	auto _score_callback = [&score, &_combo_plus, &_combo_end](std::string_view type)->void
	{
		if (type == "p")		{ ++score[0]; gotoxy(20, 10 + 0); std::cout << "Perfect    : " << score[0] << "     "; _combo_plus(); }
		else if (type == "e")	{ ++score[1]; gotoxy(20, 10 + 1); std::cout << "Early      : " << score[1] << "     "; _combo_plus(); }
		else if (type == "l")	{ ++score[2]; gotoxy(20, 10 + 2); std::cout << "Late       : " << score[2] << "     "; _combo_plus(); }
		else if (type == "em")	{ ++score[3]; gotoxy(20, 10 + 3); std::cout << "Early Miss : " << score[3] << "     "; _combo_end(); }
		else if (type == "lm")	{ ++score[4]; gotoxy(20, 10 + 4); std::cout << "Late Miss  : " << score[4] << "     "; _combo_end(); }
		else if (type == "m")	{ ++score[5]; gotoxy(20, 10 + 5); std::cout << "Miss       : " << score[5] << "     "; _combo_end(); }
	};

	
	
	constexpr size_t readybpm = 198;
	constexpr size_t bpm = 198;
	constexpr Time_t readyunit	= std::chrono::duration_cast<Time_t>(1s * (60.0 / readybpm));
	constexpr Time_t unit		= std::chrono::duration_cast<Time_t>(1s * (60.0 / bpm));
	constexpr Time_t unitoffset = Time_t(730ms);
		
	std::list<Time_t> note[4];
	std::list<Time_t> soundNote;
	Time_t songPlayTime(0);
	{
		for (size_t i = 0; i < 4; i++)
			;// soundNote.push_back(readyunit * i);
		songPlayTime = readyunit * 4;

		constexpr size_t count = 128*3;
		for (size_t i = 0; i < count; i++)
		{
			//soundNote.push_back(readyunit * 4 + unit * i);
			
			//if (i % 4 == 0)	note[0].push_back(readyunit * 4 + unit * i + unitoffset);
			//if (i % 4 == 1)	note[0].push_back(readyunit * 4 + unit * i + unitoffset);
			//if (i % 4 == 2)	note[0].push_back(readyunit * 4 + unit * i + unitoffset);
			//if (i % 4 == 3)	note[0].push_back(readyunit * 4 + unit * i + unitoffset);
		}

		//custom
		{
			auto lamb = [&](size_t lane, double _1)
			{
				note[lane].push_back(readyunit * 4 + unitoffset + std::chrono::duration_cast<Time_t>(unit * _1));
			};

			#include "temp pattern.inl"
		}
	}
	Lane lane[4] = {
		{ std::move(note[0]), 33ms, 66ms, 100ms, _score_callback },
		{ std::move(note[1]), 33ms, 66ms, 100ms, _score_callback },
		{ std::move(note[2]), 33ms, 66ms, 100ms, _score_callback },
		{ std::move(note[3]), 33ms, 66ms, 100ms, _score_callback } };

	constexpr size_t _game_bpm_magnification = 3;
	GameVisualizer gameVis(&lane[0], &lane[1], &lane[2], &lane[3], bpm * _game_bpm_magnification);


	static int offset = 0;


	SoundSample _test_sound(soundDevice.CreateSoundSample(_path_test));
	SoundSample song(soundDevice.CreateSoundSample(_path_song));
	const TimePoint_t start = Clock_t::now();
	while (!GetAsyncKeyState(VK_TAB))
	{
		HWND ___a(nullptr);
		g_inputDevice.BeginFrame(___a);


		const TimePoint_t now = Clock_t::now();
		const Time_t _primitive_interval = std::chrono::duration_cast<Time_t>(now - start);
		const Time_t interval = _primitive_interval + Time_t(1ms) * offset;

		// 음악 재생
		if (songPlayTime.count() && songPlayTime <= interval)
		{
			songPlayTime = 0ns;
			song.play().detach();
		}

		// 박자음 begin
		if (soundNote.size())
		{
			if (soundNote.front() < interval)
			{
				_test_sound.play().detach();
				soundNote.pop_front();
			}
		}
		// 박자음 end

		for (size_t i = 0; i < 4; i++)
		{
			gotoxy(0, 33 + i);
			lane[i].update(interval);
		}
		if (g_inputDevice.IsKeyDown('7') || g_inputDevice.IsKeyDown(VK_SPACE))	{ gotoxy(0, 33); lane[0].hit(interval); }
		if (g_inputDevice.IsKeyDown('8'))	{ gotoxy(0, 34); lane[1].hit(interval); }
		if (g_inputDevice.IsKeyDown('9'))	{ gotoxy(0, 35); lane[2].hit(interval); }
		if (g_inputDevice.IsKeyDown('0'))	{ gotoxy(0, 36); lane[3].hit(interval); }

		gotoxy(8, 31); std::cout << "─";
		gameVis.Render(interval);


		if (g_inputDevice.IsKeyDown(VK_PRIOR))	{ ++offset; }
		if (g_inputDevice.IsKeyDown(VK_NEXT))	{ --offset; }
		gotoxy(0, 38); std::cout << offset;

		g_inputDevice.EndFrame();
	}
}
