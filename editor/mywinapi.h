#pragma once
#include "MyDC.h"
#include "MyRenderTarget.h"

inline void DC_FillRect(DC_Base& dc, int left, int top, int right, int bottom, HBRUSH brush)
{
	RECT temp{ left, top, right, bottom };
	FillRect(dc.DeviceContextHandle(), &temp, brush);
}
