#include "MyRenderTarget.h"
#include "MyDC.h"

MyRenderTarget::MyRenderTarget()
	: m_bitmapHandle(nullptr)
	, m_referenceCount(nullptr)
{
}

MyRenderTarget::MyRenderTarget(MyRenderTarget& rendertarget)
	: m_bitmapHandle(rendertarget.m_bitmapHandle)
	, m_referenceCount(rendertarget.m_referenceCount)
{
	if (_allocated())
		_refcount_up();
}

MyRenderTarget::MyRenderTarget(MyRenderTarget&& rendertarget)
	: m_bitmapHandle(rendertarget.m_bitmapHandle)
	, m_referenceCount(rendertarget.m_referenceCount)
{
	rendertarget._set_null();
}

MyRenderTarget::~MyRenderTarget()
{
	if (_allocated())
		Release();
}


MyRenderTarget& MyRenderTarget::operator=(MyRenderTarget& rendertarget)
{
	if (operator==(rendertarget))
		return *this;
	
	if (_allocated())
	{
		_refcount_down();
		if (!_refcount())
			_release();
	}

	m_bitmapHandle = rendertarget.m_bitmapHandle;
	m_referenceCount = rendertarget.m_referenceCount;
	if (_allocated())
		_refcount_up();

	return *this;
}

MyRenderTarget& MyRenderTarget::operator=(MyRenderTarget&& rendertarget)
{
	if (operator==(rendertarget))
		return *this;

	if (_allocated())
	{
		_refcount_down();
		if (!_refcount())
			_release();
	}

	m_bitmapHandle = rendertarget.m_bitmapHandle;
	m_referenceCount = rendertarget.m_referenceCount;
	rendertarget._set_null();
	return *this;
}


bool MyRenderTarget::operator==(const MyRenderTarget& rendertarget) const	{ return m_bitmapHandle == rendertarget.m_bitmapHandle; }
bool MyRenderTarget::operator!=(const MyRenderTarget& rendertarget) const	{ return m_bitmapHandle != rendertarget.m_bitmapHandle; }


MyRenderTarget::operator bool() const	{ return _allocated(); }
bool MyRenderTarget::operator!() const	{ return !_allocated(); }
MyRenderTarget::refcount_t MyRenderTarget::RefCount() const { return _refcount(); }


void MyRenderTarget::Allocation(DC_Base& dc, size_t sizeX, size_t sizeY)
{
	_allocation(dc.DeviceContextHandle(), sizeX, sizeY);
}

void MyRenderTarget::Release()
{
	_refcount_down();
	if (!_refcount())
		_release();
	_set_null();
}



HBITMAP MyRenderTarget::_get_primitive()	{ return m_bitmapHandle; }


bool MyRenderTarget::_allocated() const { return m_bitmapHandle != nullptr; }
MyRenderTarget::refcount_t& MyRenderTarget::_refcount()			{ return (*m_referenceCount); }
MyRenderTarget::refcount_t MyRenderTarget::_refcount() const	{ return (*m_referenceCount); }

void MyRenderTarget::_refcount_up()		{ ++_refcount(); }
void MyRenderTarget::_refcount_down()	{ --_refcount(); }

void MyRenderTarget::_allocation(HDC devicecontextHandle, size_t sizeX, size_t sizeY)
{
	m_bitmapHandle = CreateCompatibleBitmap(devicecontextHandle, sizeX, sizeY);
	m_referenceCount = new size_t(1);
}
void MyRenderTarget::_release()
{
	DeleteObject(m_bitmapHandle);
	delete m_referenceCount;
}
void MyRenderTarget::_set_null()
{
	m_bitmapHandle = nullptr;
	m_referenceCount = nullptr;
}
