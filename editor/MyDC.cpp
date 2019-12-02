#include "MyDC.h"
#include <utility>

inline void _set_pen_null(HDC hdc) { SelectObject(hdc, GetStockObject(NULL_PEN)); }
inline void _set_pen_stock(HDC hdc) { SelectObject(hdc, GetStockObject(DC_PEN)); }
inline void _set_brush_null(HDC hdc) { SelectObject(hdc, GetStockObject(NULL_BRUSH)); }
inline void _set_brush_stock(HDC hdc) { SelectObject(hdc, GetStockObject(DC_BRUSH)); }



DC_Base::DC_Base()
	: m_dcHandle(nullptr)
{
}

DC_Base::~DC_Base()
{
}


bool DC_Base::Allocated() const { return m_dcHandle != nullptr; }


void DC_Base::SetPenColor(COLORREF color)			{ _if_unallocated_throw(); SetDCPenColor(m_dcHandle, color); }
void DC_Base::SetPenColor(BYTE r, BYTE g, BYTE b)	{ _if_unallocated_throw(); SetPenColor(RGB(r, g, b)); }
void DC_Base::SetBrushColor(COLORREF color)			{ _if_unallocated_throw(); SetDCBrushColor(m_dcHandle, color); }
void DC_Base::SetBrushColor(BYTE r, BYTE g, BYTE b)	{ _if_unallocated_throw(); SetBrushColor(RGB(r, g, b)); }

void DC_Base::SetPenTransparent(bool transparent)	{ _if_unallocated_throw(); transparent ? _set_pen_null(m_dcHandle) : _set_pen_stock(m_dcHandle); }
void DC_Base::SetBrushTransparent(bool transparent)	{ _if_unallocated_throw(); transparent ? _set_brush_null(m_dcHandle) : _set_brush_stock(m_dcHandle); }

void DC_Base::LinePoint(int x, int y)						{ _if_unallocated_throw(); MoveToEx(m_dcHandle, x, y, nullptr); }
void DC_Base::LineLink(int x, int y)						{ _if_unallocated_throw(); LineTo(m_dcHandle, x, y); }
void DC_Base::SingleLine(int x1, int y1, int x2, int y2)	{ _if_unallocated_throw(); LinePoint(x1, y1); LineLink(x2, y2); }

void DC_Base::DrawRectBySize(int positionX, int positionY, int sizeX, int sizeY)	{ _if_unallocated_throw(); DrawRectByRect(positionX, positionY, positionX + sizeX, positionY + sizeY); }
void DC_Base::DrawRectByRect(int left, int top, int right, int bottom)				{ _if_unallocated_throw(); Rectangle(m_dcHandle, left, top, right, bottom); }


void DC_Base::Paste(const DC_Base& copy_dc, int copyPositionX, int copyPositionY, int pastePositionX, int pastePositionY, int sizeX, int sizeY)
{ _if_unallocated_throw(); 

int a = BitBlt(m_dcHandle, pastePositionX, pastePositionY, sizeX, sizeY, copy_dc.m_dcHandle, copyPositionX, copyPositionY, SRCCOPY);
if (!a)
int b = 5;
}
void DC_Base::PasteTransparent(const DC_Base& copy_dc, int copyPositionX, int copyPositionY, int pastePositionX, int pastePositionY, int sizeX, int sizeY, COLORREF transparentColor)
{ _if_unallocated_throw(); TransparentBlt(m_dcHandle, pastePositionX, pastePositionY, sizeX, sizeY, copy_dc.m_dcHandle, copyPositionX, copyPositionY, sizeX, sizeY, transparentColor); }


void DC_Base::_if_allocated_throw() const { if (Allocated()) throw std::logic_error("is already has been allocated."); }
void DC_Base::_if_unallocated_throw() const { if (!Allocated()) throw std::logic_error("is unallocated."); }







DC_Window::DC_Window()
	: m_windowHandle(nullptr)
{
}

DC_Window::~DC_Window()
{
	if (Allocated())
		Release();
}


void DC_Window::Initialize(HWND windowHandle)
{
	_if_allocated_throw();

	m_windowHandle = windowHandle;
	m_dcHandle = GetDC(m_windowHandle);

	_set_pen_stock(m_dcHandle);
	_set_brush_stock(m_dcHandle);
}

void DC_Window::Release()
{
	_if_unallocated_throw();

	ReleaseDC(m_windowHandle, m_dcHandle);
	m_windowHandle = nullptr;
	m_dcHandle = nullptr;
}


HWND DC_Window::BasedWindowHandle() { _if_unallocated_throw(); return m_windowHandle; }






DC_RenderTarget::DC_RenderTarget()
	: m_default_bitmap(nullptr)
	, m_currentRendertarget()
{
}

DC_RenderTarget::~DC_RenderTarget()
{
	if (Allocated())
		Release();
}



void DC_RenderTarget::Initialize(DC_Window& base_dc)
{
	_if_allocated_throw();

	m_dcHandle = CreateCompatibleDC(base_dc.DeviceContextHandle());
	m_default_bitmap = (HBITMAP)GetCurrentObject(m_dcHandle, OBJ_BITMAP);

	_set_pen_stock(m_dcHandle);
	_set_brush_stock(m_dcHandle);
}

void DC_RenderTarget::Release()
{
	_if_unallocated_throw();

	if (m_currentRendertarget)
		m_currentRendertarget.Release();
	DeleteDC(m_dcHandle);
	m_dcHandle = nullptr;
}


void DC_RenderTarget::SetRenderTarget(MyRenderTarget& renderTarget)		{ _if_unallocated_throw(); if (m_currentRendertarget != renderTarget) SelectObject(m_dcHandle, (m_currentRendertarget = renderTarget)._get_primitive()); }
void DC_RenderTarget::SetRenderTarget(MyRenderTarget&& renderTarget)	{ _if_unallocated_throw(); if (m_currentRendertarget != renderTarget) SelectObject(m_dcHandle, (m_currentRendertarget = std::move(renderTarget))._get_primitive()); }
void DC_RenderTarget::DetachRenderTarget()
{
	_if_unallocated_throw();
	if (m_currentRendertarget)
	{
		auto a = SelectObject(m_dcHandle, m_default_bitmap);
		m_currentRendertarget.Release();
	}
}

bool DC_RenderTarget::HasRenderTarget() const						{ _if_unallocated_throw(); return m_currentRendertarget; }
MyRenderTarget& DC_RenderTarget::CurrentRenderTarget()				{ _if_unallocated_throw(); return m_currentRendertarget; }
const MyRenderTarget& DC_RenderTarget::CurrentRenderTarget() const	{ _if_unallocated_throw(); return m_currentRendertarget; }
