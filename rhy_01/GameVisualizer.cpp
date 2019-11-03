#include "GameVisualizer.h"

GameVisualizer::GameVisualizer(Lane* lane1, Lane* lane2, Lane* lane3, Lane* lane4, size_t speedBpm)
	: m_lane{ lane1, lane2, lane3, lane4 }
	, m_speedBpm(speedBpm)
{
	if (!lane1 || !lane2 || !lane3 || !lane4)
		abort();
}
GameVisualizer::~GameVisualizer()
{
}



/*
�� ���̴� 32 + 1
bpm 60�� �ʴ� 4ĭ�� �����´� ����
*/
void GameVisualizer::Render(Lane::time_t timing) const
{
	using namespace std::chrono_literals;


	// ���� ���� begin
	constexpr size_t _line_size_x = 4;
	constexpr size_t _line_size_y = 33;
	constexpr size_t _buffer_size_x = _line_size_x * 2 + 1;
	std::string buffer(_buffer_size_x * _line_size_y, ' ');
	for (size_t i = 0; i < _line_size_y; i++)
		buffer[_buffer_size_x * (i + 1) - 1] = '\n';

	const auto _draw_box_buffer = [&](size_t lane, size_t y)
	{
		constexpr auto _box_char = [](size_t i)->char { char temp[] = "��"; return temp[i]; };
		buffer[lane * 2 +     _buffer_size_x * y] = _box_char(0);
		buffer[lane * 2 + 1 + _buffer_size_x * y] = _box_char(1);
	};
	const auto _draw_voidbox_buffer = [&](size_t lane, size_t y)
	{
		constexpr auto _box_char = [](size_t i)->char { char temp[] = "��"; return temp[i]; };
		buffer[lane * 2 + _buffer_size_x * y] = _box_char(0);
		buffer[lane * 2 + 1 + _buffer_size_x * y] = _box_char(1);
	};
	// ���� ���� _end


	constexpr size_t _note_down_speed = 4;	// 60bpm ���� �ʴ� �������� ĭ�� ��
	Time_t unit = Time_t(1s) * 60 / long long(m_speedBpm * _note_down_speed);

	for (size_t lane = 0; lane < 4; lane++)
	{
		const auto& remainNoteList = m_lane[lane]->data();
		for (auto iter = remainNoteList.cbegin(); iter != remainNoteList.cend(); ++iter)
		{
			Lane::time_t noteTime = *iter;
			Lane::time_t remainTime = (noteTime + unit / 2) - timing;
			auto notePos_uncalcul = remainTime / unit;
			auto notePos = 32 - (notePos_uncalcul + 1);

			if (notePos > 32)	continue;
			if (notePos < 0)	break;

			notePos == 31 ?
			_draw_box_buffer(lane, notePos):
			_draw_voidbox_buffer(lane, notePos);
		}
	}

	gotoxy(0, 0);
	std::cout << buffer;
}
