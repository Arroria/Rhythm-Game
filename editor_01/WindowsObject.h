#pragma once
#include "windefinclude.h"
class WindowsDeviceContext;


class WindowsBitmap
{
public:
	WindowsBitmap();
	WindowsBitmap(WindowsBitmap&) = delete; 
	WindowsBitmap(WindowsBitmap&& windowBitmap) noexcept;
	~WindowsBitmap();

	WindowsBitmap& operator=(WindowsBitmap&) = delete;
	WindowsBitmap& operator=(WindowsBitmap&& windowBitmap) noexcept;


	bool Create(WindowsDeviceContext& wdc, size_t width, size_t height);
	void Release();

	bool Created() const;
	bool operator==(const WindowsBitmap& bitmap) const;
	bool operator!=(const WindowsBitmap& bitmap) const;


	size_t Width() const;
	size_t Height() const;

	HBITMAP _getraw_hbitmap() { return m_bitmapHandle; }

private:
	bool _create(WindowsDeviceContext& wdc, size_t width, size_t height);
	void _release();

	bool _created() const;

private:
	HBITMAP m_bitmapHandle;
	size_t m_width;
	size_t m_height;
};

