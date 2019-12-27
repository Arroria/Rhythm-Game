#include <stdexcept>
#include "WindowsDeviceContext.h"
#include "WindowsObject.h"
#pragma comment (lib, "Msimg32.lib")

/*
private functions are not thrown exception
*/

WindowsDeviceContext::WindowsDeviceContext()
	: m_dcHandle(nullptr)
{
}
WindowsDeviceContext::WindowsDeviceContext(WindowsDeviceContext&& _mv_wdc) noexcept
	: m_dcHandle(_mv_wdc.m_dcHandle)
{
	_mv_wdc.m_dcHandle = nullptr;
}
WindowsDeviceContext::~WindowsDeviceContext()
{
}

WindowsDeviceContext& WindowsDeviceContext::operator=(WindowsDeviceContext&& _mv_wdc) noexcept
{
	m_dcHandle = _mv_wdc.m_dcHandle;
	_mv_wdc.m_dcHandle = nullptr;
	return *this;
}


bool WindowsDeviceContext::Created() const
{
	return _created();
}

bool WindowsDeviceContext::_created() const	{ return m_dcHandle; }
void WindowsDeviceContext::_exceptionThrow_created() const noexcept(false)		{ throw std::logic_error("is created"); }
void WindowsDeviceContext::_exceptionThrow_uncreated() const noexcept(false)	{ throw std::logic_error("is uncreated"); }



WDC_Screen::WDC_Screen()
	: WindowsDeviceContext()
	, m_windowHandle(nullptr)
{
}
WDC_Screen::WDC_Screen(WDC_Screen&& screenWDC) noexcept
	: WindowsDeviceContext(std::move(screenWDC))
	, m_windowHandle(screenWDC.m_windowHandle)
{
	screenWDC.m_windowHandle = nullptr;
}
WDC_Screen::~WDC_Screen()
{
	if (Created())
		_release();
}

WDC_Screen& WDC_Screen::operator=(WDC_Screen&& screenWDC) noexcept
{
	WindowsDeviceContext::operator=(std::move(screenWDC));
	m_windowHandle = screenWDC.m_windowHandle;
	screenWDC.m_windowHandle = nullptr;
	return *this;
}


bool WDC_Screen::Initialze(HWND windowHandle)
{
	if (Created())
	{
		if constexpr (_ISDEBUG())
			_exceptionThrow_created();
		else
			return false;
	}

	HDC getdcResult = GetDC(windowHandle);
	if (!getdcResult)
		return false;

	m_windowHandle = windowHandle;
	m_dcHandle = getdcResult;
	return true;
}

void WDC_Screen::Release()
{
	if (!Created())
	{
		//if constexpr (_DEBUG)
		//	_exceptionThrow_uncreated();
		//else
			return;
	}

	_release();
}

void WDC_Screen::_release()		{ ReleaseDC(m_windowHandle, m_dcHandle); m_dcHandle = nullptr; m_windowHandle = nullptr; }



WDC_Memory::WDC_Memory()
	: WindowsDeviceContext()
	, m_dcDefBitmapHandle(nullptr)
	, m_customBitmap(nullptr)
{
}

WDC_Memory::WDC_Memory(WDC_Memory&& memoryWDC) noexcept
	: WindowsDeviceContext(std::move(memoryWDC))
	, m_dcDefBitmapHandle(memoryWDC.m_dcDefBitmapHandle)
	, m_customBitmap(memoryWDC.m_customBitmap)
{
	memoryWDC.m_dcDefBitmapHandle = nullptr;
	memoryWDC.m_customBitmap = nullptr;
}

WDC_Memory::~WDC_Memory()
{
	if (_created())
		_release();
}


WDC_Memory& WDC_Memory::operator=(WDC_Memory&& memoryWDC) noexcept
{
	m_dcHandle = memoryWDC.m_dcHandle;
	m_dcDefBitmapHandle = memoryWDC.m_dcDefBitmapHandle;
	m_customBitmap = std::move(memoryWDC.m_customBitmap);
	memoryWDC.m_dcHandle = nullptr;
	memoryWDC.m_dcDefBitmapHandle = nullptr;
	return *this;
}

bool WDC_Memory::Initialze(WDC_Screen& baseWDCScreen)
{
	if (Created())
	{
		if constexpr (_ISDEBUG())
			_exceptionThrow_created();
		else
			return false;
	}

	HDC getdcResult = CreateCompatibleDC(baseWDCScreen._getraw_hdc());
	if (!getdcResult)
		return false;

	m_dcHandle = getdcResult;
	m_dcDefBitmapHandle = (HBITMAP)GetCurrentObject(m_dcHandle, OBJ_BITMAP);
	if (m_customBitmap && m_customBitmap->Created())
		_regist_wdc_with_bitmap_custom();
	return true;
}

void WDC_Memory::Release()
{
	if (!Created())
	{
		//if constexpr (_DEBUG)
		//	_exceptionThrow_created();
		//else
			return;
	}
	_release();
}

void WDC_Memory::RegistBitmap(nullptr_t)				{ _bitmap_set_null(); }
void WDC_Memory::RegistBitmap(Bitmap_ptr_t bitmap_ptr)	{ _bitmap_set(bitmap_ptr); }
void WDC_Memory::UnregistBitmap()						{ _bitmap_set_null(); }

WDC_Memory::Bitmap_ptr_t WDC_Memory::RegistedBitmap()				{ return m_customBitmap; }
const WDC_Memory::Bitmap_ptr_t& WDC_Memory::RegistedBitmap() const	{ return m_customBitmap; }


void WDC_Memory::_release()
{
	DeleteDC(m_dcHandle);
	m_dcHandle = nullptr;
}

void WDC_Memory::_bitmap_set_null()
{
	m_customBitmap = nullptr;
	if (_created())
		_regist_wdc_with_bitmap_default();
}
void WDC_Memory::_bitmap_set(Bitmap_ptr_t bitmap_ptr)
{
	if (bitmap_ptr == m_customBitmap)
		return;
	if (!bitmap_ptr->Created())
		_bitmap_set_null();
	else
	{
		m_customBitmap = std::move(bitmap_ptr);
		_regist_wdc_with_bitmap_custom();
	}
}

void WDC_Memory::_regist_wdc_with_bitmap_default()	{ SelectObject(m_dcHandle, m_dcDefBitmapHandle); }
void WDC_Memory::_regist_wdc_with_bitmap_custom()	{ SelectObject(m_dcHandle, m_customBitmap->_getraw_hbitmap()); }
