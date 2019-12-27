#pragma once
#include "WindowsDeviceContext.h"
#include "WindowsObject.h"

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
	void DrawOnCustomBitmap(std::shared_ptr<WindowsBitmap> bitmap_ptr);
	void CopyTargetBitmap(std::shared_ptr<WindowsBitmap> bitmap_ptr);
	void UnlinkCustomBitmap();
	void UnlinkCopyTargetBitmap();

	void Clipping();

	//std::shared_ptr<WindowBitmap> CreateCustomBitmap

	// Tools
	void SetPenColor(COLORREF color);
	void SetPenColor(BYTE r, BYTE g, BYTE b);
	void SetBrushColor(COLORREF color);
	void SetBrushColor(BYTE r, BYTE g, BYTE b);
	void SetPenTransparent(bool transparent);
	void SetBrushTransparent(bool transparent);

	// Draw
	void LinePoint(int xPos, int yPos);
	void LineLink(int xPos, int yPos);
	void SingleLine(int xTo, int yTo, int xAt, int yAt);

	void DrawBox(int xPos, int yPos, int width, int height);
	void Fill(int xPos, int yPos, int width, int height);

	void Copy(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height);
	void CopyTransparent(int xPos_copy, int yPos_copy, int xPos_paste, int yPos_paste, int width, int height, COLORREF transparentColor = RGB(255, 0, 255));

private:
	bool _target_available() { return m_targetDC && m_targetDC->Created(); }
	HDC _get_raw_targetDC() { return m_targetDC->_getraw_hdc(); }

private:
	WindowsDeviceContext* m_targetDC;
	WDC_Screen m_screenDC;
	WDC_Memory m_memoryDC;
	WDC_Memory m_copyDC;
	std::shared_ptr<WindowsBitmap> m_backbufferBitmap;
};

