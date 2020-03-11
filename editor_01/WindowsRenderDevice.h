#pragma once
#include "WindowsDeviceContext.h"
#include "WindowsObject.h"

class WindowsRenderDevice;
class WNDRD_RenderTarget
{
public:
	WNDRD_RenderTarget();
	WNDRD_RenderTarget(WindowsBitmap&& _raw_bitmap);
	WNDRD_RenderTarget(WNDRD_RenderTarget& wndrd_rt);
	WNDRD_RenderTarget(WNDRD_RenderTarget&& wndrd_rt) noexcept;
	~WNDRD_RenderTarget();

	WNDRD_RenderTarget& operator=(nullptr_t);
	WNDRD_RenderTarget& operator=(WNDRD_RenderTarget& wndrd_rt);
	WNDRD_RenderTarget& operator=(WNDRD_RenderTarget&& wndrd_rt) noexcept;

	bool Create(WindowsRenderDevice& wndRenderDevice, size_t width, size_t height);
	void Release();

	bool Created() const;
	size_t Width() const;
	size_t Height() const;

	std::shared_ptr<WindowsBitmap> _getraw_sptr() { return m_wndBitmap; }

private:
	void _set_null();

private:
	std::shared_ptr<WindowsBitmap> m_wndBitmap;
};

class WindowsRenderDevice
{
public:
	WindowsRenderDevice();
	~WindowsRenderDevice();


	bool Initialize(HWND windowHandle, size_t width, size_t height);
	void Release();

	void BackBufferResizing(size_t width, size_t height);


	// Draw Target
	bool DrawOnScreenDirect();
	bool DrawOnScreen();
	bool DrawOnRenderTarget(WNDRD_RenderTarget& renderTarget);
	void UnlinkRenderTarget();

	bool LinkCopyRenderTarget(WNDRD_RenderTarget& bitmap_ptr);
	void UnlinkCopyRenderTarget();

	void Clipping();


	bool CreateRenderTarget(WNDRD_RenderTarget& out_renderTarget, size_t width, size_t height);

	// Tools
	void SetPenColor(COLORREF color);
	void SetPenColor(BYTE r, BYTE g, BYTE b);
	void SetBrushColor(COLORREF color);
	void SetBrushColor(BYTE r, BYTE g, BYTE b);
	void SetPenTransparent(bool transparent);
	void SetBrushTransparent(bool transparent);

	constexpr static COLORREF DefaultTransparentColor() { return RGB(255, 0, 255); }


	// Draw
	constexpr static bool _drawOption_negativeSize_allow = false;

	bool DrawPixel(int xPos, int yPos, COLORREF color);
	bool DrawPixelByPen(int xPos, int yPos);
	bool DrawPixelByBrush(int xPos, int yPos);

	bool DrawLine(int begin_xPos, int begin_yPos, int end_xPos, int end_yPos);
	//bool DrawLineList(???); 

	bool DrawBoxSize(int xPos, int yPos, int width, int height);
	bool DrawBoxCoord(int xPos1, int yPos1, int xPos2, int yPos2);

	bool FillSize(int xPos, int yPos, int width, int height);
	bool FillCoord(int xPos1, int yPos1, int xPos2, int yPos2);
	void _FillSize_ExtendExample(int xPos, int yPos, int width, int height, DWORD ropCode);

	bool CopySize(int copy_xPos, int copy_yPos, int copy_width, int copy_height, int paste_xPos, int paste_yPos);
	bool CopyCoord(int copy_xPos1, int copy_yPos1, int copy_xPos2, int copy_yPos2, int paste_xPos, int paste_yPos);
	bool CopyTransparentSize(int copy_xPos, int copy_yPos, int copy_width, int copy_height, int paste_xPos, int paste_yPos, COLORREF transparentColor = DefaultTransparentColor());
	bool CopyTransparentCoord(int copy_xPos1, int copy_yPos1, int copy_xPos2, int copy_yPos2, int paste_xPos, int paste_yPos, COLORREF transparentColor = DefaultTransparentColor());

	// Old draw func
	//void LinePoint(int xPos, int yPos);
	//void LineLink(int xPos, int yPos);
	//void SingleLine(int xTo, int yTo, int xAt, int yAt);
	//
	//void DrawBox(int xPos, int yPos, int width, int height);
	//void Fill(int xPos, int yPos, int width, int height);
	//
	//void Copy(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height);
	//void CopyTransparent(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height, COLORREF transparentColor = DefaultTransparentColor());

private:
	bool _target_available() { return m_targetDC && m_targetDC->Created(); }
	HDC _get_raw_targetDC() { return m_targetDC->_getraw_hdc(); }

	COLORREF _get_pen_color();
	COLORREF _get_brush_color();
	bool _draw_pixel(int xPos, int yPos, COLORREF color);




private:
	WindowsDeviceContext* m_targetDC;
	WDC_Screen m_screenDC;
	WDC_Memory m_memoryDC;
	WDC_Memory m_copyDC;
	WNDRD_RenderTarget m_backbufferBitmap;
};
