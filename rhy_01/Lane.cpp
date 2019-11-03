#include "Lane.h"

Lane::Lane(dataList_t data, time_t critical, time_t _near, time_t miss)
	: m_data(std::move(data))
	, m_critical(critical)
	, m_near(_near)
	, m_miss(miss)
{
}

Lane::~Lane()
{
}


void _draw_result(std::string_view type, Time_t relativeTime, Time_t absoluteTime) { std::cout << type << " : " << relativeTime.count() << '(' << absoluteTime.count() << ')' << "                    " << std::endl; }

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
	else if (hittime < -m_near)		_draw_result("Early Miss", hittime, nearnote);
	else if (hittime < -m_critical)	_draw_result("Early", hittime, nearnote);
	else if (hittime < m_critical)	_draw_result("Critical", hittime, nearnote);
	else if (hittime < m_near)		_draw_result("Near", hittime, nearnote);
	else if (hittime < m_miss)		_draw_result("Near Miss", hittime, nearnote);
	else							std::cout << "???" << std::endl;
	pop();
}
