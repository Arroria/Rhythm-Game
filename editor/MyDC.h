#pragma once
#include "mywinapi_default_include.h"
#include "MyRenderTarget.h"
#include <stdexcept>

class DC_Base abstract
{
public:
	DC_Base();
	DC_Base(const DC_Base&) = delete;
	virtual ~DC_Base();

	DC_Base& operator=(const DC_Base&) = delete;


	// 항목명 뭐라하지
	virtual void Release() = 0;
	virtual bool Allocated() const;


	// Setting
	void SetPenColor(COLORREF color);
	void SetPenColor(BYTE r, BYTE g, BYTE b);
	void SetBrushColor(COLORREF color);
	void SetBrushColor(BYTE r, BYTE g, BYTE b);
	void SetPenTransparent(bool transparent);
	void SetBrushTransparent(bool transparent);

	// Tools
	void LinePoint(int x, int y);
	void LineLink(int x, int y);
	void SingleLine(int x1, int y1, int x2, int y2);

	void DrawRectBySize(int positionX, int positionY, int sizeX, int sizeY);
	void DrawRectByRect(int left, int top, int right, int bottom);
	inline void DrawRectByRect(RECT rect) { DrawRectByRect(rect.left, rect.top, rect.right, rect.bottom); }

	void Paste(const DC_Base& copy_dc, int copyPositionX, int copyPositionY, int pastePositionX, int pastePositionY, int sizeX, int sizeY);
	void PasteTransparent(const DC_Base& copy_dc, int copyPositionX, int copyPositionY, int pastePositionX, int pastePositionY, int sizeX, int sizeY, COLORREF transparentColor);

	// Get Data
	HDC DeviceContextHandle() { return m_dcHandle; }

protected:
	HDC m_dcHandle;

	void _if_allocated_throw() const;
	void _if_unallocated_throw() const;
};


class DC_Window
	: public DC_Base
{
public:
	DC_Window();
	~DC_Window();

	void Initialize(HWND windowHandle);
	void Release() override;

	HWND BasedWindowHandle();


private:
	HWND m_windowHandle;
};


class DC_RenderTarget
	: public DC_Base
{
public:
	DC_RenderTarget();
	~DC_RenderTarget();

	void Initialize(DC_Window& base_dc);
	void Release();

	void SetRenderTarget(MyRenderTarget& renderTarget);
	void SetRenderTarget(MyRenderTarget&& renderTarget); // release upon change or release
	void DetachRenderTarget();

	bool HasRenderTarget() const;
	MyRenderTarget& CurrentRenderTarget();
	const MyRenderTarget& CurrentRenderTarget() const;


private:
	HBITMAP m_default_bitmap;
	MyRenderTarget m_currentRendertarget;
};
using DC_Memory = DC_RenderTarget;
