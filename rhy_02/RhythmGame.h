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


	RhythmGame();
	~RhythmGame();


	void Initialize(SoundSample&& music, size_t musinBpm, std::list<time_t> _1, std::list<time_t> _2, std::list<time_t> _3, std::list<time_t> _4);
	void Update(TimePoint_t inputTime);
	void Render();

	void SetKey(input_key_code_t lane1, input_key_code_t lane2, input_key_code_t lane3, input_key_code_t lane4, input_key_code_t offsetL, input_key_code_t offsetR, input_key_code_t speedL, input_key_code_t speedR);


private:
	// For game
	std::array<std::pair<std::unique_ptr<Lane>, input_key_code_t>, _lane_count> m_lane;
	
	bool m_isRunning;
	TimePoint_t m_soundBeginTime;

	SoundSample m_music;
	SoundChannel m_mChannel;
	size_t m_musicBpm;

	// For user	
	size_t m_noteSpeed;
	Time_t m_offset;

	input_key_code_t m_offsetL;
	input_key_code_t m_offsetR;
	input_key_code_t m_speedL;
	input_key_code_t m_speedR;


	void WaitUpate();
	void RunningUpdate(TimePoint_t inputTime);
};

class Lane
{
public:
	using time_t = RhythmGame::time_t;
	using list_t = std::list<time_t>;
	using event_t = RhythmGame::event_t;

	Lane(list_t noteList, time_t perfect_time, time_t near_time, time_t miss_time, event_t event_);
	~Lane();


	void miss_check(time_t delta);
	void hit(time_t delta);

	const list_t& data() const { return m_noteList; }


private:
	list_t m_noteList;
	time_t m_perfect_time;
	time_t m_near_time;
	time_t m_miss_time;

	event_t m_event;
};

