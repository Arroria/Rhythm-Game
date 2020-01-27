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

constexpr int _previewScn_count = 8;
constexpr int _previewScn_width = 100;
constexpr int _previewScn_height = 1000;
constexpr int _previewScn_laneWidth = 22;
constexpr int _previewScn_barCount = 4;
constexpr int _previewScn_barHeight = 240;

constexpr int _previewScn_scrollDenominator = 3;


// Auto Custom
constexpr size_t _noteScn_offsetWidth = (_noteScn_width - (_noteScn_laneWidth * 4)) / 2;
constexpr size_t _noteScn_offsetHeight = (_noteScn_height - (_noteScn_barHeight * _noteScn_barCount)) / 2;
constexpr size_t _noteScn_beatHeight = _noteScn_barHeight / _beat_per_bar;

constexpr size_t _previewScn_widthEdge = (_previewScn_width - (_previewScn_laneWidth * 4)) / 2;
constexpr size_t _previewScn_heightEdge = (_previewScn_height - (_previewScn_barHeight * _previewScn_barCount)) / 2;
constexpr size_t _previewScn_beatHeight = _previewScn_barHeight / _beat_per_bar;



BNS_Editor::BNS_Editor()
	: m_bpm(180)
	, m_beatPerBar(_beat_per_bar)
	, m_beatNote()

	, m_noteCursor(0)
	, m_previewScroll(0)

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
	DrawNoteScreenGrid();
	DrawNoteScreen();
	DrawPreviewScreenGreed();
	DrawPreviewScreen();
	return true;
}

void BNS_Editor::Update()
{
	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
	{
		int lane = 0, beat = 0;
		if (ScreenToNote(lane, beat, g_inputDevice.MousePos()))
		{
			m_beatNote[lane][beat] = !m_beatNote[lane][beat];
			DrawNoteScreen();
			DrawPreviewScreen();
		}

		int musicCursor = 0;
		if (ScreenToMusicCursor(musicCursor, g_inputDevice.MousePos()))
		{
			m_noteCursor = musicCursor;
			DrawNoteScreen();
		}
	}

	if (int wheel = (g_inputDevice.MouseWheel() / 120); wheel)
	{
		bool right = 0 < wheel;
		if (!right)
			wheel = -wheel;
		while (wheel--)
			right ? PreScnScrollRight() : PreScnScrollLeft();
	}

	_test_update();
}

void BNS_Editor::_test_update()
{
	static std::chrono::steady_clock::time_point tp_old(std::chrono::steady_clock::now());
	std::chrono::steady_clock::time_point tp_now(std::chrono::steady_clock::now());
	
	if (g_inputDevice.IsKeyPressed(VK_SPACE))
	{
		m_noteCursor += std::chrono::duration_cast<std::chrono::milliseconds>(tp_now - tp_old).count();
		DrawNoteScreen();
	}

	tp_old = tp_now;
}

void BNS_Editor::Render()
{
	RenderPreviewScreenGreed();
	RenderPreviewScreen();
	RenderPreviewScreenCursor();

	RenderNoteScreenGrid();
	RenderNoteScreen();
	RenderNoteScreenCursor();
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
	DrawNoteScreen();
}



int BNS_Editor::MaxBeatCount() const
{
	int maxBeat = 0;
	for (const auto& vec : m_beatNote)
		maxBeat = std::max<int>(vec.size(), maxBeat);
	return maxBeat;
}
int BNS_Editor::MaxBarCount() const			{ return (MaxBeatCount() + _beat_per_bar - 1) / _beat_per_bar; }
int BNS_Editor::PreScnMaxChunkCount() const	{ return (MaxBarCount() + _previewScn_barCount - 1) / _previewScn_barCount; }



void BNS_Editor::PreScnScrollLeft()			{ if (m_previewScroll) --m_previewScroll; }
void BNS_Editor::PreScnScrollRight()		{ if (m_previewScroll < PreScnScrollMax()) ++m_previewScroll; }
int  BNS_Editor::PreScnScrollMax() const	{ return std::max(0, (PreScnMaxChunkCount() - _previewScn_count) * _previewScn_scrollDenominator); }



bool BNS_Editor::ScreenToNote(int& out_lane, int& out_beat, POINT pos)
{
	int lane = ((pos.x - _noteScn_position_x) - _noteScn_offsetWidth) / _noteScn_laneWidth;
	int beat = ((_noteScn_height - (pos.y - _noteScn_position_y + 1)) - _noteScn_offsetHeight + (NoteCursorToBar() * _noteScn_barHeight)) / _noteScn_beatHeight;
	if (lane < 0 || 4 <= lane ||
		beat < 0 || m_beatNote[lane].size() <= beat)
		return false;

	out_lane = lane;
	out_beat = beat;
	return true;
}

bool BNS_Editor::ScreenToMusicCursor(int& out_musicCursor, POINT pos)
{
	if (pos.x < 0 || _previewScn_width * _previewScn_count <= pos.x ||
		pos.y < 0 || _previewScn_height <= pos.y)
		return false;

	float chunkBar = (float)((_previewScn_height - pos.y) - (int)_previewScn_heightEdge) / _previewScn_barHeight - 1;
	int chunk = ((float)pos.x / _previewScn_width) + ((float)m_previewScroll / _previewScn_scrollDenominator);
	out_musicCursor = std::max<int>(0, (chunk * _previewScn_barCount + chunkBar) * 1000 * 60 * 4 / m_bpm);
	return true;
}



bool BNS_Editor::DrawNoteScreenGrid()
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
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);
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

bool BNS_Editor::DrawNoteScreen()
{
	constexpr size_t _note_height = 8;
	constexpr COLORREF _color_noteEdge = RGB(191, 191, 191);
	constexpr COLORREF _color_note = RGB(255, 255, 255);
	constexpr COLORREF _color_judLine = RGB(0, 255, 255);

	auto& rendertarget = _get_rt(RTName::NoteScreen);
	if (!rendertarget.Created())
		rendertarget.Create(WNDRD, _noteScn_width, _noteScn_height);

	WNDRD.DrawOnCustomBitmap(rendertarget);
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);
	WNDRD.SetBrushColor(WNDRD.DefaultTransparentColor());
	WNDRD.Fill(0, 0, _noteScn_width, _noteScn_height);

	WNDRD.SetPenColor(_color_noteEdge);
	WNDRD.SetBrushColor(_color_note);
	
	auto _DrawNote = [&](int lane, int beat)
	{
		int width = _noteScn_laneWidth - 1;
		int height = _note_height;
		int x = _noteScn_offsetWidth + (_noteScn_laneWidth * lane) + 1;
		int y = _noteScn_height - (_noteScn_offsetHeight + (_noteScn_beatHeight * beat) - (NoteCursorToBar() * _noteScn_barHeight) + height);
		WNDRD.DrawBox(x, y, width, height);
	};

	for (size_t lane = 0; lane < 4; ++lane)
	{
		const auto& vec = m_beatNote[lane];
		size_t min;
		{
			int value = -((int)_noteScn_offsetHeight + (int)_note_height + (int)_noteScn_beatHeight - 1);
			value += (NoteCursorToBar() * _noteScn_barHeight);
			min = std::max(0, value / (int)_noteScn_beatHeight);
		}
		size_t max;
		{
			int value = ((int)_noteScn_height - (int)_noteScn_offsetHeight + (int)_noteScn_beatHeight - 1);
			value += (NoteCursorToBar() * _noteScn_barHeight);
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

bool BNS_Editor::DrawPreviewScreenGreed()
{
	constexpr COLORREF _color_background = RGB(0, 0, 0);
	constexpr COLORREF _color_laneedge = RGB(255, 255, 255);
	constexpr COLORREF _color_laneborder = RGB(127, 127, 127);
	constexpr COLORREF _color_baredge = RGB(127, 127, 127);
	constexpr COLORREF _color_bargrid = RGB(63, 63, 63);

	auto& rendertarget = _get_rt(RTName::PreviewScreenGrid);
	if (!rendertarget.Created())
		rendertarget.Create(WNDRD, _previewScn_width, _previewScn_height);
	
	WNDRD.DrawOnCustomBitmap(rendertarget);
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);
	WNDRD.SetBrushColor(_color_background);
	WNDRD.Fill(0, 0, _previewScn_width, _previewScn_height);

	auto xLineDraw = [](int y) { y = _previewScn_height - y; WNDRD.SingleLine(_previewScn_widthEdge, y, _previewScn_widthEdge + _previewScn_laneWidth * 4, y); };
	WNDRD.SetPenColor(_color_baredge);
	for (size_t bar = 0; bar <= _previewScn_barCount; ++bar)
		xLineDraw(_previewScn_heightEdge + _previewScn_barHeight * bar);
	WNDRD.SetPenColor(_color_bargrid);
	for (size_t bar = 0; bar <= _previewScn_barCount; ++bar)
		for (size_t beat = 1; beat < 4; ++beat)
			xLineDraw(_previewScn_heightEdge + _previewScn_barHeight * bar + (_previewScn_barHeight / 4) * beat);


	auto yLineDraw = [](int x) { WNDRD.SingleLine(x, 0, x, _previewScn_height); };
	WNDRD.SetPenColor(_color_laneborder);
	for (size_t lane = 1; lane < 4; ++lane)
		yLineDraw(_previewScn_widthEdge + _previewScn_laneWidth * lane);
	WNDRD.SetPenColor(_color_laneedge);
	yLineDraw(_previewScn_widthEdge);
	yLineDraw(_previewScn_widthEdge + _previewScn_laneWidth * 4);

	WNDRD.UnlinkCustomBitmap();
	return true;
}

bool BNS_Editor::DrawPreviewScreen()
{
	constexpr size_t _note_height = 4;
	constexpr COLORREF _color_noteEdge = RGB(191, 191, 191);
	constexpr COLORREF _color_note = RGB(255, 255, 255);

	auto& rendertarget = _get_rt(RTName::PreviewScreen);
	if (int chunkCount = PreScnMaxChunkCount(); !rendertarget.Created() || rendertarget.Width() != chunkCount * _previewScn_width)
		rendertarget.Create(WNDRD, chunkCount * _previewScn_width, _previewScn_height);

	WNDRD.DrawOnCustomBitmap(rendertarget);
	WNDRD.SetBrushColor(WNDRD.DefaultTransparentColor());
	WNDRD.Fill(0, 0, rendertarget.Width(), rendertarget.Height());

	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);
	WNDRD.SetPenColor(_color_noteEdge);
	WNDRD.SetBrushColor(_color_note);
	
	auto _DrawNote = [&](int chunk, int lane, int beatlevel)
	{
		int width = _previewScn_laneWidth - 1;
		int height = _note_height;
		int x = (_previewScn_width * chunk) + (_previewScn_widthEdge + (_previewScn_laneWidth * lane) + 1);
		int y = _previewScn_height - (_previewScn_heightEdge + (_previewScn_beatHeight * beatlevel) + height);
		WNDRD.DrawBox(x, y, width, height);
	};

	for (size_t lane = 0; lane < 4; ++lane)
	{
		const auto& vec = m_beatNote[lane];
		int chunkUnit = (_previewScn_height - _previewScn_heightEdge * 2) / _previewScn_beatHeight;
		for (size_t index = 0; index < vec.size(); ++index)
		{
			if (vec[index])
				_DrawNote(index / chunkUnit, lane, index % chunkUnit);
		}
	}

	WNDRD.UnlinkCustomBitmap();
	return true;
}



void BNS_Editor::RenderNoteScreenGrid()
{
	auto& rt = _get_rt(RTName::NoteScreenGrid);
	WNDRD.DrawOnMainBuffer();
	WNDRD.CopyTargetBitmap(rt);
	WNDRD.Copy(0, 0, _noteScn_position_x, _noteScn_position_y, _noteScn_width, _noteScn_height);
	WNDRD.UnlinkCopyTargetBitmap();
}

void BNS_Editor::RenderNoteScreen()
{
	auto& rt = _get_rt(RTName::NoteScreen);
	WNDRD.DrawOnMainBuffer();
	WNDRD.CopyTargetBitmap(rt);
	WNDRD.CopyTransparent(0, 0, _noteScn_position_x, _noteScn_position_y, _noteScn_width, _noteScn_height);
	WNDRD.UnlinkCopyTargetBitmap();
}

void BNS_Editor::RenderNoteScreenCursor()
{
	int lane = 0, beat = 0;
	if (ScreenToNote(lane, beat, g_inputDevice.MousePos()))
	{
		constexpr COLORREF _color_noteEdge = RGB(255, 0, 0);
		constexpr COLORREF _color_note = RGB(0, 255, 0);
		WNDRD.DrawOnMainBuffer();
		WNDRD.SetPenTransparent(false);
		WNDRD.SetBrushTransparent(false);
		WNDRD.SetPenColor(_color_noteEdge);
		WNDRD.SetBrushColor(_color_note);
		[&](int lane, int beat)
		{
			int width = _noteScn_laneWidth - 1;
			int height = 10;
			int x = _noteScn_offsetWidth + (_noteScn_laneWidth * lane) + 1;
			int y = _noteScn_height - (_noteScn_offsetHeight + (_noteScn_beatHeight * beat) - (NoteCursorToBar() * _noteScn_barHeight) + height);
			WNDRD.DrawBox(_noteScn_position_x + x, _noteScn_position_y + y, width, height);
		}(lane, beat);
	}

}

void BNS_Editor::RenderPreviewScreenGreed()
{
	constexpr int _screen_limite = _previewScn_width * _previewScn_count;

	auto& rt = _get_rt(RTName::PreviewScreenGrid);
	WNDRD.DrawOnMainBuffer();
	WNDRD.SetBrushTransparent(false);
	WNDRD.SetBrushColor(RGB(0, 0, 0));
	WNDRD.Fill(0, 0, _previewScn_width * _previewScn_count, _previewScn_height);
	WNDRD.CopyTargetBitmap(rt);


	int remainBar = MaxBarCount() % _previewScn_barCount;
	int chunkCount = PreScnMaxChunkCount();
	for (int chunk = 0; chunk < chunkCount; chunk++)
	{
		int drawScroll = m_previewScroll * _previewScn_width / _previewScn_scrollDenominator;
		int drawPosition = (_previewScn_width * chunk) - drawScroll;
		int renderRange = std::min(_previewScn_width, _screen_limite - drawPosition);

		if (drawPosition + renderRange < 0)	continue;
		if (drawPosition >= _screen_limite)	break;

		if (!remainBar || chunk + 1 < chunkCount)
			WNDRD.Copy(0, 0, drawPosition, 0, renderRange, _previewScn_height);
		else
		{
			int removeRange = _previewScn_barHeight * remainBar;
			WNDRD.Copy(0, _previewScn_barHeight, drawPosition, _previewScn_barHeight, renderRange, _previewScn_height - _previewScn_barHeight);
		}
	}

	WNDRD.UnlinkCopyTargetBitmap();
}

void BNS_Editor::RenderPreviewScreen()
{
	auto& rt = _get_rt(RTName::PreviewScreen);
	WNDRD.DrawOnMainBuffer();
	WNDRD.CopyTargetBitmap(rt);
	WNDRD.CopyTransparent(m_previewScroll * _previewScn_width / _previewScn_scrollDenominator, 0, 0, 0,
		_previewScn_width * _previewScn_count, _previewScn_height);
	WNDRD.UnlinkCopyTargetBitmap();
}

void BNS_Editor::RenderPreviewScreenCursor()
{
	WNDRD.DrawOnMainBuffer();

	int y_position = _previewScn_height - (_previewScn_heightEdge + (_previewScn_barHeight * 2) + (NoteCursorToBar() * _previewScn_barHeight));
	int chunk_level = 0;
	{
		while (y_position + _previewScn_barHeight * 2 < 0)
		{
			y_position += _previewScn_barHeight * 4;
			++chunk_level;
		}
	}
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(true);
	WNDRD.SetPenColor(RGB(255, 0, 0));
	auto _draw_box = [&](int y_pos, int chunk_level)
	{
		int x_pos = _previewScn_widthEdge - (m_previewScroll * _previewScn_width / _previewScn_scrollDenominator) + (chunk_level * _previewScn_width);
		int x_range = _previewScn_laneWidth * 4 + 1;
		int y_range = _previewScn_barHeight * 2 + 1;
		if (_previewScn_width * _previewScn_count <= x_pos + x_range)
			return;
		
		int heightOs = 0;
		if (int maxChunk = PreScnMaxChunkCount(); chunk_level + 1 == maxChunk)
		{
			if (int remainBar = MaxBarCount() % _previewScn_barCount; remainBar)
			{
				int y_pos_min = _previewScn_heightEdge + ((4 - remainBar) * _previewScn_barHeight);
				if (y_pos < y_pos_min)
				{
					heightOs = y_pos_min - y_pos;
					y_pos = y_pos_min;
				}
			}
		}
		if (y_range <= heightOs)
			return;
		WNDRD.DrawBox(x_pos, y_pos, x_range, y_range - heightOs);
	};
	_draw_box(y_position, chunk_level);
	_draw_box(y_position + _previewScn_barHeight * 4, chunk_level + 1);
}



float BNS_Editor::NoteCursorToBar() const	{ return (float)m_noteCursor * m_bpm / 60 / 1000 / 4; }



WNDRD_RenderTarget& BNS_Editor::_get_rt(RTName rtName)	{ return m_renderTarget[rtName]; }
