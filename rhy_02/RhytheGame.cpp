#include "pch.h"
#include "RhytheGame.h"

Lane::Lane(list_t noteList, time_t perfect_time, time_t near_time, time_t miss_time, event_t event_)
	: m_noteList(std::move(noteList))
	, m_perfect_time(perfect_time), m_near_time(near_time), m_miss_time(miss_time)
	
	, m_event(event_)
{
}

Lane::~Lane()
{
}



void Lane::miss_check(time_t delta)
{
	while (m_noteList.size())
	{
		const auto& note = m_noteList.front();
		time_t interval = delta - note;

		if (!(interval > m_miss_time))
			break;
		
		m_noteList.pop_front();
		m_event(JudgementScore_t::Lost);
	}
}

void Lane::hit(time_t delta)
{
	if (m_noteList.empty())
		return;

	const auto& note = m_noteList.front();
	time_t interval = delta - note;

	JudgementScore_t jscore(JudgementScore_t::Lost);
	if (interval > m_near_time)				jscore = JudgementScore_t::Miss;
	else if (interval > m_perfect_time)		jscore = JudgementScore_t::Late;
	else if (interval >= -m_perfect_time)	jscore = JudgementScore_t::Perfect;
	else if (interval >= -m_near_time)		jscore = JudgementScore_t::Early;
	else if (interval >= -m_miss_time)		jscore = JudgementScore_t::Miss;
	else 
		return;

	m_noteList.pop_front();
	m_event(jscore);
}





RhytheGame::RhytheGame()
	: m_isRunning(false)
	, m_soundBeginTime(m_soundBeginTime.min())

	, m_music()
	, m_mChannel()
{
}

RhytheGame::~RhytheGame()
{
}


void temp(JudgementScore_t s)
{
	gotoxy(0, 34);
	switch (s)
	{
	case Perfect:	std::cout << "Perfect" << std::endl;break;
	case Early:		std::cout << "Early  " << std::endl;break;
	case Late:		std::cout << "Late   " << std::endl;break;
	case Miss:		std::cout << "Miss   " << std::endl;break;
	case Lost:		std::cout << "Lost   " << std::endl;break;
	}
}
void RhytheGame::Initialize(SoundSample&& music, Lane::list_t _1, Lane::list_t _2, Lane::list_t _3, Lane::list_t _4)
{
	m_music = std::move(music);

	m_lane[0].first = std::move(std::make_unique<Lane>(_1, _perfect_time, _near_time, _miss_time, temp));
	m_lane[1].first = std::move(std::make_unique<Lane>(_2, _perfect_time, _near_time, _miss_time, temp));
	m_lane[2].first = std::move(std::make_unique<Lane>(_3, _perfect_time, _near_time, _miss_time, temp));
	m_lane[3].first = std::move(std::make_unique<Lane>(_4, _perfect_time, _near_time, _miss_time, temp));
	m_lane[0].second = '7';
	m_lane[1].second = '8';
	m_lane[2].second = '9';
	m_lane[3].second = '0';
}

void RhytheGame::Update(TimePoint_t inputTime)
{
	m_isRunning ?
		RunningUpdate(inputTime) :
		WaitUpate();
}

void RhytheGame::WaitUpate()
{
	if (g_inputDevice.IsKeyDown(VK_RETURN))
	{
		m_mChannel = SoundChannel(m_music);
		m_soundBeginTime = Clock_t::now();
		m_isRunning = true;
	}
}

void RhytheGame::RunningUpdate(TimePoint_t inputTime)
{
	if (g_inputDevice.IsKeyDown(VK_BACK))
	{
		m_mChannel.release();
		m_isRunning = false;
		return;
	}


	time_t delta = inputTime - m_soundBeginTime;
	for (size_t index = 0; index < _lane_count; ++index)
	{
		Lane& lane = *m_lane[index].first;
		input_key_code_t keyCode = m_lane[index].second;

		lane.miss_check(delta);
		if (g_inputDevice.IsKeyDown(keyCode))
			lane.hit(delta);
	}
}

void RhytheGame::Render()
{
	if (!m_isRunning)
		return;

	system("cls");
	time_t current = Clock_t::now() - m_soundBeginTime;

	for (size_t index = 0; index < _lane_count; index++)
	{
		const Lane::list_t& lane = (*m_lane[index].first).data();
		for (auto iter = lane.begin(); iter != lane.end(); ++iter)
		{
			using namespace std::chrono_literals;
			// constexpr size_t readybpm = 198;
			// constexpr Time_t readyunit = std::chrono::duration_cast<Time_t>(1s * (60.0 / readybpm));
			constexpr size_t bpm = 198;
			constexpr size_t _note_down_speed = 4;	// 60bpm 기준 초당 내려오는 칸의 수
			//constexpr Time_t unitoffset = Time_t(730ms);

			const Time_t unit = Time_t(60s) / (bpm * _note_down_speed * 3);
			Lane::time_t noteTime = *iter;
			Lane::time_t interval = current - (noteTime + unit / 2);	// late be plus
			auto notePos = interval / unit;
			auto notePos_forScreen = 32 + notePos;

			if (notePos_forScreen < 0)	break;
			if (notePos_forScreen > 32)	continue;

			std::string box = notePos_forScreen == 31 ? "■" : "□";

			gotoxy(index * 2, notePos_forScreen);
			std::cout << box;
		}
	}
}



