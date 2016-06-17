#ifdef _MSC_BUILD

#include "ds_utils.h"
#include "comdef.h"
#include <string>

#define VideoControl_PowerFrequency 13
#define VideoControl_Brightness 100
#define VideoControl_Contrast 101
#define VideoControl_Hue 102
#define VideoControl_Saturation 103
#define VideoControl_Sharpness 104
#define VideoControl_Gamma 105
#define VideoControl_Colorenable 106
#define VideoControl_WhiteBalance 107
#define VideoControl_BacklightCompensation 108
#define VideoControl_PropertiesGain 109

typedef struct CAPTURE_DEVICE {
	IGraphBuilder* _graph;
	IMediaControl* _mediaControl;
	IMediaPosition* _mediaPosition;
	ISampleGrabber* _grabber;
	IBaseFilter* pGrab;
	IBaseFilter* pCap;
} CAPTURE_DEVICE;


#define COMCHECK(info,hr) { if(hr!=S_OK) { _com_error com_err(hr); const char* com_error = (char*)com_err.ErrorMessage(); Log(LOG_FATAL, "%s - COM error in %s on line %d: %s", info, __FILE__, __LINE__, com_error ); } }

std::string utf8_encode(const std::wstring &wstr) {
	if( wstr.empty() ) { return std::string(); }
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo( size_needed, 0 );
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

/* -------------------------------------------------------------------------------------------------------------------------
** CaptureRelease
** ---------------------------------------------------------------------------------------------------------------------- */
void CaptureRelease(void* _device) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	/*if(device->_graph) while(device->_graph->Release()) {  };
	if(device->_mediaControl) while(device->_mediaControl->Release()) {  };
	if(device->_mediaPosition) while(device->_mediaPosition->Release()) {  };
	if(device->_grabber) while(device->_grabber->Release()) {  };
	if(device->pGrab) while(device->pGrab->Release()) {  };
	if(device->pCap) while(device->pCap->Release()) {  };*/
	free(device);
}


/* -------------------------------------------------------------------------------------------------------------------------
** CaptureDevice
** ---------------------------------------------------------------------------------------------------------------------- */
void* CaptureDevice(int index, const wchar_t* deviceName, int width, int height, int bits) {
	HRESULT hr;

	// Initialize COM
	CoInitialize(NULL);

	// Find device moniker
	char foundName[1024];
	IMoniker* pMoniker = GetDeviceMoniker(index, deviceName, foundName);
	if(pMoniker == NULL) {
		if(deviceName) { Log(LOG_FATAL, "Capture device '%s' (index %d) not found!", deviceName, index);}
		else { Log(LOG_FATAL, "Capture device with index %d not found!", index); }
		return 0;
	}

	// Create capture device object
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)malloc(sizeof(CAPTURE_DEVICE));
	memset(device, 0, sizeof(CAPTURE_DEVICE));

	// create graph
	hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&device->_graph);	COMCHECK(foundName, hr);
	if(FAILED(hr)) { CaptureRelease(device); return 0; }

	// obtain interface for media control
	hr = device->_graph->QueryInterface(IID_IMediaControl,(LPVOID*) &device->_mediaControl);	COMCHECK(foundName, hr);
	if(FAILED(hr)) { CaptureRelease(device); return 0; }

	// create BaseFilter for moniker
	hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&device->pCap);	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }

	// set format for device filter
	IPin* pin = GetPin(device->pCap, PINDIR_OUTPUT);
	IAMStreamConfig* pConfig = NULL;
	hr = pin->QueryInterface(IID_IAMStreamConfig, (void**)&pConfig);	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }

	// find and set format/resolution
	int count = 0, size = 0;
	char selectedMode[256];
	strcpy(selectedMode, "NO USABLE MODE FOUND!");
	hr = pConfig->GetNumberOfCapabilities(&count, &size);	COMCHECK(foundName, hr);
	Log(LOG_INFO, "Modes: ");
	bool modeFound = false;
	for(int i=0; i<count; i++) {
		AM_MEDIA_TYPE* mediatype;
		VIDEO_STREAM_CONFIG_CAPS caps;
		hr = pConfig->GetStreamCaps(i, &mediatype, (BYTE*)&caps);

		VIDEOINFOHEADER* vidheader = reinterpret_cast<VIDEOINFOHEADER*>(mediatype->pbFormat);
		BITMAPINFOHEADER* bmiHeader = &vidheader->bmiHeader;

		if(bmiHeader->biWidth != 0 && bmiHeader->biHeight != 0 && bmiHeader->biBitCount != 0) {
			Log(LOG_NOBREAK, "[%dx%d@%d] ", bmiHeader->biWidth, bmiHeader->biHeight, bmiHeader->biBitCount);
		}

		if(width > 0 && height > 0) {
			if( bmiHeader->biWidth == width  &&  bmiHeader->biHeight == height/* && bmiHeader->biBitCount == 24*/) {
				if( bits == 0  ||  bmiHeader->biBitCount == bits ) {
					modeFound = true;
					pConfig->SetFormat(mediatype);
					sprintf(selectedMode, "Using: [%dx%d@%d]", bmiHeader->biWidth, bmiHeader->biHeight, bmiHeader->biBitCount);
				}
			}
		}

		DeleteMediaType(mediatype);
	}
	if(modeFound) { Log(LOG_INFO, selectedMode); }
	else { Log(LOG_FATAL, "Mode (%dx%dx%d) not found for camera!", width, height, bits); }

	// create sample grabber
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_ISampleGrabber, (void**)&device->_grabber);	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }

	// create BaseFilter for grabber
	hr = device->_grabber->QueryInterface(IID_IBaseFilter, (void**)&device->pGrab);	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }

	// set media type for sample grabber
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_ARGB32; // MEDIASUBTYPE_RGB24
	hr = device->_grabber->SetMediaType(&mt);	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }

	// set grabber to one-shot mode
	device->_grabber->SetBufferSamples(TRUE);
	device->_grabber->SetOneShot(TRUE);

	// add filters to graph
	hr = device->_graph->AddFilter(device->pCap, L"Capture Filter");	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }
	hr = device->_graph->AddFilter(device->pGrab, L"Sample Grabber");	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }

	// connect filters
	IEnumPins* pEnum = NULL;
	IPin* pPin = NULL;
	hr = device->pCap->EnumPins(&pEnum);	COMCHECK(foundName, hr);
	if (FAILED(hr)) { CaptureRelease(device); return 0; }

	while (S_OK == pEnum->Next(1, &pPin, NULL)) {
		hr = ConnectFilters(device->_graph, pPin, device->pGrab);
		if(pPin != NULL) { pPin->Release(); }
		if (SUCCEEDED(hr)) { break; }
	}

	// create preview pin filter
//	hr = _captureBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCap, NULL, NULL);
//	hr = _captureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pGrab, NULL, NULL);
	hr = device->_mediaControl->Run();	COMCHECK(foundName,hr);
	if(hr != S_OK) { CaptureRelease(device); return 0; }

	return (void*)device;
}


/* -------------------------------------------------------------------------------------------------------------------------
** CaptureConfig
** ---------------------------------------------------------------------------------------------------------------------- */
void CaptureConfig(void* _device) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IBaseFilter* pFilter = device->pCap;
	ISpecifyPropertyPages* pProp;
	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pProp);
	if (SUCCEEDED(hr)) {
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo);
		IUnknown* pFilterUnk;
		pFilter->QueryInterface(IID_IUnknown, (void**)&pFilterUnk);

		// Show the page.
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		pProp->Release();
		OleCreatePropertyFrame(
		    GetActiveWindow(),      // Parent window
		    0, 0,                   // Reserved
		    FilterInfo.achName,     // Caption for the dialog box
		    1,                      // Number of objects (just the filter)
		    &pFilterUnk,            // Array of object pointers.
		    caGUID.cElems,          // Number of property pages
		    caGUID.pElems,          // Array of property page CLSIDs
		    0,                      // Locale identifier
		    0, NULL                 // Reserved
		);

		// Clean up.
		pFilterUnk->Release();
		FilterInfo.pGraph->Release();
		CoTaskMemFree(caGUID.pElems);
	}
}


/* -------------------------------------------------------------------------------------------------------------------------
** CaptureGetImage
** ---------------------------------------------------------------------------------------------------------------------- */
coBool CaptureGetImage(void* _device, IMAGE* image, coDword* _width, coDword* _height) {
	HRESULT hr;
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	if(!device) { Log(LOG_ERROR, "Device is null in CaptureGetImage()!"); return coFalse; }

	// get media information
	AM_MEDIA_TYPE mt;
	hr = device->_grabber->GetConnectedMediaType(&mt);
	if( mt.formattype != FORMAT_VideoInfo || mt.cbFormat < sizeof(VIDEOINFOHEADER) || mt.pbFormat == NULL) {
		Log(LOG_ERROR, "CaptureGetImage() failed, media-type is not a video stream!");
		return coFalse;
	}

	// get capture dimensions
	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
	coDword width = pVih->bmiHeader.biWidth;
	coDword height = pVih->bmiHeader.biHeight;
	if(_width) {
		*_width = width;
	}
	if(_height) {
		*_height = height;
	}
	CoTaskMemFree((void*)mt.pbFormat);

	// check that image has the correct dimensions
	if(!image) { return coTrue; }
	if(image->width != width || image->height != height) {
		Log(LOG_ERROR, "CaptureGetImage() failed! Image has different resolution (%d, %d) as the camera (%d, %d).", image->width, image->height, width, height);
		return false;
	}

	// get buffer size
	long bufferSize = 0;
	hr = device->_grabber->GetCurrentBuffer(&bufferSize, NULL);
	if (FAILED(hr))	{ Log(LOG_ERROR, "CaptureGetImage() failed in GetCurrentBuffer()!");  return coFalse; }

	// read buffer into image
	hr = device->_grabber->GetCurrentBuffer(&bufferSize, (long*)image->data);
	if (FAILED(hr))	{ Log(LOG_ERROR, "CaptureGetImage() failed in GetCurrentBuffer2()!");  return coFalse; }

	return coTrue;
}


void CaptureLogPropertyRange(IAMVideoProcAmp* pControl, long prop, char* error, int val) {
	long min, max, step, def, flags;
	if(!pControl) { Log(LOG_ERROR, "%s!", error); return; }
	HRESULT hr = pControl->GetRange(prop, &min, &max, &step, &def, &flags);
	if(hr != S_OK) { Log(LOG_ERROR, "%s", error); return; }
	Log(LOG_ERROR, "%s ... %d is not valid!\nPossible values - Range: %d - %d;  Steps: %d;  Default: %d", error, val, min, max, step, def);
}


void CaptureLogCameraPropertyRange(IAMCameraControl* pControl, long prop, char* error, int val) {
	long min, max, step, def, flags;
	if(!pControl) { Log(LOG_ERROR, "%s!", error); return; }
	HRESULT hr = pControl->GetRange(prop, &min, &max, &step, &def, &flags);
	if(hr != S_OK) { Log(LOG_ERROR, "%s", error); return; }
	Log(LOG_ERROR, "%s ... %d is not valid!\nPossible values - Range: %d - %d;  Steps: %d;  Default: %d", error, val, min, max, step, def);
}



void CaptureSetFocus(void* _device, int focus) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMCameraControl* pCameraControl;
	HRESULT hr;
	hr = device->pCap->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (hr == S_OK) {
		long defaultFocusValue;
		if(focus < 0) {
			hr = pCameraControl->Set(CameraControl_Focus, 0, CameraControl_Flags_Auto);
		} else {
			hr = pCameraControl->Set(CameraControl_Focus, focus, CameraControl_Flags_Manual);
		}
	}
	if(hr != S_OK) { CaptureLogCameraPropertyRange(pCameraControl, CameraControl_Focus, "Error while calling CaptureSetFocus()", focus); }
}


void CaptureSetZoom(void* _device, int zoom) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMCameraControl* pCameraControl;
	HRESULT hr = device->pCap->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (hr == S_OK) {
		hr = pCameraControl->Set(CameraControl_Zoom, zoom, CameraControl_Flags_Manual);
	}
	if(hr != S_OK) { CaptureLogCameraPropertyRange(pCameraControl, CameraControl_Zoom, "Error while calling CaptureSetZoom()", zoom); }
}


void CaptureSetExposure(void* _device, int exp) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMCameraControl* pCameraControl;
	HRESULT hr = device->pCap->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (hr == S_OK) {
		hr = pCameraControl->Set(CameraControl_Exposure, exp, CameraControl_Flags_Manual);
		if(exp <= -1000 || exp >= 1000 || hr != S_OK) {
			Log(LOG_ERROR, "Failed setting camera exposure to %d.", exp);
			hr = pCameraControl->Set(CameraControl_Exposure, exp, CameraControl_Flags_Auto);
		}
	}
	if(hr != S_OK) { CaptureLogCameraPropertyRange(pCameraControl, CameraControl_Exposure, "Error while calling CaptureSetExposure()", exp); }
}


void CaptureSetWhiteBalance(void* _device, int balance) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMVideoProcAmp* pControl;
	HRESULT hr = device->pCap->QueryInterface(IID_IAMVideoProcAmp, (void**)&pControl);
	if (hr == S_OK) {
		if(balance > 0) {
			hr = pControl->Set(VideoProcAmp_WhiteBalance, balance, CameraControl_Flags_Manual);
		} else {
			hr = pControl->Set(VideoProcAmp_WhiteBalance, balance, CameraControl_Flags_Auto);
		}
	}
	if(hr != S_OK) { CaptureLogPropertyRange(pControl, VideoProcAmp_WhiteBalance, "Error while calling CaptureSetWhiteBalance()", balance); }
}


void CaptureSetBrightness(void* _device, int brightness) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMVideoProcAmp* pControl;
	HRESULT hr = device->pCap->QueryInterface(IID_IAMVideoProcAmp, (void**)&pControl);
	if (hr == S_OK) {
		if(brightness > 0) {
			hr = pControl->Set(VideoProcAmp_Brightness, brightness, CameraControl_Flags_Manual);
		} else {
			hr = pControl->Set(VideoProcAmp_Brightness, brightness, CameraControl_Flags_Auto);
		}
	}
	if(hr != S_OK) { CaptureLogPropertyRange(pControl, VideoProcAmp_Brightness, "Error while calling CaptureSetBrightness()", brightness); }
}


void CaptureSetContrast(void* _device, int contrast) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMVideoProcAmp* pControl;
	HRESULT hr = device->pCap->QueryInterface(IID_IAMVideoProcAmp, (void**)&pControl);
	if (hr == S_OK) {
		if(contrast > 0) {
			hr = pControl->Set(VideoProcAmp_Contrast, contrast, CameraControl_Flags_Manual);
		} else {
			hr = pControl->Set(VideoProcAmp_Contrast, contrast, CameraControl_Flags_Auto);
		}
	}
	if(hr != S_OK) { CaptureLogPropertyRange(pControl, VideoProcAmp_Contrast, "Error while calling CaptureSetContrast()", contrast); }
}


void CaptureSetSaturation(void* _device, int saturation) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMVideoProcAmp* pControl = 0;
	HRESULT hr = device->pCap->QueryInterface(IID_IAMVideoProcAmp, (void**)&pControl);
	if (hr == S_OK) {
		if(saturation > 0) {
			hr = pControl->Set(VideoProcAmp_Saturation, saturation, CameraControl_Flags_Manual);
		} else {
			hr = pControl->Set(VideoProcAmp_Saturation, saturation, CameraControl_Flags_Auto);
		}
	}
	if(hr != S_OK) { CaptureLogPropertyRange(pControl, VideoProcAmp_Saturation, "Error while calling CaptureSetSaturation()", saturation); }
}


void CaptureSetPowerlineFrequency(void* _device, int freq) {
	CAPTURE_DEVICE* device = (CAPTURE_DEVICE*)_device;
	IAMVideoProcAmp* pControl;
	HRESULT hr = device->pCap->QueryInterface(IID_IAMVideoProcAmp, (void**)&pControl);
	if (hr == S_OK) {
		if(freq > 0) {
			hr = pControl->Set(13, freq, CameraControl_Flags_Manual);
		} else {
			hr = pControl->Set(13, freq, CameraControl_Flags_Auto);
		}
	}
	if(hr != S_OK) { CaptureLogPropertyRange(pControl, 13, "Error while calling CaptureSetPowerlineFrequency()", freq); }
}


#endif
