#pragma once

enum JudgementScore_t
{
	Perfect,
	Early,
	Late,
	Miss,
	Lost,
};


#include <array>
#include <memory>
#include <list>
#include <chrono>
#include <functional>
class Lane;
class RhytheGame
{
public:
	using time_t = Time_t;
	using event_t = std::function<void(JudgementScore_t)>;
	using input_key_code_t = int;
	static constexpr size_t _lane_count = 4;
	static constexpr Time_t _perfect_time	= std::chrono::milliseconds(33);
	static constexpr Time_t _near_time		= std::chrono::milliseconds(66);
	static constexpr Time_t _miss_time		= std::chrono::milliseconds(100);

	RhytheGame();
	~RhytheGame();


	void Initialize(SoundSample&& music, std::list<time_t> _1, std::list<time_t> _2, std::list<time_t> _3, std::list<time_t> _4);
	void Update(TimePoint_t inputTime);
	void Render();


private:
	std::array<std::pair<std::unique_ptr<Lane>, input_key_code_t>, _lane_count> m_lane;
	
	bool m_isRunning;
	TimePoint_t m_soundBeginTime;

	SoundSample m_music;
	SoundChannel m_mChannel;



	void WaitUpate();
	void RunningUpdate(TimePoint_t inputTime);
};

class Lane
{
public:
	using time_t = RhytheGame::time_t;
	using list_t = std::list<time_t>;
	using event_t = RhytheGame::event_t;

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

