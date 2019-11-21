#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>


enum class MyDC_Type
{
	Unallocated,
	EntireScreen,
	WindowScreen,
	Memory,
};

class MyDC
{
public:
	using Type_t = MyDC_Type;

	MyDC();
	~MyDC();

	// Creation & Destruction
	void Initialize(HWND windowHandle);
	void Initialize(const MyDC& base_dc, size_t memorySizeX, size_t memorySizeY);
	void Release();

	// Setting
	void SetPenColor(BYTE r, BYTE g, BYTE b);
	void SetBrushColor(BYTE r, BYTE g, BYTE b);
	void SetPenTransparent(bool transparent);
	void SetBrushTransparent(bool transparent);
	
	// Tools
	void LinePoint(int x, int y);
	void LineLink(int x, int y);

	void DrawRectBySize(int positionX, int positionY, int sizeX, int sizeY);
	void DrawRectByRect(int left, int top, int right, int bottom);
	inline void DrawRectByRect(RECT rect) { DrawRectByRect(rect.left, rect.top, rect.right, rect.bottom); }

	void Paste(const MyDC& copy_dc, int copyPositionX, int copyPositionY, int pastePositionX, int pastePositionY, int sizeX, int sizeY);

	// Get Data
	HDC GetDCHandle() const { return m_dcHandle; }

private:
	Type_t m_type;
	HDC m_dcHandle;

	HWND m_windowHandle;
	HBITMAP m_bitmapHandle;

};


inline void MyDC_FillRect(MyDC& myDC, int left, int top, int right, int bottom, HBRUSH brush)
{
	RECT temp{left, top, right, bottom};
	FillRect(myDC.GetDCHandle(), &temp, brush);
}
