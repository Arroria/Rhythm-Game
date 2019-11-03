#include "LaneVisualizer.h"

LaneVisualizer::LaneVisualizer(Lane* lane, size_t speedBpm)
	: m_lane(lane)
	, m_speedBpm(speedBpm)
{
}

LaneVisualizer::~LaneVisualizer()
{
}



/*
총 길이는 32 + 1
bpm 60당 초당 4칸씩 내려온다 가정
*/
void LaneVisualizer::Render(size_t line, Lane::time_t timing) const
{
	using namespace std::chrono_literals;
	constexpr size_t _down_speed = 4;
	const std::chrono::microseconds unit = std::chrono::duration_cast<std::chrono::microseconds>(1000ms * (60.0 / m_speedBpm) / _down_speed);

	const auto& list = m_lane->data();


	for (size_t i = 0; i < 33; i++)
	{
		gotoxy(line * 2, i);
		std::cout << "  ";
	}

	for (auto iter = list.cbegin(); iter != list.cend(); ++iter)
	{
		Lane::time_t noteTime = *iter;
		Lane::time_t interval = -(timing - noteTime);

		int t = interval / unit;
		if (t > 31)
			break;
		if (t < -1)
			continue;

		gotoxy(line * 2, 32 - (t + 1));
		std::cout << (t == 0 ? "■" : "□");
	}
}
