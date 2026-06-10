#include "stdafx.h"
#pragma hdrstop
#include "dx10EventWrapper.h"
#include "../xrRender/HW.h"

#if !defined(USE_DX12)
dxPixEventWrapper::dxPixEventWrapper(LPCWSTR wszName)
{
    if (HW.pAnnotation)
        HW.pAnnotation->BeginEvent(wszName);
}

dxPixEventWrapper::~dxPixEventWrapper()
{
    if (HW.pAnnotation)
        HW.pAnnotation->EndEvent();
}
#endif // !USE_DX12 