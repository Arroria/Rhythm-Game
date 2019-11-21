#include "mywinapi.h"


inline void _set_pen_null(HDC hdc)		{ SelectObject(hdc, GetStockObject(NULL_PEN)); }
inline void _set_pen_stock(HDC hdc)		{ SelectObject(hdc, GetStockObject(DC_PEN)); }
inline void _set_brush_null(HDC hdc)	{ SelectObject(hdc, GetStockObject(NULL_BRUSH)); }
inline void _set_brush_stock(HDC hdc)	{ SelectObject(hdc, GetStockObject(DC_BRUSH)); }





MyDC::MyDC()
	: m_type(Type_t::Unallocated)
	, m_dcHandle(nullptr)

	, m_windowHandle(nullptr)
	, m_bitmapHandle(nullptr)
{
}

MyDC::~MyDC()
{
}



void MyDC::Initialize(HWND hWnd)
{
	m_windowHandle = hWnd;
	m_type = (m_windowHandle == nullptr ? Type_t::EntireScreen : Type_t::WindowScreen);
	m_dcHandle = GetDC(m_windowHandle);
	m_bitmapHandle = nullptr;

	_set_pen_stock(m_dcHandle);
	_set_brush_stock(m_dcHandle);
}

void MyDC::Initialize(const MyDC& base_dc, size_t memorySizeX, size_t memorySizeY)
{
	HDC handle_by_base_dc = base_dc.GetDCHandle();

	m_windowHandle = nullptr;
	m_type = Type_t::Memory;
	m_dcHandle = CreateCompatibleDC(handle_by_base_dc);
	m_bitmapHandle = CreateCompatibleBitmap(handle_by_base_dc, memorySizeX, memorySizeY);

	SelectObject(m_dcHandle, m_bitmapHandle);

	_set_pen_stock(m_dcHandle);
	_set_brush_stock(m_dcHandle);
}

void MyDC::Release()
{
	if (m_type == Type_t::Memory)
	{
		DeleteObject(m_bitmapHandle);
		DeleteDC(m_dcHandle);
	}
	else
		ReleaseDC(m_windowHandle, m_dcHandle);
	m_windowHandle = nullptr;
}



void MyDC::SetPenColor(BYTE r, BYTE g, BYTE b)		{ SetDCPenColor(m_dcHandle, RGB(r, g, b)); }
void MyDC::SetBrushColor(BYTE r, BYTE g, BYTE b)	{ SetDCBrushColor(m_dcHandle, RGB(r, g, b)); }

void MyDC::SetPenTransparent(bool transparent)		{ transparent ? _set_pen_null(m_dcHandle) : _set_pen_stock(m_dcHandle); }
void MyDC::SetBrushTransparent(bool transparent)	{ transparent ? _set_brush_null(m_dcHandle) : _set_brush_stock(m_dcHandle); }


void MyDC::LinePoint(int x, int y)	{ MoveToEx(m_dcHandle, x, y, nullptr); }
void MyDC::LineLink(int x, int y)	{ LineTo(m_dcHandle, x, y); }

void MyDC::DrawRectBySize(int positionX, int positionY, int sizeX, int sizeY)	{ DrawRectByRect(positionX, positionY, positionX + sizeX, positionY + sizeY); }
void MyDC::DrawRectByRect(int left, int top, int right, int bottom)				{ Rectangle(m_dcHandle, left, top, right, bottom); }

void MyDC::Paste(const MyDC& copy_dc, int copyPositionX, int copyPositionY, int pastePositionX, int pastePositionY, int sizeX, int sizeY)
{ BitBlt(m_dcHandle, pastePositionX, pastePositionY, sizeX, sizeY, copy_dc.GetDCHandle(), copyPositionX, copyPositionY, SRCCOPY); }
