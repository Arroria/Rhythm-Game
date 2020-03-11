#include "pch.h"
#include "BNS_Editor.h"

#define IF_FAIL_RETURN_FALSE(_bool) { if (!_bool) return false; }

constexpr size_t _beat_per_bar = 48;

constexpr COLORREF _color_transparent = WNDRD.DefaultTransparentColor();


// note screen
constexpr int _noteScn_width = 320;
constexpr int _noteScn_height = 1000;
constexpr int _noteScn_position_x = 1600 - _noteScn_width;
constexpr int _noteScn_position_y = 0;

constexpr int _noteScn_laneWidth = 72;
constexpr int _noteScn_barHeight = 480;
constexpr int _noteScn_beatHeight = _noteScn_barHeight / _beat_per_bar;
constexpr int _noteScn_judgeLinePosition = 20;

constexpr int _noteScn_rtHeight = _noteScn_height + _noteScn_barHeight;
constexpr int _noteScn_sideWidth = (_noteScn_width - (_noteScn_laneWidth * 4)) / 2;
constexpr int _noteScn_subbarCount = 4;
constexpr int _noteScn_subbarHeight = _noteScn_barHeight / _noteScn_subbarCount;
constexpr int _noteScn_noteHeight = 8;

// preview screen
constexpr int _previewScn_width = 1000;
constexpr int _previewScn_height = 1000;
constexpr int _previewScn_position_x = 0;
constexpr int _previewScn_position_y = 0;

constexpr int _previewScn_laneWidth = 22;
constexpr int _previewScn_barHeight = 240;
constexpr int _previewScn_beatHeight = _previewScn_barHeight / _beat_per_bar;

constexpr int _previewScn_rtWidth = 100;
constexpr int _previewScn_barPerChunk = _previewScn_height / _previewScn_barHeight;
constexpr int _previewScn_sideWidth = (_previewScn_rtWidth - (_previewScn_laneWidth * 4)) / 2;
constexpr int _previewScn_sideHeight = (_previewScn_height - (_previewScn_barHeight * _previewScn_barPerChunk)) / 2;
constexpr int _previewScn_noteHeight = 4;

constexpr int _previewScn_scrollUnitPerChunk = 3;

// preview screen noteScn camera
constexpr int _previewScn_noteScn_camera_height = (float)_noteScn_height * _previewScn_barHeight / _noteScn_barHeight;
constexpr int _previewScn_noteScn_camera_judgementLine = (float)_noteScn_judgeLinePosition * _previewScn_barHeight / _noteScn_barHeight;




void BNS_Editor::Render()
{
	Render_noteScn();
	Render_previewScn();
	m_previewScn_scrolled = false;
	m_noteFixed = false;
	m_prevRenderedBarPosition = _editorCursor();
}



bool BNS_Editor::PreviewScreenScroll(int powerForRight)
{
	if (!powerForRight)
		return false;

	m_previewScn_scrolled = true;

	const auto previousScrollLevel = m_previewScn_scrollLevel;
	m_previewScn_scrollLevel = (powerForRight < 0) ?
		std::max<int>(_previewScn_minimum_scrollLevel(), m_previewScn_scrollLevel + powerForRight):
		std::min<int>(_previewScn_maximum_scrollLevel(), m_previewScn_scrollLevel + powerForRight);
	return previousScrollLevel != m_previewScn_scrollLevel;
}
bool BNS_Editor::PreviewScreenScrollLeftOnce() { return PreviewScreenScroll(-1); }
bool BNS_Editor::PreviewScreenScrollRightOnce() { return PreviewScreenScroll(1); }



bool BNS_Editor::MouseInNoteScreen(POINT mousePosition) const
{
	return
		_noteScn_position_x <= mousePosition.x &&
		_noteScn_position_y <= mousePosition.y &&
		mousePosition.x < _noteScn_position_x + _noteScn_width &&
		mousePosition.y < _noteScn_position_y + _noteScn_height;
}
bool BNS_Editor::MouseInPreviewScreen(POINT mousePosition) const
{
	return
		_previewScn_position_x <= mousePosition.x &&
		_previewScn_position_y <= mousePosition.y &&
		mousePosition.x < _previewScn_position_x + _previewScn_width &&
		mousePosition.y < _previewScn_position_y + _previewScn_height;
}

bool BNS_Editor::MouseToNoteScreenNotePosition(POINT mousePosition, int& out_laneIndex, int& out_beatIndex) const
{
	POINT pickedCoord = { mousePosition.x - _noteScn_position_x, _noteScn_height - (mousePosition.y - _noteScn_position_y) };
	
	// laneIndex
	int pickedLanePosition = pickedCoord.x - _noteScn_sideWidth;
	if (pickedLanePosition < 0 || 4 * _noteScn_laneWidth <= pickedLanePosition)
		return false;
	int laneIndex = pickedLanePosition / _noteScn_laneWidth;

	// beatIndex
	float judgeBarPosition = _editorCursor();
	float pickedBarPosition = (float)(pickedCoord.y - _noteScn_judgeLinePosition) / _noteScn_barHeight + judgeBarPosition;
	if (pickedBarPosition < 0)
		return false;

	int beatIndex = pickedBarPosition * _beat_per_bar;
	if (_maximum_beat() <= beatIndex)
		return false;

	// out
	out_laneIndex = laneIndex;
	out_beatIndex = beatIndex;
	return true;
}

bool BNS_Editor::MouseToPreviewScreenBarPositon(POINT mousePosition, float& out_barPostion) const
{
	POINT pickedCoord = { mousePosition.x - _previewScn_position_x, _previewScn_height - (mousePosition.y - _previewScn_position_y) };

	int pickedWidth = pickedCoord.x + _previewScn_scrolledWidth();
	int chunkIndex = pickedWidth / _previewScn_rtWidth;
	int xPositionInChunk = pickedWidth - (chunkIndex * _previewScn_rtWidth);

	if (xPositionInChunk < _previewScn_sideWidth ||
		_previewScn_sideWidth + _previewScn_laneWidth * 4 < xPositionInChunk)
		return false;
	if (chunkIndex < 0 || _previewScn_maximum_chunk() <= chunkIndex)
		return false;

	int pickedHeight = pickedCoord.y - _previewScn_sideHeight;
	float barPosInChunk = (float)pickedHeight / _previewScn_barHeight;

	out_barPostion = chunkIndex * _previewScn_barPerChunk + barPosInChunk;
	if (out_barPostion < 0)	out_barPostion = 0;
	return true;
}



void BNS_Editor::DrawRT_noteScn_grid()
{
	constexpr COLORREF _color_laneedge = RGB(255, 255, 255);
	constexpr COLORREF _color_laneborder = RGB(127, 127, 127);
	constexpr COLORREF _color_baredge = RGB(127, 127, 127);
	constexpr COLORREF _color_bargrid = RGB(63, 63, 63);

	auto& rendertarget = _get_rt(RTName::noteScn_grid);

	// RT가 없으면 생성, 생성이 안되면 셧다운
	if (!rendertarget.Created())
		if (!rendertarget.Create(WNDRD, _noteScn_width, _noteScn_rtHeight))
			std::terminate();

	auto LineDrawX = [](int yPos) { yPos = _noteScn_rtHeight - yPos - 1; WNDRD.DrawLine(_noteScn_sideWidth + 1, yPos, _noteScn_sideWidth + _noteScn_laneWidth * 4 - 1, yPos); };
	auto LineDrawY = [](int xPos) { WNDRD.DrawLine(xPos, _noteScn_rtHeight - 1, xPos, 0); };


	WNDRD.DrawOnRenderTarget(rendertarget);
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);

	// 투명 바탕
	WNDRD.SetBrushColor(_color_transparent);
	WNDRD.FillSize(0, 0, _noteScn_width, _noteScn_rtHeight);
	
	// 박자선
	// bar 단위로 선명한 선, subbar 단위로 흐린 선
	WNDRD.SetPenColor(_color_baredge);
	for (int currentY = _noteScn_judgeLinePosition; currentY < _noteScn_rtHeight; currentY += _noteScn_barHeight)
		LineDrawX(currentY);
	WNDRD.SetPenColor(_color_bargrid);
	for (int currentY = _noteScn_judgeLinePosition; currentY < _noteScn_rtHeight; currentY += _noteScn_subbarHeight)
		for (size_t subbarIndex = 1; subbarIndex < _noteScn_subbarCount; ++subbarIndex)
			LineDrawX(currentY += _noteScn_subbarHeight);
	
	// 라인 경계선
	WNDRD.SetPenColor(_color_laneborder);
	for (size_t lane = 1; lane < 4; ++lane)
		LineDrawY(_noteScn_sideWidth + _noteScn_laneWidth * lane);
	WNDRD.SetPenColor(_color_laneedge);
	LineDrawY(_noteScn_sideWidth);
	LineDrawY(_noteScn_sideWidth + _noteScn_laneWidth * 4);

	WNDRD.UnlinkRenderTarget();
}

void BNS_Editor::DrawRT_noteScn_note()
{
	constexpr int _temp_noteOffset = 1; // 오프셋
	constexpr COLORREF _color_noteEdge = RGB(191, 191, 191);
	constexpr COLORREF _color_note = RGB(255, 255, 255);

	auto& rendertarget = _get_rt(RTName::noteScn_note);

	// RT가 없으면 생성, 생성이 안되면 셧다운
	if (!rendertarget.Created())
		if (!rendertarget.Create(WNDRD, _noteScn_width, _noteScn_rtHeight))
			std::terminate();

	WNDRD.DrawOnRenderTarget(rendertarget);
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);

	// 투명 바탕
	WNDRD.SetBrushColor(_color_transparent);
	WNDRD.FillSize(0, 0, _noteScn_width, _noteScn_rtHeight);


	WNDRD.SetPenColor(_color_noteEdge);
	WNDRD.SetBrushColor(_color_note);
	auto NoteDraw_raw = [&](int xPos, int yPos)
	{
		yPos += (_noteScn_noteHeight - 1) + _temp_noteOffset;
		yPos = _noteScn_rtHeight - yPos - 1;
		WNDRD.DrawBoxSize(xPos, yPos, _noteScn_laneWidth - 1, _noteScn_noteHeight);
	};
	auto NoteDraw = [&NoteDraw_raw](int lane, int beatIndex)
	{
		int xPos = _noteScn_sideWidth + (_noteScn_laneWidth * lane) + 1;
		int yPos = _noteScn_judgeLinePosition + beatIndex * _noteScn_beatHeight;
		NoteDraw_raw(xPos, yPos);
	};


	// range check
	m_noteScn_barIndex = (int)_editorCursor();
	int beatRange_under = (m_noteScn_barIndex * _beat_per_bar) - (_noteScn_judgeLinePosition + _temp_noteOffset + _noteScn_noteHeight - 1) / _noteScn_beatHeight;
	int beatRange_upper = (m_noteScn_barIndex * _beat_per_bar) + (_noteScn_rtHeight - _noteScn_judgeLinePosition - _temp_noteOffset) / _noteScn_beatHeight;

	int beginBeat = std::max<int>(beatRange_under, 0);
	for (size_t lane = 0; lane < 4; ++lane)
	{
		const auto& vec = m_beatNote[lane];
		int endBeat = std::min<int>(beatRange_upper, vec.size() - 1);

		for (int index = beginBeat; index <= endBeat; ++index)
		{
			if (vec[index])
				NoteDraw(lane, index - m_noteScn_barIndex * _beat_per_bar);
		}
	}

	WNDRD.UnlinkRenderTarget();
}

void BNS_Editor::DrawRT_noteScn_master()
{
	constexpr COLORREF _color_background = RGB(0, 0, 0);
	constexpr COLORREF _color_judLine = RGB(0, 255, 255);

	auto& rendertarget = _get_rt(RTName::noteScn_master);
	auto& rt_grid = _get_rt(RTName::noteScn_grid);
	auto& rt_note = _get_rt(RTName::noteScn_note);

	// RT가 없으면 생성, 생성이 안되면 셧다운
	if (!rendertarget.Created())
		if (!rendertarget.Create(WNDRD, _noteScn_width, _noteScn_height))
			std::terminate();


	bool rt_drawed = false;
	
	// 노트화면 그리드 미생성시 생성
	if (!rt_grid.Created())
	{
		DrawRT_noteScn_grid();
		rt_drawed = true;
	}

	// 노트화면 노트를 생성 또는 재출력 필요시 재생성
	if (!rt_note.Created() || (int)_editorCursor() != m_noteScn_barIndex)
	{
		DrawRT_noteScn_note();
		rt_drawed = true;
	}

	// 이전과 동일한 화면이면 다시 그리지 않음
	if (!rt_drawed && m_prevRenderedBarPosition == _editorCursor() && !m_noteFixed)
		return;



	WNDRD.DrawOnRenderTarget(rendertarget);

	// 투명 바탕
	WNDRD.SetBrushColor(_color_background);
	WNDRD.SetBrushTransparent(false);
	WNDRD.FillSize(0, 0, _noteScn_width, _noteScn_height);

	// 라인, 노트
	float currentMusicBar = _editorCursor();
	int cursorOff = _noteScn_barHeight * (currentMusicBar - (int)currentMusicBar);
	int copyPos = _noteScn_rtHeight - _noteScn_height - cursorOff;

	WNDRD.LinkCopyRenderTarget(rt_grid);
	WNDRD.CopyTransparentSize(0, copyPos, _noteScn_width, _noteScn_height, 0, 0, _color_transparent);
	WNDRD.LinkCopyRenderTarget(rt_note);
	WNDRD.CopyTransparentSize(0, copyPos, _noteScn_width, _noteScn_height, 0, 0, _color_transparent);

	// 판정선
	WNDRD.SetPenColor(_color_judLine);
	int judgeLinePos = _noteScn_height - _noteScn_judgeLinePosition - 1;
	WNDRD.DrawLine(0, judgeLinePos, _noteScn_width - 1, judgeLinePos);


	WNDRD.UnlinkCopyRenderTarget();
	WNDRD.UnlinkRenderTarget();
}

void BNS_Editor::Render_noteScn()
{
	DrawRT_noteScn_master();
	
	WNDRD.DrawOnScreen();

	WNDRD.LinkCopyRenderTarget(_get_rt(RTName::noteScn_master));
	WNDRD.CopyTransparentSize(0, 0, _noteScn_width, _noteScn_height, _noteScn_position_x, _noteScn_position_y, _color_transparent);
	WNDRD.UnlinkCopyRenderTarget();
}



void BNS_Editor::DrawRT_previewScn_grid()
{
	constexpr COLORREF _color_laneedge = RGB(255, 255, 255);
	constexpr COLORREF _color_laneborder = RGB(127, 127, 127);
	constexpr COLORREF _color_baredge = RGB(127, 127, 127);
	//constexpr COLORREF _color_bargrid = RGB(63, 63, 63);

	auto& rendertarget = _get_rt(RTName::previewScn_grid);

	// RT가 없으면 생성, 생성이 안되면 셧다운
	if (!rendertarget.Created())
		if (!rendertarget.Create(WNDRD, _previewScn_rtWidth, _previewScn_height))
			std::terminate();

	auto LineDrawX = [](int yPos) { yPos = _previewScn_height - yPos - 1; WNDRD.DrawLine(_previewScn_sideWidth + 1, yPos, _previewScn_sideWidth + _previewScn_laneWidth * 4 - 1, yPos); };
	auto LineDrawY = [](int xPos) { WNDRD.DrawLine(xPos, _previewScn_height - 1, xPos, 0); };


	WNDRD.DrawOnRenderTarget(rendertarget);
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);

	// 투명 바탕
	WNDRD.SetBrushColor(_color_transparent);
	WNDRD.FillSize(0, 0, _previewScn_rtWidth, _previewScn_height);

	// 박자선
	WNDRD.SetPenColor(_color_baredge);
	for (int currentY = _previewScn_sideHeight; currentY < _previewScn_height; currentY += _previewScn_barHeight)
		LineDrawX(currentY);

	// 라인 경계선
	WNDRD.SetPenColor(_color_laneborder);
	for (size_t lane = 1; lane < 4; ++lane)
		LineDrawY(_previewScn_sideWidth + _previewScn_laneWidth * lane);
	WNDRD.SetPenColor(_color_laneedge);
	LineDrawY(_previewScn_sideWidth);
	LineDrawY(_previewScn_sideWidth + _previewScn_laneWidth * 4);

	WNDRD.UnlinkRenderTarget();
}

void BNS_Editor::DrawRT_previewScn_note()
{
	constexpr int _temp_noteOffset = 1; // 오프셋
	constexpr COLORREF _color_noteEdge = RGB(191, 191, 191);
	constexpr COLORREF _color_note = RGB(255, 255, 255);

	auto& rendertarget = _get_rt(RTName::previewScn_note);

	// RT가 없으면 생성, 생성이 안되면 셧다운
	if (!rendertarget.Created())
		if (!rendertarget.Create(WNDRD, _previewScn_rtWidth * _previewScn_maximum_chunk(), _previewScn_height))
			std::terminate();

	WNDRD.DrawOnRenderTarget(rendertarget);
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);

	// 투명 바탕
	WNDRD.SetBrushColor(WNDRD.DefaultTransparentColor());
	WNDRD.FillSize(0, 0, rendertarget.Width(), rendertarget.Height());

	// 노트
	WNDRD.SetPenColor(_color_noteEdge);
	WNDRD.SetBrushColor(_color_note);

	auto _DrawNote = [&](int chunk, int lane, int beatlevel)
	{
		int width = _previewScn_laneWidth - 1;
		int height = _previewScn_noteHeight;
		int x = (_previewScn_rtWidth * chunk) + (_previewScn_sideWidth + (_previewScn_laneWidth * lane) + 1);
		int y = _previewScn_height - (_previewScn_sideHeight + (_previewScn_beatHeight * beatlevel) + height);
		WNDRD.DrawBoxSize(x, y, width, height);
	};

	for (size_t lane = 0; lane < 4; ++lane)
	{
		const auto& vec = m_beatNote[lane];
		int chunkUnit = (_previewScn_height - _previewScn_sideHeight * 2) / _previewScn_beatHeight;
		for (size_t index = 0; index < vec.size(); ++index)
		{
			if (vec[index])
				_DrawNote(index / chunkUnit, lane, index % chunkUnit);
		}
	}

	WNDRD.UnlinkRenderTarget();
}

void BNS_Editor::DrawRT_previewScn_master()
{
	constexpr COLORREF _color_background = RGB(0, 0, 0);

	auto& rendertarget = _get_rt(RTName::previewScn_master);
	auto& rt_grid = _get_rt(RTName::previewScn_grid);
	auto& rt_note = _get_rt(RTName::previewScn_note);

	// RT가 없으면 생성, 생성이 안되면 셧다운
	if (!rendertarget.Created())
		if (!rendertarget.Create(WNDRD, _previewScn_width, _previewScn_height))
			std::terminate();


	bool rt_drawed = false;

	// 노트화면 그리드 미생성시 생성
	if (!rt_grid.Created())
	{
		DrawRT_previewScn_grid();
		rt_drawed = true;
	}

	// 노트화면 노트를 생성
	if (!rt_note.Created())
	{
		DrawRT_previewScn_note();
		rt_drawed = true;
	}

	// 동일한 화면이면 다시 그리지 않음
	if (!rt_drawed && !m_noteFixed && !m_previewScn_scrolled)
		return;


	WNDRD.DrawOnRenderTarget(rendertarget);

	// 투명 바탕
	WNDRD.SetBrushColor(_color_background);
	WNDRD.SetBrushTransparent(false);
	WNDRD.FillSize(0, 0, _previewScn_width, _previewScn_height);

	//라인, 노트
	int scrolledWidth = _previewScn_scrolledWidth();

	WNDRD.LinkCopyRenderTarget(rt_grid);
	int maximumChunk = _previewScn_maximum_chunk();
	for (size_t chunkIndex = 0; chunkIndex < maximumChunk; chunkIndex++)
	{
		int position = _previewScn_rtWidth * chunkIndex - scrolledWidth;
		if (position <= -_previewScn_rtWidth) continue;
		if (_previewScn_width <= position) break;
		WNDRD.CopyTransparentSize(0, 0, _previewScn_rtWidth, _previewScn_height, position, 0, _color_transparent);
	}
	{
		int copyPos = scrolledWidth;
		int copyWidth = std::min<int>(_previewScn_width, rt_note.Width() - scrolledWidth);
		int pastePos = std::max(-scrolledWidth, 0);
		WNDRD.LinkCopyRenderTarget(rt_note);
		WNDRD.CopyTransparentSize(copyPos, 0, copyWidth, _previewScn_height, pastePos, 0, _color_transparent);
	}


	WNDRD.UnlinkCopyRenderTarget();
	WNDRD.UnlinkRenderTarget();
}

void BNS_Editor::DrawRT_previewScn_noteScnCamera()
{
	constexpr COLORREF _color_edge = RGB(255, 0, 0);
	auto& rendertarget = _get_rt(RTName::previewScn_noteScnCamera);

	int width = _previewScn_laneWidth * 4 + 3;
	int height = _previewScn_noteScn_camera_height;
	int position = (_previewScn_rtWidth - width) / 2;
	int posY = -_previewScn_noteScn_camera_judgementLine;

	// RT가 없으면 생성, 생성이 안되면 셧다운
	if (!rendertarget.Created())
		if (!rendertarget.Create(WNDRD, _previewScn_rtWidth, height))
			std::terminate();

	WNDRD.DrawOnRenderTarget(rendertarget);
	WNDRD.SetPenTransparent(false);
	WNDRD.SetBrushTransparent(false);

	// 투명 바탕
	WNDRD.SetBrushColor(WNDRD.DefaultTransparentColor());
	WNDRD.FillSize(0, 0, rendertarget.Width(), rendertarget.Height());

	// 카메라
	WNDRD.SetPenColor(_color_edge);
	WNDRD.SetBrushTransparent(true);
	WNDRD.DrawBoxSize(position, 0, width, height);

	WNDRD.UnlinkRenderTarget();
}

void BNS_Editor::Render_previewScn()
{
	DrawRT_previewScn_master();

	WNDRD.DrawOnScreen();

	WNDRD.LinkCopyRenderTarget(_get_rt(RTName::previewScn_master));
	WNDRD.CopyTransparentSize(0, 0, _previewScn_width, _previewScn_height, _previewScn_position_x, _previewScn_position_y, _color_transparent);
	WNDRD.UnlinkCopyRenderTarget();

	// 렌더링 속도 문제로 별도 렌더링
	auto& rendertarget = _get_rt(RTName::previewScn_noteScnCamera);
	if (!rendertarget.Created())
		DrawRT_previewScn_noteScnCamera();

	WNDRD.LinkCopyRenderTarget(rendertarget);
	{
		constexpr float sideHeightWeight = (float)_previewScn_sideHeight / _previewScn_barHeight;
		float editorCursor = _editorCursor();
		int chunkIndex = std::max<float>(0, editorCursor - sideHeightWeight * 2) / _previewScn_barPerChunk;
		float chunkCursor = editorCursor - chunkIndex * _previewScn_barPerChunk;
		auto DrawNoteScnCamera = [this, &rendertarget](int chunkIndex, float chunkCursor)
		{
			int scrollLevel = m_previewScn_scrollLevel - chunkIndex * _previewScn_scrollUnitPerChunk;
			int positionX = _previewScn_rtWidth * (float)-scrollLevel / _previewScn_scrollUnitPerChunk;
			int positionY = (float)chunkCursor * _previewScn_barHeight + _previewScn_sideHeight - ((float)_noteScn_judgeLinePosition / _noteScn_barHeight * _previewScn_barHeight);

			if (0 <= positionX + rendertarget.Width() || positionX < _previewScn_width ||
				0 <= positionY + rendertarget.Height() || positionX < _previewScn_height)
			{
				int width = rendertarget.Width();
				int height = rendertarget.Height();
				positionY += height;
				positionY = (_previewScn_height - positionY);

				int copyPosX = 0;
				int copyPosY = 0;
				if (positionX < 0)
				{
					copyPosX = -positionX;
					width += positionX;
					positionX = 0;
				}
				else
					width = std::min<int>(width, _previewScn_width - positionX);
				if (positionY < 0)
				{
					copyPosY = -positionY;
					height += positionY;
					positionY = 0;
				}
				else
					height = std::min<int>(height, _previewScn_height - positionY);

				WNDRD.CopyTransparentSize(copyPosX, copyPosY, width, height, _previewScn_position_x + positionX, _previewScn_position_y + positionY, _color_transparent);
			}
		};
		DrawNoteScnCamera(chunkIndex, chunkCursor);
		DrawNoteScnCamera(chunkIndex + 1, chunkCursor - _previewScn_barPerChunk);
	}
	WNDRD.UnlinkCopyRenderTarget();
}


void BNS_Editor::FixRT_noteScn_note(int laneIndex, int beatIndex, bool active)
{
	constexpr int _temp_noteOffset = 1;
	constexpr COLORREF _color_noteEdge = RGB(191, 191, 191);
	constexpr COLORREF _color_note = RGB(255, 255, 255);

	auto& rendertarget = _get_rt(RTName::noteScn_note);
	if (!rendertarget.Created())
		return;

	beatIndex -= m_noteScn_barIndex * _beat_per_bar;

	int xPos = _noteScn_sideWidth + (_noteScn_laneWidth * laneIndex) + 1;
	int yPos = _noteScn_judgeLinePosition + beatIndex * _noteScn_beatHeight;
	yPos += (_noteScn_noteHeight - 1) + _temp_noteOffset;
	yPos = _noteScn_rtHeight - yPos - 1;

	WNDRD.DrawOnRenderTarget(rendertarget);
	if (active)
	{
		WNDRD.SetPenTransparent(false);
		WNDRD.SetBrushTransparent(false);
		WNDRD.SetPenColor(_color_noteEdge);
		WNDRD.SetBrushColor(_color_note);
		WNDRD.DrawBoxSize(xPos, yPos, _noteScn_laneWidth - 1, _noteScn_noteHeight);
	}
	else
	{
		WNDRD.SetBrushTransparent(false);
		WNDRD.SetBrushColor(_color_transparent);
		WNDRD.FillSize(xPos, yPos, _noteScn_laneWidth - 1, _noteScn_noteHeight);
	}
}

void BNS_Editor::FixRT_previewScn_note(int laneIndex, int beatIndex, bool active)
{
	constexpr int _temp_noteOffset = 1; // 오프셋
	constexpr COLORREF _color_noteEdge = RGB(191, 191, 191);
	constexpr COLORREF _color_note = RGB(255, 255, 255);

	auto& rendertarget = _get_rt(RTName::previewScn_note);
	if (!rendertarget.Created())
		return;

	int chunkIndex = beatIndex / (_previewScn_barPerChunk * _beat_per_bar);
	int beatLevel = beatIndex - ((_previewScn_barPerChunk * _beat_per_bar) * chunkIndex);

	int width = _previewScn_laneWidth - 1;
	int height = _previewScn_noteHeight;
	int x = (_previewScn_rtWidth * chunkIndex) + (_previewScn_sideWidth + (_previewScn_laneWidth * laneIndex) + 1);
	int y = _previewScn_height - (_previewScn_sideHeight + (_previewScn_beatHeight * beatLevel) + height);

	WNDRD.DrawOnRenderTarget(rendertarget);
	if (active)
	{
		WNDRD.SetPenTransparent(false);
		WNDRD.SetBrushTransparent(false);
		WNDRD.SetPenColor(_color_noteEdge);
		WNDRD.SetBrushColor(_color_note);
		WNDRD.DrawBoxSize(x, y, width, height);
	}
	else
	{
		WNDRD.SetBrushTransparent(false);
		WNDRD.SetBrushColor(_color_transparent);
		WNDRD.FillSize(x, y, width, height);
	}
}



WNDRD_RenderTarget& BNS_Editor::_get_rt(RTName rtName) { return m_renderTarget[rtName]; }

int BNS_Editor::_previewScn_maximum_chunk() const { return (_maximum_bar() + _previewScn_barPerChunk - 1) / _previewScn_barPerChunk; }
int BNS_Editor::_previewScn_minimum_scrollLevel() const { return -1; }
int BNS_Editor::_previewScn_maximum_scrollLevel() const { return std::max<int>(0, _previewScn_maximum_chunk() - (_previewScn_width / _previewScn_rtWidth)) * _previewScn_scrollUnitPerChunk + 1; }
int BNS_Editor::_previewScn_scrolledWidth() const	{ return m_previewScn_scrollLevel * _previewScn_rtWidth / _previewScn_scrollUnitPerChunk; }
