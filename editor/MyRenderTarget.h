#pragma once
#include "mywinapi_default_include.h"

class DC_Base;
class MyRenderTarget
{
public:
	using refcount_t = size_t;

	MyRenderTarget();
	MyRenderTarget(MyRenderTarget& rendertarget);
	MyRenderTarget(MyRenderTarget&& rendertarget);
	~MyRenderTarget();

	MyRenderTarget& operator=(MyRenderTarget& rendertarget);
	MyRenderTarget& operator=(MyRenderTarget&& rendertarget);

	bool operator==(const MyRenderTarget& rendertarget) const;
	bool operator!=(const MyRenderTarget& rendertarget) const;


	operator bool() const;
	bool operator!() const;
	refcount_t RefCount() const;


	void Allocation(DC_Base& dc, size_t sizeX, size_t sizeY);
	void Release();

	HBITMAP _get_primitive();


private:
	HBITMAP m_bitmapHandle;
	refcount_t* m_referenceCount;

	bool _allocated() const;

	refcount_t& _refcount();
	refcount_t _refcount() const;
	void _refcount_up();
	void _refcount_down();

	void _allocation(HDC devicecontextHandle, size_t sizeX, size_t sizeY);
	void _release();
	void _set_null();
};
