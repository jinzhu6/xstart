#ifndef _DS_UTILS_H_
#define _DS_UTILS_H_

#include "corela.h"
#include <stdio.h>
#include <DShow.h>
#include <Windows.h>
#include "Qedit.h"

#pragma comment(lib, "strmiids")

#define SAFE_RELEASE(x) {if(x){x->Release();x=0;}}


void _FreeMediaType(AM_MEDIA_TYPE &mt);
void DeleteMediaType(AM_MEDIA_TYPE* pmt);
HRESULT GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin);
HRESULT ConnectFilters(IGraphBuilder* pGraph, IPin* pOut, IBaseFilter* pDest);
HRESULT ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pSrc, IBaseFilter* pDest);
bool filterHasCompatibleOutputPin(IBaseFilter* pFilter, const GUID* mediaType, IPin** ppPin);
IPin* GetPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir);
coBool DSGrabberGetImage(ISampleGrabber* _grabber, IMAGE* image, coDword* _width, coDword* _height);
IMoniker* GetDeviceMoniker(int index, const wchar_t* deviceName, char* nameOut, REFGUID category = CLSID_VideoInputDeviceCategory);
IPin* EnumPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, int index = 0);
void PrintFilter(IBaseFilter* pFilter);
void ListPins(IBaseFilter* pFilter);


#endif
