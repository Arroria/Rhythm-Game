#include "pch.h"
#include "SheetEditor_IO.h"


// Customizable
constexpr size_t _laneCount = 4;
constexpr size_t _subbarPerBar = 4;
constexpr size_t _beatPerBar = 48;

constexpr size_t _grid_screenWidth = 50;
constexpr size_t _grid_screenHeight = 1000;
constexpr size_t _grid_lineWidth = 40;
constexpr size_t _grid_lineHeight = 960;
constexpr size_t _grid_barPerLine = 4;
constexpr size_t _grid_note_height = 4;

constexpr size_t __editableScreen_screenWidth = 200;
constexpr size_t __editableScreen_screenHeight = 1000;
constexpr size_t __editableScreen_lineWidth = 180;
constexpr size_t __editableScreen_lineHeight = 960;
constexpr size_t __editableScreen_barPerLine = 2;
constexpr size_t __editableScreen_note_height = 8;

constexpr BYTE _brightnessOf_bar = 127;
constexpr BYTE _brightnessOf_subbar = 31;
constexpr BYTE _brightnessOf_laneEdge = 255;
constexpr BYTE _brightnessOf_laneGrid = 127;
constexpr BYTE _brightnessOf_pen = 127;
constexpr BYTE _brightnessOf_brush = 255;


constexpr float __editableScreen_widthScale = 4.0f;
constexpr float __editableScreen_heightScale = 0.5f;


constexpr size_t _preview_focusBox_barCount = __editableScreen_barPerLine;
constexpr COLORREF _preview_focusBox_color = RGB(255, 0, 0);
constexpr COLORREF _chromakeyColor = RGB(255, 0, 255);

// Uncustomizable
constexpr size_t __grid_lineEdgeWidth	= (_grid_screenWidth - _grid_lineWidth) / 2;
constexpr size_t __grid_lineEdgeHeight	= (_grid_screenHeight - _grid_lineHeight) / 2;
constexpr size_t __grid_barInterval		= _grid_lineHeight / _grid_barPerLine;
constexpr size_t __grid_subbarInterval	= __grid_barInterval / _subbarPerBar;
constexpr size_t __grid_beatInterval	= __grid_barInterval / _beatPerBar;
constexpr size_t __grid_laneInterval	= _grid_lineWidth / _laneCount;

constexpr size_t __editableScreen_lineEdgeWidth		= (__editableScreen_screenWidth - __editableScreen_lineWidth) / 2;
constexpr size_t __editableScreen_lineEdgeHeight	= (__editableScreen_screenHeight - __editableScreen_lineHeight) / 2;
constexpr size_t __editableScreen_barInterval		= __editableScreen_lineHeight / __editableScreen_barPerLine;
constexpr size_t __editableScreen_subbarInterval	= __editableScreen_barInterval / _subbarPerBar;
constexpr size_t __editableScreen_beatInterval		= __editableScreen_barInterval / _beatPerBar;
constexpr size_t __editableScreen_laneInterval		= __editableScreen_lineWidth / _laneCount;

constexpr auto __grayscaleToRGB = [](BYTE brightness)->COLORREF { return RGB(brightness, brightness, brightness); };

constexpr float __preview_focusBox_offset = _preview_focusBox_barCount / 2.f;



SheetEditor_IO::SheetEditor_IO()
	: m_editor(nullptr)
	, m_resultRt()
	, m_gridRt()
	, m_noteRt()

	, m_previewScreenScroll(NULL)
	, m_previewScreenRedraw(false)
	, m_focusBox_barPosition(NULL)

	, m_edisc_focusBox_laneIndex(NULL)
	, m_edisc_focusBox_beatIndex(NULL)
	, m_editableScreenRedraw(false)

	, m_beatForEdit(NULL)
	, m_bpmForEdit(NULL)
	, m_offsetTimeForEdit(m_offsetTimeForEdit.zero())


	, m_inMusicTesting(false)
	, m_music()
	, m_beatHitSound()
	, m_musicChannel()
{
}

SheetEditor_IO::~SheetEditor_IO()
{
}


#include <fstream>
#include <string>
void SheetEditor_IO::Initialize(size_t predictionMaxNode, size_t baseBeatPerBar)
{
	m_music = g_soundDevice.CreateSoundSample("../rhy_02/697873717765.mp3");
	//m_beatHitSound = g_soundDevice.CreateSoundSample("handclap.wav");
	m_beatHitSound = g_soundDevice.CreateSoundSample("tick.mp3");

	// Alloc Sheet editor
	m_editor = new SheetEditor();
	m_editor->Initialize(predictionMaxNode, baseBeatPerBar);

	// Alloc RenderTarget
	m_resultRt.Allocation(g_dc_window, 1600, 1000);
	m_gridRt.Allocation(g_dc_window, _grid_screenWidth, _grid_screenHeight);
	_resize_note_rt(predictionMaxNode / 4);

	m_editableScreenRt.Allocation(g_dc_window, 200 ,1000);

	// Draw grid
	_draw_grid();

	// Variable initalize
	m_previewScreenScroll = 0;
	m_previewScreenRedraw = true;
	m_focusBox_barPosition = 0;

	m_edisc_focusBox_laneIndex = _laneCount;
	m_edisc_focusBox_beatIndex = 0;
	m_editableScreenRedraw = true;

	m_beatForEdit = 16;
	m_bpmForEdit = 198;
	m_offsetTimeForEdit = m_offsetTimeForEdit.zero();
	m_offsetTimeForEdit = std::chrono::milliseconds(730);



	m_musicChannel = std::move(m_music.play());
	m_musicChannel.pause();

	// ÀÓ½Ã·Îµå
	std::ifstream file("697873717765.txt");
	while (!file.eof())
	{
		size_t lane, bar, beat;
		file >> lane >> bar >> beat;
		m_editor->AddNote(lane, bar, beat);
		_draw_note(lane, bar, beat, true);
	}
}

void SheetEditor_IO::Update()
{
	if (g_inputDevice.IsKeyDown(VK_OEM_4))	--m_bpmForEdit;
	if (g_inputDevice.IsKeyDown(VK_OEM_6))	++m_bpmForEdit;

	if (g_inputDevice.IsKeyDown(VK_OEM_COMMA))	m_offsetTimeForEdit -= std::chrono::milliseconds(10);
	if (g_inputDevice.IsKeyDown(VK_OEM_PERIOD))	m_offsetTimeForEdit += std::chrono::milliseconds(10);


	if (g_inputDevice.IsKeyDown(VK_SPACE))
	{
		m_inMusicTesting = !m_inMusicTesting;
		if (m_inMusicTesting)
		{
			using namespace std::chrono_literals;
			m_musicChannel.set_time(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(240s) / m_bpmForEdit * m_focusBox_barPosition) + m_offsetTimeForEdit);
			
			if (m_musicChannel.is_available())
				m_musicChannel.play();
			else
				m_musicChannel = m_music.play();

			m_edisc_focusBox_laneIndex = _laneCount;
		}
		else
		{
			m_musicChannel.pause();
		}
	}
		
	if (m_inMusicTesting)
		_update_musicTest();
	else
		_update_editable();
}

void SheetEditor_IO::Render()
{
	DC_RenderTarget& drawDC		= g_dc_rendertarget1;
	DC_RenderTarget& resultDC	= g_dc_rendertarget2;
	resultDC.SetRenderTarget(m_resultRt);

	// Preview Screen
	if (m_previewScreenRedraw)
	{
		// Clear
		DC_FillRect(resultDC, 0, 0, 800, 1000, (HBRUSH)GetStockObject(BLACK_BRUSH));

		// Grid
		drawDC.SetRenderTarget(m_gridRt);
		for (size_t i = 0; i < m_noteRt.size(); ++i)
		{
			int renderPos = i * _grid_screenWidth - m_previewScreenScroll;
			if (renderPos < -(int)_grid_screenWidth)	continue;
			if (renderPos > 800) break;

			resultDC.Paste(drawDC, 0, 0, renderPos, 0, std::min(800 - renderPos, 50), 1000);
		}
		for (size_t i = 0; i < m_noteRt.size(); ++i)
		{
			int renderPos = i * _grid_screenWidth - m_previewScreenScroll;
			if (renderPos < -(int)_grid_screenWidth)	continue;
			if (renderPos > 800) break;

			drawDC.SetRenderTarget(m_noteRt[i]);
			resultDC.PasteTransparent(drawDC, 0, 0, renderPos, 0, std::min(800 - renderPos, 50), 1000, _chromakeyColor);
		}

		// Focus box
		{
			constexpr float costEdge = (float)__grid_lineEdgeHeight / __grid_barInterval;
			constexpr size_t focusBoxHeight = _preview_focusBox_barCount * __grid_barInterval;

			size_t line = m_focusBox_barPosition / _grid_barPerLine;
			float barPos = m_focusBox_barPosition - line * _grid_barPerLine;

			int drawPosX = (line * _grid_screenWidth) + __grid_lineEdgeWidth - m_previewScreenScroll;
			int drawPosY = (int)_grid_screenHeight - ((barPos * __grid_barInterval) + __grid_lineEdgeHeight) - focusBoxHeight;
			int drawWidth = _grid_lineWidth + 1;
			int drawHeight = focusBoxHeight;

			resultDC.SetPenTransparent(false);
			resultDC.SetBrushTransparent(true);
			resultDC.SetPenColor(_preview_focusBox_color);

			resultDC.DrawRectBySize(drawPosX, drawPosY, drawWidth, drawHeight);
			if (line != 0 && barPos < costEdge)
				resultDC.DrawRectBySize(drawPosX - _grid_screenWidth, drawPosY - _grid_lineHeight, drawWidth, drawHeight);
			if (_preview_focusBox_barCount - costEdge < barPos)
				resultDC.DrawRectBySize(drawPosX + _grid_screenWidth, drawPosY + _grid_lineHeight, drawWidth, drawHeight);
		}
		DC_FillRect(resultDC, 801, 0, 1400 - 2, 1000, (HBRUSH)GetStockObject(BLACK_BRUSH)); // ¹ö±×µ¤±â
		m_previewScreenRedraw = false;
	}

	if (m_editableScreenRedraw)
	{
		drawDC.SetRenderTarget(m_editableScreenRt);
		drawDC.SetPenTransparent(false);
		DC_FillRect(drawDC, 0, 0, __editableScreen_screenWidth, __editableScreen_screenHeight, (HBRUSH)GetStockObject(BLACK_BRUSH));

		auto xAxisDraw = [&](size_t positionY) { drawDC.SingleLine(__editableScreen_lineEdgeWidth, positionY, __editableScreen_lineEdgeWidth + __editableScreen_lineWidth, positionY); };
		auto yAxisDraw = [&](size_t positionX) { drawDC.SingleLine(positionX, 0, positionX, __editableScreen_screenHeight); };

		// Draw X Axis
		float offset = m_focusBox_barPosition;
		if (offset < 0)
			offset += (size_t)(1 - offset);
		else
			offset -= (size_t)offset;

		drawDC.SetPenColor(__grayscaleToRGB(_brightnessOf_bar));
		for (size_t barIndex = 0; barIndex <= __editableScreen_barPerLine + 1; ++barIndex)
			xAxisDraw(__editableScreen_screenHeight - (((float)barIndex - offset) * __editableScreen_barInterval + __editableScreen_lineEdgeHeight));

		drawDC.SetPenColor(__grayscaleToRGB(_brightnessOf_subbar));
		for (size_t barIndex = 0; barIndex <= __editableScreen_barPerLine; ++barIndex)
			for (size_t subbarIndex = 1; subbarIndex < _subbarPerBar; ++subbarIndex)
				xAxisDraw(__editableScreen_screenHeight - ((((float)barIndex - offset) * __editableScreen_barInterval) + (subbarIndex * __editableScreen_subbarInterval) + __editableScreen_lineEdgeHeight));

		// Draw Y Axis
		drawDC.SetPenColor(__grayscaleToRGB(_brightnessOf_laneEdge));
		yAxisDraw(__editableScreen_lineEdgeWidth);
		yAxisDraw(__editableScreen_lineEdgeWidth + __editableScreen_lineWidth);

		drawDC.SetPenColor(__grayscaleToRGB(_brightnessOf_laneGrid));
		for (size_t laneIndex = 1; laneIndex < _laneCount; ++laneIndex)
			yAxisDraw(laneIndex * __editableScreen_laneInterval + __editableScreen_lineEdgeWidth);

		// Draw Note
		drawDC.SetPenTransparent(false);
		drawDC.SetBrushTransparent(false);
		drawDC.SetPenColor(_brightnessOf_pen, _brightnessOf_pen, _brightnessOf_pen);
		drawDC.SetBrushColor(_brightnessOf_brush, _brightnessOf_brush, _brightnessOf_brush);
		const auto& notedata = m_editor->__get_data();
		for (size_t laneIndex = 0; laneIndex < notedata.size(); ++laneIndex)
		{
			size_t drawPositionX = (laneIndex * __editableScreen_laneInterval) + __editableScreen_lineEdgeWidth + 1;
			const auto& laneNoteData = notedata[laneIndex];
			for (size_t noteIndex = 0; noteIndex < laneNoteData.size(); ++noteIndex)
			{
				const auto& note = laneNoteData[noteIndex];
				if (!note)
					continue;
		
				size_t barIndex = noteIndex / _beatPerBar;
				size_t beatIndex = noteIndex - (barIndex * _beatPerBar);
		
				size_t staticPosition = (__editableScreen_lineEdgeHeight) + (barIndex * __editableScreen_barInterval) + (beatIndex * __editableScreen_beatInterval);
				int drawPositionY = __editableScreen_screenHeight - (staticPosition - m_focusBox_barPosition * __editableScreen_barInterval);
		
				if (drawPositionY < 0) break;
				if (drawPositionY > _grid_screenHeight) continue;
		
				size_t drawSizeX = __editableScreen_laneInterval - 1;
				size_t drawSizeY = __editableScreen_note_height;

				drawDC.DrawRectBySize(drawPositionX, drawPositionY - __editableScreen_note_height, drawSizeX, __editableScreen_note_height);
			}
		}

		// Draw FocusBox
		if (m_edisc_focusBox_laneIndex != _laneCount)
		{
			size_t barIndex = m_edisc_focusBox_beatIndex / _beatPerBar;
			size_t beatIndex = m_edisc_focusBox_beatIndex - (barIndex * _beatPerBar);
			
			size_t staticPosition = (__editableScreen_lineEdgeHeight) + (barIndex * __editableScreen_barInterval) + (beatIndex * __editableScreen_beatInterval);

			size_t drawPositionX = (m_edisc_focusBox_laneIndex * __editableScreen_laneInterval) + __editableScreen_lineEdgeWidth + 1;
			int drawPositionY = __editableScreen_screenHeight - (staticPosition - m_focusBox_barPosition * __editableScreen_barInterval);
			size_t drawSizeX = __editableScreen_laneInterval - 1;
			size_t drawSizeY = __editableScreen_note_height;

			drawDC.SetPenTransparent(false);
			drawDC.SetBrushTransparent(true);
			drawDC.SetPenColor(255, 0, 0);
			drawDC.DrawRectBySize(drawPositionX, drawPositionY - __editableScreen_note_height, drawSizeX, __editableScreen_note_height);
		}

		// Paste at resultDC
		resultDC.Paste(drawDC, 0, 0, 1400 - 1, 0, 200, 1000);
		m_editableScreenRedraw = false;
	}

	// ¾Æ ¸¸µé±â ±ÍÂú´Ù
	resultDC.SetPenTransparent(false);
	resultDC.SetPenColor(127, 127, 127);
	RECT rc;

	rc = { 810, 10, 810, 10 };
	std::string beatStr = std::to_string(m_beatForEdit) + " beat   ";
	DrawTextA(resultDC.DeviceContextHandle(), (beatStr).data(), -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP);

	rc = { 810, 30, 810, 30 };
	std::string bpmStr = std::to_string(m_bpmForEdit) + " bpm   ";
	DrawTextA(resultDC.DeviceContextHandle(), (bpmStr).data(), -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP);

	rc = { 810, 50, 810, 50 };
	std::string offsetStr = std::to_string(m_offsetTimeForEdit.count()) + " offset                      ";
	DrawTextA(resultDC.DeviceContextHandle(), (offsetStr).data(), -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP);


	// :3
	drawDC.DetachRenderTarget();
	resultDC.DetachRenderTarget();
}

void SheetEditor_IO::Release()
{
	m_editableScreenRt.Release();

	_resize_note_rt(0);
	m_gridRt.Release();
	m_resultRt.Release();

	if (m_editor)
	{
		delete m_editor;
		m_editor = nullptr;
	}

	m_beatHitSound.release();
	m_music.release();
}



void SheetEditor_IO::_resize_note_rt(size_t size)
{
	size_t prevSize = m_noteRt.size();
	if (size == prevSize)
		return;

	m_noteRt.resize(size);
	if (size < prevSize)
		return;

	auto& dc = g_dc_rendertarget1;
	dc.SetPenTransparent(false);
	dc.SetBrushTransparent(false);
	dc.SetPenColor(_chromakeyColor);
	dc.SetBrushColor(_chromakeyColor);
	for (size_t index = prevSize; index < m_noteRt.size(); ++index)
	{
		m_noteRt[index].Allocation(g_dc_window, _grid_screenWidth, _grid_screenHeight);
		dc.SetRenderTarget(m_noteRt[index]);
		dc.DrawRectBySize(0, 0, _grid_screenWidth, _grid_screenHeight);
	}
}


void SheetEditor_IO::_draw_grid()
{
	auto& dc = g_dc_rendertarget1;
	dc.SetRenderTarget(m_gridRt);
	dc.SetPenTransparent(false);

	auto xAxisDraw = [&](size_t positionY) { dc.SingleLine(__grid_lineEdgeWidth, positionY, __grid_lineEdgeWidth + _grid_lineWidth, positionY); };
	auto yAxisDraw = [&](size_t positionX) { dc.SingleLine(positionX, 0, positionX, _grid_screenHeight); };

	// Draw X Axis
	dc.SetPenColor(__grayscaleToRGB(_brightnessOf_bar));
	for (size_t barIndex = 0; barIndex <= _grid_barPerLine; ++barIndex)
		xAxisDraw(_grid_screenHeight - (barIndex * __grid_barInterval + __grid_lineEdgeHeight));

	dc.SetPenColor(__grayscaleToRGB(_brightnessOf_subbar));
	for (size_t barIndex = 0; barIndex < _grid_barPerLine; ++barIndex)
		for (size_t subbarIndex = 1; subbarIndex < _subbarPerBar; ++subbarIndex)
			xAxisDraw(_grid_screenHeight - ((barIndex * __grid_barInterval) + (subbarIndex * __grid_subbarInterval) + __grid_lineEdgeHeight));

	// Draw Y Axis
	dc.SetPenColor(__grayscaleToRGB(_brightnessOf_laneEdge));
	yAxisDraw(__grid_lineEdgeWidth);
	yAxisDraw(__grid_lineEdgeWidth + _grid_lineWidth);

	dc.SetPenColor(__grayscaleToRGB(_brightnessOf_laneGrid));
	for (size_t laneIndex = 1; laneIndex < _laneCount; ++laneIndex)
		yAxisDraw(__grid_lineEdgeWidth + __grid_laneInterval * laneIndex);
}

void SheetEditor_IO::_draw_note(size_t laneIndex, size_t barIndex, size_t beatIndex, bool state)
{
	size_t lineIndex = barIndex / _grid_barPerLine;
	barIndex = barIndex - lineIndex * _grid_barPerLine;

	size_t x = (__grid_lineEdgeWidth) + (laneIndex * __grid_laneInterval);
	size_t y = _grid_screenHeight - ((__grid_lineEdgeHeight)+(barIndex * __grid_barInterval) + (beatIndex * __grid_beatInterval));

	auto& dc = g_dc_rendertarget1;
	dc.SetRenderTarget(m_noteRt[lineIndex]);
	dc.SetPenTransparent(false);
	dc.SetBrushTransparent(false);
	if (state)
	{
		dc.SetPenColor(_brightnessOf_pen, _brightnessOf_pen, _brightnessOf_pen);
		dc.SetBrushColor(_brightnessOf_brush, _brightnessOf_brush, _brightnessOf_brush);
	}
	else
	{
		dc.SetPenColor(_chromakeyColor);
		dc.SetBrushColor(_chromakeyColor);
	}
	dc.DrawRectBySize(x + 1, y - _grid_note_height, __grid_laneInterval - 1, _grid_note_height);
}



void SheetEditor_IO::_update_editable()
{
	const POINT mousePos = g_inputDevice.MousePos();
	const POINT mouseDelta = g_inputDevice.MouseDelta();
	
	// Editor Beat
	if (g_inputDevice.IsKeyDown('1')) m_beatForEdit = 1;
	if (g_inputDevice.IsKeyDown('2')) m_beatForEdit = 2;
	if (g_inputDevice.IsKeyDown('3')) m_beatForEdit = 3;
	if (g_inputDevice.IsKeyDown('4')) m_beatForEdit = 4;
	if (g_inputDevice.IsKeyDown('5')) m_beatForEdit = 6;
	if (g_inputDevice.IsKeyDown('6')) m_beatForEdit = 8;
	if (g_inputDevice.IsKeyDown('7')) m_beatForEdit = 12;
	if (g_inputDevice.IsKeyDown('8')) m_beatForEdit = 16;
	if (g_inputDevice.IsKeyDown('9')) m_beatForEdit = 24;
	if (g_inputDevice.IsKeyDown('0')) m_beatForEdit = 48;


	// Preview Screen Update
	if (0 <= mousePos.x && mousePos.x <= 800 &&
		0 <= mousePos.y && mousePos.y <= 1000)
	{
		const POINT uiMousePos = mousePos;
		if ((mouseDelta.x || mouseDelta.y) && mousePos.x < _grid_screenWidth * 16)
		{
			if (g_inputDevice.IsKeyPressed(VK_RBUTTON))
				if (!g_inputDevice.IsKeyDown(VK_RBUTTON))
				{
					m_previewScreenScroll -= mouseDelta.x;
					m_previewScreenRedraw = true;
				}

			if (g_inputDevice.IsKeyPressed(VK_LBUTTON))
			{
				int clickedPosX = uiMousePos.x + m_previewScreenScroll;
				int clickedPosY = _grid_lineHeight - (uiMousePos.y - __grid_lineEdgeHeight);

				float a = clickedPosX / _grid_screenWidth * _grid_barPerLine;
				m_focusBox_barPosition = std::max<float>(0, (std::max<int>(clickedPosX, 0) / _grid_screenWidth * _grid_barPerLine) + ((float)clickedPosY / __grid_barInterval));// -__preview_focusBox_offset);
				m_previewScreenRedraw = true;
				m_editableScreenRedraw = true;
			}
		}
	}
	if (m_previewScreenScroll < -300)
	{
		m_previewScreenScroll = -300;
		m_previewScreenRedraw = true;
	}

	// Editable Screen Update
	if (1400 - 1 <= mousePos.x && mousePos.x <= 1600 &&
				0 <= mousePos.y && mousePos.y <= 1000)
	{
		const POINT uiMousePos = { mousePos.x - (1400 - 1), mousePos.y };
		if (__editableScreen_lineEdgeWidth <= uiMousePos.x && uiMousePos.x < __editableScreen_lineEdgeWidth + __editableScreen_lineWidth)
		{
			size_t prevLane = m_edisc_focusBox_laneIndex, prevBeat = m_edisc_focusBox_beatIndex;

			m_edisc_focusBox_laneIndex = std::max<int>(0, ((int)uiMousePos.x - __editableScreen_lineEdgeWidth) / __editableScreen_laneInterval);
			m_edisc_focusBox_beatIndex = std::max<int>(0, ((float)__editableScreen_lineHeight - ((int)uiMousePos.y - __editableScreen_lineEdgeHeight)) / __editableScreen_beatInterval + (m_focusBox_barPosition * _beatPerBar));
			m_edisc_focusBox_beatIndex = (m_edisc_focusBox_beatIndex / (_beatPerBar / m_beatForEdit)) * (_beatPerBar / m_beatForEdit);

			if (g_inputDevice.IsKeyDown(VK_LBUTTON))
			{
				size_t bar = m_edisc_focusBox_beatIndex / _beatPerBar;
				size_t beat = m_edisc_focusBox_beatIndex - (bar * _beatPerBar);
				bool state = m_editor->NoteFlip(m_edisc_focusBox_laneIndex, bar, beat);
				_draw_note(m_edisc_focusBox_laneIndex, bar, beat, state);
				m_previewScreenRedraw = true;
				m_editableScreenRedraw = true;
			}

			if (m_edisc_focusBox_laneIndex != prevLane ||
				m_edisc_focusBox_beatIndex != prevBeat)
				m_editableScreenRedraw = true;
		}
		else
		{
			m_edisc_focusBox_laneIndex = _laneCount;
			m_editableScreenRedraw = true;
		}
	}
}

void SheetEditor_IO::_update_musicTest()
{
	using namespace std::chrono_literals;
	using nanoSec_t = std::chrono::nanoseconds;
	using milliSec_t = std::chrono::milliseconds;
	milliSec_t time = milliSec_t(m_musicChannel.get_time()) - m_offsetTimeForEdit;
	if (time < time.zero())
		return;

	float prevFocusBox_barPosition = m_focusBox_barPosition;
	m_focusBox_barPosition = (long double)(nanoSec_t(time * m_bpmForEdit).count()) / nanoSec_t(240s).count(); // milliSec_t(time) / (nanoSec_t(240s) / m_bpmForEdit);
	size_t beginNote = prevFocusBox_barPosition * _beatPerBar;
	size_t endNote = m_focusBox_barPosition * _beatPerBar;

	const auto& notedata = m_editor->__get_data();
	for (size_t laneIndex = 0; laneIndex < notedata.size(); ++laneIndex)
	{
		size_t drawPositionX = (laneIndex * __editableScreen_laneInterval) + __editableScreen_lineEdgeWidth + 1;
		const auto& laneNoteData = notedata[laneIndex];
		for (size_t noteIndex = beginNote; noteIndex < endNote && noteIndex < laneNoteData.size(); ++noteIndex)
		{
			const auto& note = laneNoteData[noteIndex];
			if (!note)
				continue;

			m_beatHitSound.play().detach();
		}
	}

	m_previewScreenRedraw = true;
	m_editableScreenRedraw = true;
}

