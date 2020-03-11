#include "pch.h"
#include "WindowsRenderDevice.h"
#include <array>


// raw guide func
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
	, m_backbufferBitmap()
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
	fails[3] = !CreateRenderTarget(m_backbufferBitmap, width, height);

	if (fails[0] || fails[1] || fails[2] || fails[3])
	{
		Release();
		return false;
	}

	_set_pen_stock	(m_screenDC._getraw_hdc());
	_set_brush_stock(m_screenDC._getraw_hdc());
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
	m_backbufferBitmap.Release();
}


void WindowsRenderDevice::BackBufferResizing(size_t width, size_t height)
{
	// 미구현
	std::terminate();
}


bool WindowsRenderDevice::DrawOnScreenDirect()
{
	m_targetDC = &m_screenDC;
	return true;
}

bool WindowsRenderDevice::DrawOnScreen()
{
	m_memoryDC.RegistBitmap(m_backbufferBitmap._getraw_sptr());
	m_targetDC = &m_memoryDC;
	return true;
}

bool WindowsRenderDevice::DrawOnRenderTarget(WNDRD_RenderTarget& bitmap_ptr)
{
	if (!bitmap_ptr.Created())
		return false;

	m_memoryDC.RegistBitmap(bitmap_ptr._getraw_sptr());
	m_targetDC = &m_memoryDC;
	return true;
}

bool WindowsRenderDevice::LinkCopyRenderTarget(WNDRD_RenderTarget& bitmap_ptr)
{
	if (!bitmap_ptr.Created())
		return false;
	m_copyDC.RegistBitmap(bitmap_ptr._getraw_sptr());
	return true;
}

void WindowsRenderDevice::UnlinkRenderTarget()		{ m_memoryDC.UnregistBitmap(); }
void WindowsRenderDevice::UnlinkCopyRenderTarget()	{ m_copyDC.UnregistBitmap(); }


void WindowsRenderDevice::Clipping()
{
	if (!m_screenDC.Created() || !m_memoryDC.Created())
		return;

	m_memoryDC.RegistBitmap(m_backbufferBitmap._getraw_sptr());
	_hdc_copy_bitmap(m_screenDC._getraw_hdc(), m_memoryDC._getraw_hdc(), 0, 0, 0, 0, (int)m_backbufferBitmap.Width(), (int)m_backbufferBitmap.Height());
}

bool WindowsRenderDevice::CreateRenderTarget(WNDRD_RenderTarget& out, size_t width, size_t height)
{
	if (out.Created())
		return false;

	WindowsBitmap bitmap;
	if (!bitmap.Create(m_screenDC, width, height))
		return false;

	out = WNDRD_RenderTarget(std::move(bitmap));
	return true;
}


void WindowsRenderDevice::SetPenColor(COLORREF color)			{ if (_target_available()) _set_pen_color(_get_raw_targetDC(), color); }
void WindowsRenderDevice::SetPenColor(BYTE r, BYTE g, BYTE b)	{ if (_target_available()) SetPenColor(RGB(r, g, b)); }
void WindowsRenderDevice::SetBrushColor(COLORREF color)			{ if (_target_available()) _set_brush_color(_get_raw_targetDC(), color); }
void WindowsRenderDevice::SetBrushColor(BYTE r, BYTE g, BYTE b)	{ if (_target_available()) SetBrushColor(RGB(r, g, b)); }
void WindowsRenderDevice::SetPenTransparent(bool transparent)	{ if (_target_available()) (transparent ? _set_pen_null(_get_raw_targetDC()) : _set_pen_stock(_get_raw_targetDC())); }
void WindowsRenderDevice::SetBrushTransparent(bool transparent)	{ if (_target_available()) (transparent ? _set_brush_null(_get_raw_targetDC()) : _set_brush_stock(_get_raw_targetDC())); }



//개선중
bool WindowsRenderDevice::DrawPixelByPen(int xPos, int yPos)
{
	if (!_target_available())
		return false;
	return _draw_pixel(xPos, yPos, _get_pen_color());
}
bool WindowsRenderDevice::DrawPixelByBrush(int xPos, int yPos)
{
	if (!_target_available())
		return false;
	return _draw_pixel(xPos, yPos, _get_brush_color());
}
bool WindowsRenderDevice::DrawPixel(int xPos, int yPos, COLORREF color)
{
	if (!_target_available())
		return false;
	return _draw_pixel(xPos, yPos, color);
}


bool WindowsRenderDevice::DrawLine(int begin_xPos, int begin_yPos, int end_xPos, int end_yPos)
{
	if (!_target_available())
		return false;
	HDC dcHandle = _get_raw_targetDC();
	return
		MoveToEx(dcHandle, begin_xPos, begin_yPos, nullptr) &&
		LineTo(dcHandle, end_xPos, end_yPos) &&
		DrawPixelByPen(end_xPos, end_yPos);
	// LineTo에서 선의 마지막 픽셀을 그리지 않으므로 DrawPixel 호출이 필요함
}

bool WindowsRenderDevice::DrawBoxSize(int xPos, int yPos, int width, int height)
{
	if (!_target_available())
		return false;

	if constexpr (!_drawOption_negativeSize_allow)
		if (width < 0 || height < 0)
			return false;
	if (!width || !height)
		return false;

	int xPos2 = xPos + width - (width > 0 ? 1 : -1);
	int yPos2 = yPos + height - (height > 0 ? 1 : -1);
	return DrawBoxCoord(xPos, yPos, xPos2, yPos2);
}

bool WindowsRenderDevice::DrawBoxCoord(int xPos1, int yPos1, int xPos2, int yPos2)
{
	if (!_target_available())
		return false;
	
	// 사각형의 넓이가 1인경우 그려지지 않으므로 DrawPixel을 이용해 출력함
	if (xPos1 == xPos2 && yPos1 == yPos2)
		return DrawPixelByPen(xPos1, yPos1);

	// 사각형의 너비, 높이가 1 만큼 낮게 출력되므로 각각 1씩 추가됨
	++(xPos1 > xPos2 ? xPos1 : xPos2);
	++(yPos1 > yPos2 ? yPos1 : yPos2);
	
	// 펜이 투명할경우 브러쉬의 영역이 1만큼 작게 출력되되는 문제점이 있지만 수정하진 않았음
	return Rectangle(_get_raw_targetDC(), xPos1, yPos1, xPos2, yPos2);
}

bool WindowsRenderDevice::FillSize(int xPos, int yPos, int width, int height)
{
	if (!_target_available())
		return false;

	if constexpr (!_drawOption_negativeSize_allow)
		if (width < 0 || height < 0)
			return false;
	if (!width || !height)
		return false;
	
	int xPos2 = xPos + width - (width > 0 ? 1 : -1);
	int yPos2 = yPos + height - (height > 0 ? 1 : -1);
	return FillCoord(xPos, yPos, xPos2, yPos2);
}

bool WindowsRenderDevice::FillCoord(int xPos1, int yPos1, int xPos2, int yPos2)
{
	if (!_target_available())
		return false;

	HBRUSH brush = (HBRUSH)GetCurrentObject(_get_raw_targetDC(), OBJ_BRUSH);
	if (!brush)
		return false;

	// 사각형의 너비, 높이가 1 만큼 낮게 출력되므로 각각 1씩 추가됨
	++(xPos1 > xPos2 ? xPos1 : xPos2);
	++(yPos1 > yPos2 ? yPos1 : yPos2);

	RECT rectArea{ xPos1, yPos1, xPos2, yPos2 };
	return FillRect(_get_raw_targetDC(), &rectArea, brush);
}

// 함수 전체가 전혀 검증되지 않음
void WindowsRenderDevice::_FillSize_ExtendExample(int xPos, int yPos, int width, int height, DWORD ropCode)
{
	// FillRect의 확장격 함수
	PatBlt(_get_raw_targetDC(), xPos, yPos, width, height, ropCode);
}
// 이거말고 PlgBlt도 찾아봐야하는데

bool WindowsRenderDevice::CopySize(int copy_xPos, int copy_yPos, int copy_width, int copy_height, int paste_xPos, int paste_yPos)
{
	if (!_target_available())
		return false;

	if constexpr (!_drawOption_negativeSize_allow)
		if (copy_width < 0 || copy_height < 0)
			return false;
	if (!copy_width || !copy_height)
		return false;

	// 나중에 StaticCopy로 함수이름을 바꿔야 할 것 같음
	int copy_xPos2 = copy_xPos + copy_width - (0 < copy_width ? 1 : -1);
	int copy_yPos2 = copy_yPos + copy_height - (0 < copy_height ? 1 : -1);
	return CopyCoord(copy_xPos, copy_yPos, copy_xPos2, copy_yPos2, paste_xPos, paste_yPos);
}

bool WindowsRenderDevice::CopyCoord(int copy_xPos1, int copy_yPos1, int copy_xPos2, int copy_yPos2, int paste_xPos, int paste_yPos)
{
	// 복사본 관련 예외처리 없음
	if (!_target_available())
		return false;
	const auto& copyRenderTarget = m_copyDC.RegistedBitmap();
	
	if (copy_xPos1 > copy_xPos2) std::swap(copy_xPos1, copy_xPos2);
	if (copy_yPos1 > copy_yPos2) std::swap(copy_yPos1, copy_yPos2);
	int copy_width = copy_xPos2 - copy_xPos1 + 1;
	int copy_height = copy_yPos2 - copy_yPos1 + 1;

	if (copy_xPos1 < 0) copy_xPos1 = 0;
	if (copy_yPos1 < 0) copy_yPos1 = 0;
	if (copyRenderTarget->Width() < copy_xPos2)		copy_width = copyRenderTarget->Width();
	if (copyRenderTarget->Height() < copy_yPos2)	copy_height = copyRenderTarget->Height();

	return BitBlt(_get_raw_targetDC(), paste_xPos, paste_yPos, copy_width, copy_height, m_copyDC._getraw_hdc(), copy_xPos1, copy_yPos1, SRCCOPY);
}

bool WindowsRenderDevice::CopyTransparentSize(int copy_xPos, int copy_yPos, int copy_width, int copy_height, int paste_xPos, int paste_yPos, COLORREF transparentColor)
{
	if (!_target_available())
		return false;

	if constexpr (!_drawOption_negativeSize_allow)
		if (copy_width < 0 || copy_height < 0)
			return false;
	if (!copy_width || !copy_height)
		return false;

	// 나중에 StaticCopy로 함수이름을 바꿔야 할 것 같음
	int copy_xPos2 = copy_xPos + copy_width - (0 < copy_width ? 1 : -1);
	int copy_yPos2 = copy_yPos + copy_height - (0 < copy_height ? 1 : -1);
	return CopyTransparentCoord(copy_xPos, copy_yPos, copy_xPos2, copy_yPos2, paste_xPos, paste_yPos, transparentColor);
}

bool WindowsRenderDevice::CopyTransparentCoord(int copy_xPos1, int copy_yPos1, int copy_xPos2, int copy_yPos2, int paste_xPos, int paste_yPos, COLORREF transparentColor)
{
	// 복사본 관련 예외처리 없음
	if (!_target_available())
		return false;
	const auto& copyRenderTarget = m_copyDC.RegistedBitmap();

	if (copy_xPos1 > copy_xPos2) std::swap(copy_xPos1, copy_xPos2);
	if (copy_yPos1 > copy_yPos2) std::swap(copy_yPos1, copy_yPos2);
	int copy_width = copy_xPos2 - copy_xPos1 + 1;
	int copy_height = copy_yPos2 - copy_yPos1 + 1;

	if (copy_xPos1 < 0) copy_xPos1 = 0;
	if (copy_yPos1 < 0) copy_yPos1 = 0;
	if (copyRenderTarget->Width() < copy_xPos2)		copy_width = copyRenderTarget->Width();
	if (copyRenderTarget->Height() < copy_yPos2)	copy_height = copyRenderTarget->Height();

	return TransparentBlt(_get_raw_targetDC(), paste_xPos, paste_yPos, copy_width, copy_height, m_copyDC._getraw_hdc(), copy_xPos1, copy_yPos1, copy_width, copy_height, transparentColor);
}




COLORREF WindowsRenderDevice::_get_pen_color() { return GetDCPenColor(_get_raw_targetDC()); }
COLORREF WindowsRenderDevice::_get_brush_color() { return GetDCBrushColor(_get_raw_targetDC()); }

bool WindowsRenderDevice::_draw_pixel(int xPos, int yPos, COLORREF color) { return SetPixelV(_get_raw_targetDC(), xPos, yPos, color); }




WNDRD_RenderTarget::WNDRD_RenderTarget()
	: m_wndBitmap(nullptr)
{
}

WNDRD_RenderTarget::WNDRD_RenderTarget(WindowsBitmap&& _raw_bitmap)
	: m_wndBitmap(std::make_shared<WindowsBitmap>(std::move(_raw_bitmap)))
{
}

WNDRD_RenderTarget::WNDRD_RenderTarget(WNDRD_RenderTarget& wndrd_rt)
	: m_wndBitmap(wndrd_rt.m_wndBitmap)
{
}

WNDRD_RenderTarget::WNDRD_RenderTarget(WNDRD_RenderTarget&& wndrd_rt) noexcept
	: m_wndBitmap(std::move(wndrd_rt.m_wndBitmap))
{
}

WNDRD_RenderTarget::~WNDRD_RenderTarget()
{
	_set_null();
}



WNDRD_RenderTarget& WNDRD_RenderTarget::operator=(nullptr_t)								{ _set_null();															return *this; }
WNDRD_RenderTarget& WNDRD_RenderTarget::operator=(WNDRD_RenderTarget& wndrd_rt)				{ m_wndBitmap = wndrd_rt.m_wndBitmap;									return *this; }
WNDRD_RenderTarget& WNDRD_RenderTarget::operator=(WNDRD_RenderTarget&& wndrd_rt) noexcept	{ m_wndBitmap = std::move(wndrd_rt.m_wndBitmap); wndrd_rt._set_null();	return *this; }

bool WNDRD_RenderTarget::Create(WindowsRenderDevice& wndRenderDevice, size_t width, size_t height)	{ return wndRenderDevice.CreateRenderTarget(*this, width, height); }
void WNDRD_RenderTarget::Release()	{ _set_null(); }

bool WNDRD_RenderTarget::Created() const	{ return m_wndBitmap && m_wndBitmap->Created(); }
size_t WNDRD_RenderTarget::Width() const	{ return m_wndBitmap->Width(); }
size_t WNDRD_RenderTarget::Height() const	{ return m_wndBitmap->Height(); }

void WNDRD_RenderTarget::_set_null() { m_wndBitmap = nullptr; }


