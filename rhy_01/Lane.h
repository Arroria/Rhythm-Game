#pragma once
#include <list>
#include <chrono>
using Clock_t = std::chrono::steady_clock;
using ClockTime_t = Clock_t::duration;
using Time_t = std::chrono::nanoseconds;
using TimePoint_t = Clock_t::time_point;

#include <functional>

class Lane
{
public:
	using time_t = Time_t;
	using dataList_t = std::list<time_t>;

	using callback_t = std::function<void(std::string_view)>;

	Lane(dataList_t data, time_t critical, time_t _near, time_t miss, callback_t callback);
	~Lane();


	void update(time_t timing);
	void hit(time_t timing);


	const dataList_t& data() const { return m_data; }
	size_t size() const { return m_data.size(); }
	bool empty() const { return m_data.empty(); }



private:
	dataList_t m_data;
	time_t m_critical;
	time_t m_near;
	time_t m_miss;

	callback_t m_callback;
};


#include <Windows.h>
inline void gotoxy(int x, int y) { SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ (short)x, (short)y }); }
#include <iostream>
