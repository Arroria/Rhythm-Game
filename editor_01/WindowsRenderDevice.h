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


	void DrawOnScreenDirect();
	void DrawOnMainBuffer();
	void DrawOnCustomBitmap(WNDRD_RenderTarget& bitmap_ptr);
	void CopyTargetBitmap(WNDRD_RenderTarget& bitmap_ptr);
	void UnlinkCustomBitmap();
	void UnlinkCopyTargetBitmap();

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
	void LinePoint(int xPos, int yPos);
	void LineLink(int xPos, int yPos);
	void SingleLine(int xTo, int yTo, int xAt, int yAt);

	void DrawBox(int xPos, int yPos, int width, int height);
	void Fill(int xPos, int yPos, int width, int height);

	void Copy(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height);
	void CopyTransparent(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height, COLORREF transparentColor = DefaultTransparentColor());

private:
	bool _target_available() { return m_targetDC && m_targetDC->Created(); }
	HDC _get_raw_targetDC() { return m_targetDC->_getraw_hdc(); }

private:
	WindowsDeviceContext* m_targetDC;
	WDC_Screen m_screenDC;
	WDC_Memory m_memoryDC;
	WDC_Memory m_copyDC;
	WNDRD_RenderTarget m_backbufferBitmap;
};
