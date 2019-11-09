#include "pch.h"
#include "RhythmGame.h"

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





RhythmGame::RhythmGame()
	: m_isRunning(false)
	, m_soundBeginTime(m_soundBeginTime.min())

	, m_music()
	, m_mChannel()

	, m_noteSpeed(_default_note_speed), m_offset(0)
	
	, m_offsetL(VK_NEXT), m_offsetR(VK_PRIOR)
	, m_speedL(VK_OEM_COMMA), m_speedR(VK_OEM_PERIOD)

{
	m_lane[0].second = '7';
	m_lane[1].second = '8';
	m_lane[2].second = '9';
	m_lane[3].second = '0';
}

RhythmGame::~RhythmGame()
{
}


void temp(JudgementScore_t s)
{
	//g_cdb->CursorTo(0, 34);
	//switch (s)
	//{
	//case Perfect:	std::cout << "Perfect" << std::endl;break;
	//case Early:		std::cout << "Early  " << std::endl;break;
	//case Late:		std::cout << "Late   " << std::endl;break;
	//case Miss:		std::cout << "Miss   " << std::endl;break;
	//case Lost:		std::cout << "Lost   " << std::endl;break;
	//}
}

void RhythmGame::Initialize(SoundSample&& music, size_t musicBpm, Lane::list_t _1, Lane::list_t _2, Lane::list_t _3, Lane::list_t _4)
{
	m_music = std::move(music);
	m_musicBpm = musicBpm;

	m_lane[0].first = std::move(std::make_unique<Lane>(std::move(_1), _perfect_time, _near_time, _miss_time, temp));
	m_lane[1].first = std::move(std::make_unique<Lane>(std::move(_2), _perfect_time, _near_time, _miss_time, temp));
	m_lane[2].first = std::move(std::make_unique<Lane>(std::move(_3), _perfect_time, _near_time, _miss_time, temp));
	m_lane[3].first = std::move(std::make_unique<Lane>(std::move(_4), _perfect_time, _near_time, _miss_time, temp));
}

void RhythmGame::Update(TimePoint_t inputTime)
{
	if (g_inputDevice.IsKeyDown(m_offsetL))	m_offset = std::max(Time_t(-16ms), m_offset - _offset_unity);
	if (g_inputDevice.IsKeyDown(m_offsetR))	m_offset = std::min(Time_t(+16ms), m_offset + _offset_unity);
	if (g_inputDevice.IsKeyDown(m_speedL))	m_noteSpeed = std::max(size_t(05), m_noteSpeed - (!g_inputDevice.IsKeyPressed(VK_LSHIFT) ? 5 : 1));
	if (g_inputDevice.IsKeyDown(m_speedR))	m_noteSpeed = std::min(size_t(80), m_noteSpeed + (!g_inputDevice.IsKeyPressed(VK_LSHIFT) ? 5 : 1));

	m_isRunning ?
		RunningUpdate(inputTime) :
		WaitUpate();
}

void RhythmGame::WaitUpate()
{
	if (g_inputDevice.IsKeyDown(VK_RETURN))
	{
		m_mChannel = SoundChannel(m_music);
		m_soundBeginTime = Clock_t::now();
		m_isRunning = true;
	}
}

void RhythmGame::RunningUpdate(TimePoint_t inputTime)
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

void RhythmGame::Render()
{
	// GAME BPM
	g_cdb->CursorTo(0, 33);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(m_offset).count() << "ms  ";
	// OFFSET
	g_cdb->CursorTo(0, 34);
	std::cout << m_musicBpm << "б┐" << m_noteSpeed / 10.f << "=" << m_musicBpm * m_noteSpeed / 10 << "    ";

	if (!m_isRunning)
		return;



	time_t current = Clock_t::now() - m_soundBeginTime;
	for (size_t index = 0; index < _lane_count; ++index)
	{
		const Lane::list_t& lane = (*m_lane[index].first).data();
		for (auto iter = lane.begin(); iter != lane.end(); ++iter)
		{
			const Time_t unit = std::chrono::duration_cast<Time_t>(Time_t(60s) / (m_musicBpm * _note_fall_speed_per_hz * m_noteSpeed / 10.0));
			Lane::time_t noteTime = *iter;
			Lane::time_t interval = current - noteTime;	// late be plus
			Lane::time_t interval_offsetted = interval - unit / 2 - m_offset;
			auto notePos = interval_offsetted / unit;
			if (interval_offsetted > interval_offsetted.zero())
				++notePos;
			auto notePos_forScreen = 31 + notePos;
			
			if (notePos_forScreen < 0)	break;
			if (notePos_forScreen > 32)	continue;

			std::string_view box;
			if constexpr (false) //default
				box = notePos_forScreen == 31 ? "бс" : "бр";
			else if constexpr (false) // perfect
			{
				auto temp = current - noteTime;
				box = (-RhythmGame::_perfect_time <= temp && temp <= RhythmGame::_perfect_time) ? "бс" : "бр";
			}
			else // check
			{
				auto temp = interval_offsetted = interval - unit / 2;
				box = (-unit / 2 <= temp && temp <= unit / 2) ? "бс" : "бр";
			}

			g_cdb->CursorTo(index * 2, notePos_forScreen);
			std::cout << box;
		}
	}
}


void RhythmGame::SetKey(input_key_code_t lane1, input_key_code_t lane2, input_key_code_t lane3, input_key_code_t lane4, input_key_code_t offsetL, input_key_code_t offsetR, input_key_code_t speedL, input_key_code_t speedR)
{
	if (lane1)		m_lane[0].second = lane1;
	if (lane2)		m_lane[1].second = lane2;
	if (lane3)		m_lane[2].second = lane3;
	if (lane4)		m_lane[3].second = lane4;
	if (offsetL)	m_offsetL	= offsetL;
	if (offsetR)	m_offsetR	= offsetR;
	if (speedL)		m_speedL	= speedL;
	if (speedR)		m_speedR	= speedR;
}



