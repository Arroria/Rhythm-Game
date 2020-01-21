#include "pch.h"
#include "WindowsObject.h"
#include "WindowsDeviceContext.h"

WindowsBitmap::WindowsBitmap()
	: m_bitmapHandle(nullptr)
	, m_width(0)
	, m_height(0)
{
}

WindowsBitmap::WindowsBitmap(WindowsBitmap&& windowBitmap) noexcept
	: m_bitmapHandle(windowBitmap.m_bitmapHandle)
	, m_width(windowBitmap.m_width)
	, m_height(windowBitmap.m_height)
{
	windowBitmap.m_bitmapHandle = nullptr;
	windowBitmap.m_width = 0;
	windowBitmap.m_height = 0;
}

WindowsBitmap::~WindowsBitmap()
{
	if (_created())
		_release();
}


WindowsBitmap& WindowsBitmap::operator=(WindowsBitmap&& windowBitmap) noexcept
{
	m_bitmapHandle = windowBitmap.m_bitmapHandle;
	m_width = windowBitmap.m_width;
	m_height = windowBitmap.m_height;
	windowBitmap.m_bitmapHandle = nullptr;
	windowBitmap.m_width = 0;
	windowBitmap.m_height = 0;
	return *this;
}

bool WindowsBitmap::Create(WindowsDeviceContext& wdc, size_t width, size_t height)
{
	if (_created() || !_create(wdc, width, height))
		return false;
	return true;
}

void WindowsBitmap::Release()
{
	if (_created())
		_release();
}

bool WindowsBitmap::Created() const	{ return m_bitmapHandle; }
bool WindowsBitmap::operator==(const WindowsBitmap& bitmap) const	{ return m_bitmapHandle == bitmap.m_bitmapHandle; }
bool WindowsBitmap::operator!=(const WindowsBitmap& bitmap) const	{ return m_bitmapHandle != bitmap.m_bitmapHandle; }

size_t WindowsBitmap::Width() const		{ return m_width; }
size_t WindowsBitmap::Height() const	{ return m_height; }

bool WindowsBitmap::_create(WindowsDeviceContext& wdc, size_t width, size_t height)
{
	HBITMAP res = CreateCompatibleBitmap(wdc._getraw_hdc(), (int)width, (int)height);
	if (!res)
		return false;

	m_bitmapHandle = res;
	m_width = width;
	m_height = height;
	return true;
}

void WindowsBitmap::_release()
{
	DeleteObject(m_bitmapHandle);
	m_bitmapHandle = nullptr;
	m_width = 0;
	m_height = 0;
}

bool WindowsBitmap::_created() const { return m_bitmapHandle; }
