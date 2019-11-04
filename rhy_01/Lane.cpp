#include "Lane.h"

Lane::Lane(dataList_t data, time_t critical, time_t _near, time_t miss, callback_t callback)
	: m_data(std::move(data))
	, m_critical(critical)
	, m_near(_near)
	, m_miss(miss)

	, m_callback(callback)
{
}

Lane::~Lane()
{
}



//void _draw_result(std::string_view type, std::chrono::milliseconds relativeTime, std::chrono::milliseconds absoluteTime) { std::cout << type << " : " << (relativeTime.count() > 0 ? "" : "+") << -(relativeTime.count()) << '(' << absoluteTime.count() << ')' << "                    " << std::endl; }
void _draw_result(std::string_view type, std::chrono::milliseconds relativeTime, std::chrono::milliseconds absoluteTime) { std::cout << type << " : " << (relativeTime.count() > 0 ? "" : "+") << -(relativeTime.count()) << "ms                    " << std::endl; }
void _draw_result(std::string_view type, Time_t relativeTime, Time_t absoluteTime) { _draw_result(type, std::chrono::duration_cast<std::chrono::milliseconds>(relativeTime), std::chrono::duration_cast<std::chrono::milliseconds>(absoluteTime)); }

void Lane::update(time_t timing)
{
	if (m_data.empty())
		return;

	auto nearnote = m_data.front();
	auto hittime = timing - nearnote;
	auto pop = [this]() { m_data.pop_front(); };

	if (hittime > m_miss)
	{
		_draw_result("Miss", hittime, nearnote);
		m_callback("m");
		pop();
	}
}

void Lane::hit(time_t timing)
{
	if (m_data.empty())
		return;

	auto nearnote = m_data.front();
	auto hittime = timing - nearnote;
	auto pop = [this]() { m_data.pop_front(); };

	if (hittime < -m_miss)			return;
	else if (hittime < -m_near)		{ _draw_result("Early Miss", hittime, nearnote);	m_callback("em"); }
	else if (hittime < -m_critical)	{ _draw_result("Early", hittime, nearnote);			m_callback("e"); }
	else if (hittime < m_critical)	{ _draw_result("Perfect", hittime, nearnote);		m_callback("p"); }
	else if (hittime < m_near)		{ _draw_result("Late", hittime, nearnote);			m_callback("l"); }
	else if (hittime < m_miss)		{ _draw_result("Late Miss", hittime, nearnote);		m_callback("lm"); }
	//else							std::cout << "???" << std::endl;
	pop();
}
