#include "pch.h"
#include "BNS_Editor.h"

constexpr size_t _beat_per_bar = 48;

// My Custom
constexpr size_t _noteScn_width = 320;
constexpr size_t _noteScn_height = 1000;
constexpr int _noteScn_position_x = 1600 - _noteScn_width;
constexpr int _noteScn_position_y = 1000 - _noteScn_height;
constexpr size_t _noteScn_laneWidth = 72;
constexpr size_t _noteScn_barCount = 2;
constexpr size_t _noteScn_barHeight = 480;

// Auto Custom
constexpr size_t _noteScn_offsetWidth = (_noteScn_width - (_noteScn_laneWidth * 4)) / 2;
constexpr size_t _noteScn_offsetHeight = (_noteScn_height - (_noteScn_barHeight * _noteScn_barCount)) / 2;
constexpr size_t _noteScn_beatHeight = _noteScn_barHeight / _beat_per_bar;

BNS_Editor::BNS_Editor()
	: m_bpm(180)
	, m_beatPerBar(_beat_per_bar)
	, m_beatNote()

	, m_noteCursor(0)

	, m_renderTarget()
{
	for (auto& bnList : m_beatNote)
		bnList.resize(160 * m_beatPerBar, false);
}

BNS_Editor::~BNS_Editor()
{
}



bool BNS_Editor::Initialize()
{
	CreateNoteScreenGrid();
	CreateNoteScreen();
	return true;
}

void BNS_Editor::Update()
{
	int lane = 0, beat = 0;
	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
		if (ScreenToNote(lane, beat, g_inputDevice.MousePos()))
			m_beatNote[lane][beat] = !m_beatNote[lane][beat];

	_test_update();
}

void BNS_Editor::_test_update()
{
	if (g_inputDevice.IsKeyPressed(VK_SPACE))
		++m_noteCursor;

	CreateNoteScreen();
}

void BNS_Editor::Render()
{
	int lane = 0, beat = 0;
	if (ScreenToNote(lane, beat, g_inputDevice.MousePos()))
	{
		constexpr COLORREF _color_noteEdge = RGB(255, 0, 0);
		constexpr COLORREF _color_note = RGB(0, 255, 0);
		WNDRD.DrawOnCustomBitmap(_get_rt(RTName::NoteScreen));
		WNDRD.SetPenColor(_color_noteEdge);
		WNDRD.SetBrushColor(_color_note);
		[&](int lane, int beat)
		{
			int width = _noteScn_laneWidth - 1;
			int height = 10;
			int x = _noteScn_offsetWidth + (_noteScn_laneWidth * lane) + 1;
			int y = _noteScn_height - (_noteScn_offsetHeight + (_noteScn_beatHeight * beat) - m_noteCursor + height);
			WNDRD.DrawBox(x, y, width, height);
		}(lane, beat);
		WNDRD.UnlinkCustomBitmap();
	}

	DrawNoteScreenGrid();
	DrawNoteScreen();
}

void BNS_Editor::Release()
{
	for (auto& rt : m_renderTarget)
		rt.Release();
}



void BNS_Editor::InjectBNS(const BeatNoteSheet& bns)
{
	m_bpm = bns.m_bpm;
	m_beatNote = bns.m_beatData;
	CreateNoteScreen();
}



bool BNS_Editor::ScreenToNote(int& out_lane, int& out_beat, POINT pos)
{
	int lane = ((pos.x - _noteScn_position_x) - _noteScn_offsetWidth) / _noteScn_laneWidth;
	int beat = ((_noteScn_height - (pos.y - _noteScn_position_y + 1)) - _noteScn_offsetHeight + m_noteCursor) / _noteScn_beatHeight;
	if (lane < 0 || 4 <= lane ||
		beat < 0 || m_beatNote[lane].size() <= beat)
		return false;

	out_lane = lane;
	out_beat = beat;
	return true;
}



bool BNS_Editor::CreateNoteScreenGrid()
{
	constexpr COLORREF _color_background = RGB(0, 0, 0);
	constexpr COLORREF _color_laneedge = RGB(255, 255, 255);
	constexpr COLORREF _color_laneborder = RGB(127, 127, 127);
	constexpr COLORREF _color_baredge = RGB(127, 127, 127);
	constexpr COLORREF _color_bargrid = RGB(63, 63, 63);

	auto& rendertarget = _get_rt(RTName::NoteScreenGrid);
	if (!rendertarget.Created())
		rendertarget.Create(WNDRD, _noteScn_width, _noteScn_height);


	WNDRD.DrawOnCustomBitmap(rendertarget);
	WNDRD.SetBrushColor(_color_background);
	WNDRD.Fill(0, 0, _noteScn_width, _noteScn_height);

	auto xLineDraw = [](int y) { y = _noteScn_height - y; WNDRD.SingleLine(_noteScn_offsetWidth, y, _noteScn_offsetWidth + _noteScn_laneWidth * 4, y); };
	WNDRD.SetPenColor(_color_baredge);
	for (size_t bar = 0; bar <= _noteScn_barCount; ++bar)
		xLineDraw(_noteScn_offsetHeight + _noteScn_barHeight * bar);
	WNDRD.SetPenColor(_color_bargrid);
	for (size_t bar = 0; bar <= _noteScn_barCount; ++bar)
		for (size_t beat = 1; beat < 8; ++beat)
			xLineDraw(_noteScn_offsetHeight + _noteScn_barHeight * bar + (_noteScn_barHeight / 8) * beat);


	auto yLineDraw = [](int x) { WNDRD.SingleLine(x, 0, x, _noteScn_height); };
	WNDRD.SetPenColor(_color_laneborder);
	for (size_t lane = 1; lane < 4; ++lane)
		yLineDraw(_noteScn_offsetWidth + _noteScn_laneWidth * lane);
	WNDRD.SetPenColor(_color_laneedge);
	yLineDraw(_noteScn_offsetWidth);
	yLineDraw(_noteScn_offsetWidth + _noteScn_laneWidth * 4);

	WNDRD.UnlinkCustomBitmap();
	return true;
}

bool BNS_Editor::CreateNoteScreen()
{
	constexpr size_t _note_height = 10;
	constexpr COLORREF _color_noteEdge = RGB(191, 191, 191);
	constexpr COLORREF _color_note = RGB(255, 255, 255);
	constexpr COLORREF _color_judLine = RGB(0, 255, 255);

	auto& rendertarget = _get_rt(RTName::NoteScreen);
	if (!rendertarget.Created())
		rendertarget.Create(WNDRD, _noteScn_width, _noteScn_height);

	WNDRD.DrawOnCustomBitmap(rendertarget);
	WNDRD.SetBrushColor(WNDRD.DefaultTransparentColor());
	WNDRD.Fill(0, 0, rendertarget.Width(), rendertarget.Height());

	WNDRD.SetPenColor(_color_noteEdge);
	WNDRD.SetBrushColor(_color_note);
	
	auto _DrawNote = [&](int lane, int beat)
	{
		int width = _noteScn_laneWidth - 1;
		int height = _note_height;
		int x = _noteScn_offsetWidth + (_noteScn_laneWidth * lane) + 1;
		int y = _noteScn_height - (_noteScn_offsetHeight + (_noteScn_beatHeight * beat) - m_noteCursor + height);
		WNDRD.DrawBox(x, y, width, height);
	};

	for (size_t lane = 0; lane < 4; ++lane)
	{
		const auto& vec = m_beatNote[lane];
		size_t min;
		{
			int value = -((int)_noteScn_offsetHeight + (int)_note_height + (int)_noteScn_beatHeight - 1);
			value += m_noteCursor;
			min = std::max(0, value / (int)_noteScn_beatHeight);
		}
		size_t max;
		{
			int value = ((int)_noteScn_height - (int)_noteScn_offsetHeight + (int)_noteScn_beatHeight - 1);
			value += m_noteCursor;
			max = std::min((int)vec.size(), value / (int)_noteScn_beatHeight);
		}
		
		for (size_t index = min; index < max; ++index)
		{
			if (vec[index])
				_DrawNote(lane, index);
		}
	}


	WNDRD.SetPenColor(_color_judLine);
	int y = _noteScn_height - _noteScn_offsetHeight;
	WNDRD.SingleLine(0, y, _noteScn_width, y);

	WNDRD.UnlinkCustomBitmap();
	return true;
}


void BNS_Editor::DrawNoteScreenGrid()
{
	auto& rt = _get_rt(RTName::NoteScreenGrid);
	WNDRD.DrawOnMainBuffer();
	WNDRD.CopyTargetBitmap(rt);
	WNDRD.Copy(0, 0, _noteScn_position_x, _noteScn_position_y, _noteScn_width, _noteScn_height);
	WNDRD.UnlinkCopyTargetBitmap();
}

void BNS_Editor::DrawNoteScreen()
{
	auto& rt = _get_rt(RTName::NoteScreen);
	WNDRD.DrawOnMainBuffer();
	WNDRD.CopyTargetBitmap(rt);
	WNDRD.CopyTransparent(0, 0, _noteScn_position_x, _noteScn_position_y, _noteScn_width, _noteScn_height);
	WNDRD.UnlinkCopyTargetBitmap();
}



WNDRD_RenderTarget& BNS_Editor::_get_rt(RTName rtName)	{ return m_renderTarget[rtName]; }
