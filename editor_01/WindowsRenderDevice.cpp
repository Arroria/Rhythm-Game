#include "pch.h"
#include "WindowsRenderDevice.h"
#include <array>

inline void _set_pen_color(HDC hdc, COLORREF color)		{ SetDCPenColor(hdc, color); }
inline void _set_brush_color(HDC hdc, COLORREF color)	{ SetDCBrushColor(hdc, color); }
inline void _set_pen_null(HDC hdc)		{ SelectObject(hdc, GetStockObject(NULL_PEN)); }
inline void _set_pen_stock(HDC hdc)		{ SelectObject(hdc, GetStockObject(DC_PEN)); }
inline void _set_brush_null(HDC hdc)	{ SelectObject(hdc, GetStockObject(NULL_BRUSH)); }
inline void _set_brush_stock(HDC hdc)	{ SelectObject(hdc, GetStockObject(DC_BRUSH)); }

void _hdc_copy_bitmap(HDC destDC, HDC srcDC, int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height)
{ BitBlt(destDC, xPos_paste, yPos_paste, width, height, srcDC, xPos_copy, yPos_copy, SRCCOPY); }
void _hdc_copy_bitmap_transparent(HDC destDC, HDC srcDC, int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height, COLORREF transparentColor)
{ TransparentBlt(destDC, xPos_paste, yPos_paste, width, height, srcDC, xPos_copy, yPos_copy, width, height, transparentColor); }

WindowsRenderDevice::WindowsRenderDevice()
	: m_targetDC(nullptr)
	, m_screenDC()
	, m_memoryDC()
	, m_copyDC()
	, m_backbufferBitmap(std::make_shared<WindowsBitmap>())
{
}

WindowsRenderDevice::~WindowsRenderDevice()
{
	Release();
}


bool WindowsRenderDevice::Initialize(HWND windowHandle, size_t width, size_t height)
{
	std::array<bool, 4> fails;
	fails.fill(false);

	m_targetDC = &m_screenDC;
	fails[0] = !m_screenDC.Initialze(windowHandle);
	fails[1] = !m_memoryDC.Initialze(m_screenDC);
	fails[2] = !m_copyDC.Initialze(m_screenDC);
	fails[3] = !m_backbufferBitmap->Create(m_screenDC, width, height);

	if (fails[0] || fails[1] || fails[2] || fails[3])
	{
		Release();
		return false;
	}

	_set_pen_color	(m_screenDC._getraw_hdc(), RGB(0, 0, 0));
	_set_brush_color(m_screenDC._getraw_hdc(), RGB(255, 255, 255));
	_set_pen_stock	(m_screenDC._getraw_hdc());
	_set_brush_stock(m_screenDC._getraw_hdc());
	_set_pen_color	(m_memoryDC._getraw_hdc(), RGB(0, 0, 0));
	_set_brush_color(m_memoryDC._getraw_hdc(), RGB(255, 255, 255));
	_set_pen_stock	(m_memoryDC._getraw_hdc());
	_set_brush_stock(m_memoryDC._getraw_hdc());
	return true;
}

void WindowsRenderDevice::Release()
{
	m_targetDC = nullptr;
	m_screenDC.Release();
	m_memoryDC.Release();
	m_copyDC.Release();
	m_backbufferBitmap->Release();
}


void WindowsRenderDevice::BackBufferResizing(size_t width, size_t height)
{
	// ¹Ì±¸Çö
	abort();
}

void WindowsRenderDevice::DrawOnScreenDirect()
{
	m_targetDC = &m_screenDC;
}

void WindowsRenderDevice::DrawOnMainBuffer()
{
	m_memoryDC.RegistBitmap(m_backbufferBitmap);
	m_targetDC = &m_memoryDC;
}

void WindowsRenderDevice::DrawOnCustomBitmap(std::shared_ptr<WindowsBitmap> bitmap_ptr)
{
	if (bitmap_ptr && bitmap_ptr->Created())
	{
		m_memoryDC.RegistBitmap(bitmap_ptr);
		m_targetDC = &m_memoryDC;
	}
}

void WindowsRenderDevice::CopyTargetBitmap(std::shared_ptr<WindowsBitmap> bitmap_ptr)
{
	if (bitmap_ptr && bitmap_ptr->Created())
		m_copyDC.RegistBitmap(bitmap_ptr);
}

void WindowsRenderDevice::UnlinkCustomBitmap()		{ m_memoryDC.UnregistBitmap(); }
void WindowsRenderDevice::UnlinkCopyTargetBitmap()	{ m_copyDC.UnregistBitmap(); }


void WindowsRenderDevice::Clipping()
{
	if (!m_screenDC.Created() || !m_memoryDC.Created())
		return;

	m_memoryDC.RegistBitmap(m_backbufferBitmap);
	_hdc_copy_bitmap(m_screenDC._getraw_hdc(), m_memoryDC._getraw_hdc(), 0, 0, 0, 0, (int)m_backbufferBitmap->Width(), (int)m_backbufferBitmap->Height());
}


void WindowsRenderDevice::SetPenColor(COLORREF color)			{ if (_target_available()) _set_pen_color(_get_raw_targetDC(), color); }
void WindowsRenderDevice::SetPenColor(BYTE r, BYTE g, BYTE b)	{ if (_target_available()) SetPenColor(RGB(r, g, b)); }
void WindowsRenderDevice::SetBrushColor(COLORREF color)			{ if (_target_available()) _set_brush_color(_get_raw_targetDC(), color); }
void WindowsRenderDevice::SetBrushColor(BYTE r, BYTE g, BYTE b)	{ if (_target_available()) SetBrushColor(RGB(r, g, b)); }
void WindowsRenderDevice::SetPenTransparent(bool transparent)	{ if (_target_available()) (transparent ? _set_pen_null(_get_raw_targetDC()) : _set_pen_stock(_get_raw_targetDC())); }
void WindowsRenderDevice::SetBrushTransparent(bool transparent)	{ if (_target_available()) (transparent ? _set_brush_null(_get_raw_targetDC()) : _set_brush_stock(_get_raw_targetDC())); }

void WindowsRenderDevice::LinePoint(int xPos, int yPos)						{ if (_target_available()) MoveToEx(_get_raw_targetDC(), xPos, yPos, nullptr); }
void WindowsRenderDevice::LineLink(int xPos, int yPos)						{ if (_target_available()) LineTo(_get_raw_targetDC(), xPos, yPos); }
void WindowsRenderDevice::SingleLine(int xTo, int yTo, int xAt, int yAt)	{ LinePoint(xTo, yTo); LineLink(xAt, yAt); }

void WindowsRenderDevice::DrawBox(int xPos, int yPos, int width, int height)	{ if (_target_available() && width && height) Rectangle(_get_raw_targetDC(), xPos, yPos, xPos + width - 1, yPos + height - 1); }
void WindowsRenderDevice::Fill(int xPos, int yPos, int width, int height)
{
	if (!(_target_available() && width && height))
		return;
	HBRUSH brush = (HBRUSH)GetCurrentObject(_get_raw_targetDC(), OBJ_BRUSH);
	if (!brush)
		return;
	
	RECT temp{ xPos, yPos, xPos + width - 1, yPos + height - 1 };
	FillRect(_get_raw_targetDC(), &temp, brush);
}

void WindowsRenderDevice::Copy(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height)
{
	if (_target_available() && width && height)
		_hdc_copy_bitmap(_get_raw_targetDC(), m_copyDC._getraw_hdc(), xPos_copy, yPos_copy, xPos_paste, yPos_paste, width, height);
}

void WindowsRenderDevice::CopyTransparent(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height, COLORREF transparentColor)
{
	if (_target_available() && width && height)
		_hdc_copy_bitmap_transparent(_get_raw_targetDC(), m_copyDC._getraw_hdc(), xPos_copy, yPos_copy, xPos_paste, yPos_paste, width, height, transparentColor);
}
