#pragma once
#include "windefinclude.h"
#include <stdexcept>
#include <memory>
class WindowsBitmap;


class WindowsDeviceContext
{
public:
	WindowsDeviceContext();
	virtual ~WindowsDeviceContext();

protected:
	WindowsDeviceContext(WindowsDeviceContext&) = delete;
	WindowsDeviceContext(WindowsDeviceContext&& _mv_wdc) noexcept;
	WindowsDeviceContext& operator=(WindowsDeviceContext&) = delete;
	WindowsDeviceContext& operator=(WindowsDeviceContext&& _mv_wdc) noexcept;


public:
	virtual void Release() = 0; // for control refCount

	bool Created() const;

	HDC _getraw_hdc() { return m_dcHandle; }	// get data raw // Caution when using

protected:
	bool _created() const;
	void _exceptionThrow_created() const noexcept(false);
	void _exceptionThrow_uncreated() const noexcept(false);

protected:
	HDC m_dcHandle;
};
using WDC_Base = WindowsDeviceContext;


class WDC_Screen
	: public WDC_Base
{
public:
	WDC_Screen();
	WDC_Screen(WDC_Screen&) = delete;
	WDC_Screen(WDC_Screen&& screenWDC) noexcept;
	~WDC_Screen();

	WDC_Screen& operator=(WDC_Screen&) = delete;
	WDC_Screen& operator=(WDC_Screen&& screenWDC) noexcept;


public:
	bool Initialze(HWND windowHandle);
	void Release() override;

	HWND _getraw_hwnd() { return m_windowHandle; }	// get data raw // Caution when using

private:
	void _release();

private:
	HWND m_windowHandle;
};


class WDC_Memory
	: public WDC_Base
{
public:
	using Bitmap_ptr_t = std::shared_ptr<WindowsBitmap>;

	WDC_Memory();
	WDC_Memory(WDC_Memory&) = delete;
	WDC_Memory(WDC_Memory&& memoryWDC) noexcept;
	~WDC_Memory();

	WDC_Memory& operator=(WDC_Memory&) = delete;
	WDC_Memory& operator=(WDC_Memory&& memoryWDC) noexcept;


	bool Initialze(WDC_Screen& baseWDCScreen);
	void Release() override;


	void RegistBitmap(nullptr_t);
	void RegistBitmap(Bitmap_ptr_t bitmap_ptr);
	void UnregistBitmap();
	Bitmap_ptr_t RegistedBitmap();
	const Bitmap_ptr_t& RegistedBitmap() const;

private:
	void _release();

	void _bitmap_set_null();
	void _bitmap_set(Bitmap_ptr_t bitmap_ptr);
	void _regist_wdc_with_bitmap_default();
	void _regist_wdc_with_bitmap_custom();

private:
	HBITMAP m_dcDefBitmapHandle;
	Bitmap_ptr_t m_customBitmap;
};
