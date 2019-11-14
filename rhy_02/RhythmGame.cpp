#include "pch.h"
#include "RhythmGame.h"

Lane::Lane(list_t noteList, event_t event_)
	: m_noteList(std::move(noteList))
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

		if (!(interval > _miss_time))
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
		 if (interval > _near_time)			jscore = JudgementScore_t::Miss;
	else if (interval > _perfect_time)		jscore = JudgementScore_t::Late;
	else if (interval >= -_perfect_time)	jscore = JudgementScore_t::Perfect;
	else if (interval >= -_near_time)		jscore = JudgementScore_t::Early;
	else if (interval >= -_miss_time)		jscore = JudgementScore_t::Miss;
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
	, m_musicBpm(NULL)

	, m_keySetting{}
	, m_scoreData()
	, m_combo(0)
	, m_comboMax(0)
	, m_noteSpeed(_default_note_speed)
	, m_offset(0)
{
	m_keySetting.lane[0] = '7';
	m_keySetting.lane[1] = '8';
	m_keySetting.lane[2] = '9';
	m_keySetting.lane[3] = '0';
	m_keySetting.offsetL = VK_NEXT;
	m_keySetting.offsetR = VK_PRIOR;
	m_keySetting.speedL = VK_OEM_COMMA;
	m_keySetting.speedR = VK_OEM_PERIOD;

	m_scoreData.fill(0);
}

RhythmGame::~RhythmGame()
{
}




void RhythmGame::Initialize(SoundSample&& music, size_t musicBpm, Lane::list_t _1, Lane::list_t _2, Lane::list_t _3, Lane::list_t _4)
{
	m_music = std::move(music);
	m_musicBpm = musicBpm;

	m_lane[0] = std::move(std::make_unique<Lane>(std::move(_1), [this](JudgementScore_t _0){ ScoreUpdate(_0); }));
	m_lane[1] = std::move(std::make_unique<Lane>(std::move(_2), [this](JudgementScore_t _0){ ScoreUpdate(_0); }));
	m_lane[2] = std::move(std::make_unique<Lane>(std::move(_3), [this](JudgementScore_t _0){ ScoreUpdate(_0); }));
	m_lane[3] = std::move(std::make_unique<Lane>(std::move(_4), [this](JudgementScore_t _0){ ScoreUpdate(_0); }));
}

void RhythmGame::Update(TimePoint_t inputTime)
{
	if (g_inputDevice.IsKeyDown(m_keySetting.offsetL))	m_offset = std::max(Time_t(-16ms), m_offset - _offset_unity);
	if (g_inputDevice.IsKeyDown(m_keySetting.offsetR))	m_offset = std::min(Time_t(+16ms), m_offset + _offset_unity);
	if (g_inputDevice.IsKeyDown(m_keySetting.speedL))	m_noteSpeed = std::max(size_t(05), m_noteSpeed - (!g_inputDevice.IsKeyPressed(VK_LSHIFT) ? 5 : 1));
	if (g_inputDevice.IsKeyDown(m_keySetting.speedR))	m_noteSpeed = std::min(size_t(80), m_noteSpeed + (!g_inputDevice.IsKeyPressed(VK_LSHIFT) ? 5 : 1));

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
		Lane& lane = *m_lane[index];
		input_key_code_t keyCode = m_keySetting.lane[index];

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


	// Note
	time_t current = Clock_t::now() - m_soundBeginTime;
	for (size_t index = 0; index < _lane_count; ++index)
	{
		const Lane::list_t& lane = (*m_lane[index]).data();
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



	// Score
	g_cdb->CursorTo(20, 10 + 0);	std::cout << "Perfect    : " << m_scoreData[0] << "     ";
	g_cdb->CursorTo(20, 10 + 1);	std::cout << "Early      : " << m_scoreData[1] << "     ";
	g_cdb->CursorTo(20, 10 + 2);	std::cout << "Late       : " << m_scoreData[2] << "     ";
	g_cdb->CursorTo(20, 10 + 3);	std::cout << "Lost       : " << m_scoreData[3] << "     ";
	g_cdb->CursorTo(20, 10 + 4);	std::cout << "Miss       : " << m_scoreData[4] << "     ";

	g_cdb->CursorTo(20, 16);		std::cout << "Combo      : " << m_combo << "     ";
	g_cdb->CursorTo(20, 17);		std::cout << "Combo Max  : " << m_comboMax << "     ";

	constexpr size_t _max_note = 1100;
	constexpr size_t _max_score = 1000000;
	size_t myScore = 0;
	myScore += (_max_score * m_scoreData[0] / _max_note * 1);
	myScore += (_max_score * m_scoreData[1] / _max_note * 0.5);
	myScore += (_max_score * m_scoreData[2] / _max_note * 0.5);
	myScore += (_max_score * m_scoreData[3] / _max_note * 0.1);
	myScore += (_max_score * m_scoreData[4] / _max_note * 0);

	g_cdb->CursorTo(20, 18);
	std::cout << "Score      : " << myScore;
}





void RhythmGame::SetKeySetting(KeySetting keySetting)
{
	if (keySetting.lane[0])	m_keySetting.lane[0]	= keySetting.lane[0];
	if (keySetting.lane[1])	m_keySetting.lane[1]	= keySetting.lane[1];
	if (keySetting.lane[2])	m_keySetting.lane[2]	= keySetting.lane[2];
	if (keySetting.lane[3])	m_keySetting.lane[3]	= keySetting.lane[3];
	if (keySetting.offsetL)	m_keySetting.offsetL	= keySetting.offsetL;
	if (keySetting.offsetR)	m_keySetting.offsetR	= keySetting.offsetR;
	if (keySetting.speedL)	m_keySetting.speedL		= keySetting.speedL;
	if (keySetting.speedR)	m_keySetting.speedR		= keySetting.speedR;
}

void RhythmGame::ScoreUpdate(JudgementScore_t score)
{
	++m_scoreData[score];
	score == JudgementScore_t::Lost || score == JudgementScore_t::Miss ?
		m_combo = 0 :
		m_comboMax = std::max(m_comboMax, ++m_combo);
}
