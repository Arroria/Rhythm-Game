#pragma once

#include <array>
#include <memory>
#include <list>
#include <chrono>
#include <functional>
using namespace std::chrono_literals;

enum JudgementScore_t
{
	Perfect,
	Early,
	Late,
	Miss,
	Lost,
	__MAX
};

class Lane;
class RhythmGame
{
public:
	using time_t = Time_t;
	using event_t = std::function<void(JudgementScore_t)>;
	using input_key_code_t = int;
	static constexpr size_t _lane_count = 4;
	static constexpr size_t _note_fall_speed_per_hz = 4;	// 60bpm 기준 초당 내려오는 칸의 수
	static constexpr Time_t _perfect_time	= 33ms;
	static constexpr Time_t _near_time		= 66ms;
	static constexpr Time_t _miss_time		= 100ms;
	static constexpr size_t _default_note_speed	= 10;
	static constexpr Time_t _offset_unity		= 1ms;


	struct KeySetting
	{
		std::array<input_key_code_t, 4> lane;
		input_key_code_t offsetL;
		input_key_code_t offsetR;
		input_key_code_t speedL;
		input_key_code_t speedR;
	};



	RhythmGame();
	~RhythmGame();


	void Initialize(SoundSample&& music, size_t musinBpm, std::list<time_t> _1, std::list<time_t> _2, std::list<time_t> _3, std::list<time_t> _4);
	void Update(TimePoint_t inputTime);
	void Render();

	void SetKeySetting(KeySetting keySetting);


private:
	// For game
	std::array<std::unique_ptr<Lane>, _lane_count> m_lane;
	
	bool m_isRunning;
	TimePoint_t m_soundBeginTime;

	SoundSample m_music;
	SoundChannel m_mChannel;
	size_t m_musicBpm;

	// For user	
	KeySetting m_keySetting;
	std::array<size_t, JudgementScore_t::__MAX> m_scoreData;
	size_t m_combo;
	size_t m_comboMax;
	size_t m_noteSpeed;
	Time_t m_offset;



	void WaitUpate();
	void RunningUpdate(TimePoint_t inputTime);

	void ScoreUpdate(JudgementScore_t score);
};

class Lane
{
public:
	using time_t = RhythmGame::time_t;
	using list_t = std::list<time_t>;
	using event_t = RhythmGame::event_t;

	static constexpr Time_t _perfect_time	= RhythmGame::_perfect_time;
	static constexpr Time_t _near_time		= RhythmGame::_near_time;
	static constexpr Time_t _miss_time		= RhythmGame::_miss_time;

	Lane(list_t noteList, event_t event_);
	~Lane();


	void miss_check(time_t delta);
	void hit(time_t delta);

	const list_t& data() const { return m_noteList; }


private:
	list_t m_noteList;
	event_t m_event;
};

