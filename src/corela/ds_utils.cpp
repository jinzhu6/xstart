#ifdef _MSC_BUILD

#include <DShow.h>
#include "corela.h"

#include <Windows.h>
#include "Qedit.h"
#include <stdio.h>

#pragma comment(lib, "strmiids")


// Some of this stuff comes from here: http://msdn.microsoft.com/en-us/library/windows/desktop/dd407288(v=vs.85).aspx



coBool DSGrabberGetImage(ISampleGrabber* _grabber, IMAGE* image, coDword* _width, coDword* _height) {
	HRESULT hr;

	// get media information
	AM_MEDIA_TYPE mt;
	hr = _grabber->GetConnectedMediaType(&mt);
	if( mt.formattype != FORMAT_VideoInfo || mt.cbFormat < sizeof(VIDEOINFOHEADER) || mt.pbFormat == NULL) {
		Log(LOG_ERROR, "DSGrabberGetImage() failed, media-type is not a video stream!");
		return coFalse;
	}

	// get capture dimensions
	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
	coDword width = pVih->bmiHeader.biWidth;
	coDword height = pVih->bmiHeader.biHeight;
	if(_width) { *_width = width; }
	if(_height) { *_height = height; }
	CoTaskMemFree((void*)mt.pbFormat);

	// check that image has the correct dimensions
	if(!image) { return false; }
	if(image->width != width || image->height != height) {
		Log(LOG_ERROR, "DSGrabberGetImage() failed! Image has different resolution (%d, %d) as the camera (%d, %d).", image->width, image->height, width, height);
		return false;
	}

	// get buffer size
	long bufferSize = 0;
	hr = _grabber->GetCurrentBuffer(&bufferSize, NULL);
	if (FAILED(hr))	{ return coFalse; }

	// read buffer into image
	hr = _grabber->GetCurrentBuffer(&bufferSize, (long*)image->data);
	if (FAILED(hr))	{ return coFalse; }

	return coTrue;
}




// Delete a media type structure that was allocated on the heap.
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd375432%28v=vs.85%29.aspx
void _FreeMediaType(AM_MEDIA_TYPE &mt) {
	if (mt.cbFormat != 0) {
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) {
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}
void DeleteMediaType(AM_MEDIA_TYPE* pmt) {
	if (pmt != NULL) {
		_FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}

// Find an Unconnected Pin on a Filter
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/findanunconnectedpinonafilter.asp
HRESULT GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin) {
	*ppPin = 0;
	IEnumPins* pEnum = 0;
	IPin* pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) {
		return hr;
	}

	while (pEnum->Next(1, &pPin, NULL) == S_OK) {
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PinDir) {
			IPin* pTmp = 0;
			hr = pPin->ConnectedTo(&pTmp);
			// Already connected, not the pin we want.
			if (SUCCEEDED(hr)) {
				pTmp->Release();
			}
			// Unconnected, this is the pin we want.
			else {
				pEnum->Release();
				*ppPin = pPin;
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	// Did not find a matching pin.
	return E_FAIL;
}

// Connect a Pin To a Filter
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/connecttwofilters.asp
HRESULT ConnectFilters(IGraphBuilder* pGraph, IPin* pOut, IBaseFilter* pDest) {
	if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL)) {
		return E_POINTER;
	}

	// Find an input pin on the downstream filter.
	IPin* pIn = 0;
	HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (FAILED(hr)) {
		return hr;
	}

	// Try to connect them.
	hr = pGraph->Connect(pOut, pIn);
	pIn->Release();
	return hr;
}

// Connect Two Filters
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/connecttwofilters.asp
HRESULT ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pSrc, IBaseFilter* pDest) {
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL)) {
		return E_POINTER;
	}

	// Find an output pin on the first filter.
	IPin* pOut = 0;
	HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr)) {
		return hr;
	}

	// Try to connect them.
	hr = ConnectFilters(pGraph, pOut, pDest);
	pOut->Release();
	return hr;
}

// http://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/f272cea3-5019-4b7a-978b-ebfcfd230cbd/adding-sample-grabber-and-null-renderer-to-a-filter-graph
bool filterHasCompatibleOutputPin(IBaseFilter* pFilter, const GUID* mediaType, IPin** ppPin) {
	IEnumPins*         pPinEnum = NULL;
	IEnumMediaTypes*   pMtypeEnum = NULL;
	IPin*              pPin = NULL;
	PIN_DIRECTION     pinDir;
	AM_MEDIA_TYPE*     amMediaType = NULL;
	unsigned long     cFetched;
	HRESULT           hr = S_OK;
	bool              retval = false;

	//========================================================================
	// get the enumeration list of pins on the passed filter and loop on them
	//========================================================================
	if (SUCCEEDED(hr = pFilter->EnumPins (&pPinEnum))) {
		while ((hr = pPinEnum->Next(1, &pPin, &cFetched)) == S_OK) {
			//===============================================================
			// Query for info - get direction and media types
			//===============================================================
			pPin->QueryDirection (&pinDir);
			if (pinDir == PINDIR_OUTPUT) {
				if (SUCCEEDED(hr = pPin->EnumMediaTypes (&pMtypeEnum))) {
					while (hr = pMtypeEnum->Next(1, &amMediaType, &cFetched) == S_OK) {
						if (amMediaType->majortype == *mediaType) {
							retval = true;
						}

						//DeleteMediaType(amMediaType);
						//if (retval == true)
						//   break;
					}
				}
			}

			//===============================================================
			// if we found appropriate pin, decide whether to return it or
			// release it.  Otherwise, release it always.
			//===============================================================

			if (retval == true) {
				if (ppPin != NULL) {
					*ppPin = pPin;
				} else {
					pPin->Release();
				}
				break;
			} else {
				pPin->Release();
			}
		}
	}

	return retval;
}

//
void ListPins(IBaseFilter* pFilter) {
	IEnumPins*         pPinEnum = NULL;
	IEnumMediaTypes*   pMtypeEnum = NULL;
	IPin*              pPin = NULL;
	PIN_DIRECTION     pinDir;
	AM_MEDIA_TYPE*     amMediaType = NULL;
	unsigned long     cFetched;
	HRESULT           hr = S_OK;
	bool              retval = false;

	PIN_INFO pinInfo;
	char pinName[256];

	//========================================================================
	// get the enumeration list of pins on the passed filter and loop on them
	//========================================================================
	if(SUCCEEDED(hr = pFilter->EnumPins(&pPinEnum))) {
		while((hr = pPinEnum->Next(1, &pPin, &cFetched)) == S_OK) {
			//===============================================================
			// Query for info - get direction and media types
			//===============================================================
			pPin->QueryPinInfo(&pinInfo);
			wcstombs(pinName, pinInfo.achName, 255);
			Log(LOG_INFO, "Pin '%s' is '%s' ... ", pinName, pinInfo.dir==PINDIR_OUTPUT?"OUTPUT":"INPUT");
			if(SUCCEEDED(hr = pPin->EnumMediaTypes(&pMtypeEnum))) {
				while(hr = pMtypeEnum->Next(1, &amMediaType, &cFetched) == S_OK) {
					Log(LOG_INFO, " ... supports media format %d (%d)", amMediaType->majortype, amMediaType->subtype);
				}
			}
			pPin->Release();
		}
	}
}

// http://www.laganiere.name/directshowTut/section2.shtml
IPin* GetPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir) {
	BOOL       bFound = FALSE;
	IEnumPins*  pEnum;
	IPin*       pPin;

	pFilter->EnumPins(&pEnum);
	while(pEnum->Next(1, &pPin, 0) == S_OK) {
		PIN_DIRECTION PinDirThis;
		pPin->QueryDirection(&PinDirThis);
		if (bFound = (PinDir == PinDirThis)) {
			break;
		}
		pPin->Release();
	}

	pEnum->Release();
	return (bFound ? pPin : 0);
}

IPin* EnumPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, int index = 0) {
	IEnumPins* pEnum;
	IPin* pPin;
	IPin* pFound = NULL;

	pFilter->EnumPins(&pEnum);
	while(pEnum->Next(1, &pPin, 0) == S_OK) {
		PIN_DIRECTION PinDirThis;
		pPin->QueryDirection(&PinDirThis);

		if(PinDir == PinDirThis) {
			if(index == 0) {
				pEnum->Release();
				return pPin;
			}
			index--;
		}

		pPin->Release();
	}

	pEnum->Release();
	return 0;
}

IPin* GetPinConnectedTo(IBaseFilter* filter, PIN_DIRECTION pinDir) {
	IPin* inPin;
	IPin* outPin;

	inPin = GetPin(filter, pinDir);
	if(!inPin) { return NULL; }
	if(FAILED(inPin->ConnectedTo(&outPin))) { inPin->Release(); return NULL; }

	inPin->Release();
	return outPin;
}

/* -------------------------------------------------------------------------------------------------------------------------
** GetDeviceMoniker - Finds device and returns moniker
** ---------------------------------------------------------------------------------------------------------------------- */
IMoniker* GetDeviceMoniker(int index, const wchar_t* deviceName, char* nameOut, REFGUID category = CLSID_VideoInputDeviceCategory) {
	IMoniker* pFound = NULL;

	// create the System Device Enumerator
	ICreateDevEnum* pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	IEnumMoniker* pEnum = 0;
	if (SUCCEEDED(hr)) {
		// create an enumerator for the category
		hr = pDevEnum->CreateClassEnumerator(category, &pEnum, 0);
		if (hr == S_FALSE) {
			return coFalse;
		}
		pDevEnum->Release();

		// enumerate monikers
		Log(LOG_INFO, "Selecting camera ...");
		IMoniker* pMoniker = NULL;
		int n = 0;
		while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

			// get properties from moniker (to get a friendly name further below)
			IPropertyBag* pPropBag;
			HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
			if (FAILED(hr)) {
				pMoniker->Release();
				continue;
			}

			// Get description or friendly name.
			VARIANT var;
			VariantInit(&var);
			hr = pPropBag->Read(L"Description", &var, 0);
			if (FAILED(hr)) {
				hr = pPropBag->Read(L"FriendlyName", &var, 0);
			}

			// check match
			if (SUCCEEDED(hr)) {
				if(pFound == 0) {

					if(deviceName) {
						// compare name with search name
						char name[512];
						char search[512];
						wcstombs(name, (wchar_t*)var.bstrVal, 510);
						wcstombs(search, (wchar_t*)deviceName, 510);
						if(strncmp(name, search, strlen(search)) == 0 || strlen(search) == 0) {
							if(index == 0) {
								pFound = pMoniker;
							} else {
								index--;
							}
						}
					} else {
						if(index == 0) {
							pFound = pMoniker;
						} else {
							index--;
						}
					}
				}

				if(pFound == pMoniker) {
					Log(LOG_INFO, " %d. %S [selected]", n, var.bstrVal);
					if(nameOut) { sprintf(nameOut, "%S (%d)", var.bstrVal, n); }
				} else {
					Log(LOG_INFO, " %d. %S", n, var.bstrVal);
				}
				n++;

				VariantClear(&var);
			}

			// release properties
			pPropBag->Release();
			if(pFound != pMoniker) {
				pMoniker->Release();
			}
		}
		pEnum->Release();
	}

	return pFound;
}


/*GUID GetFilterMediaType(IBaseFilter* filter) {
	IPin* inPin;
	AM_MEDIA_TYPE mt;

	inPin = GetPin(filter, PINDIR_INPUT);
	if(!inPin) { return 0; }
	if(FAILED(inPin->ConnectionMediaType(&mt))) { return 0; }

	GUID ret = mt.majortype;

}

void PrintFilter(IBaseFilter* pFilter) {
	char temp[256];

	Log(LOG_INFO, "Filter Info");

//	Log(LOG_INFO, "Output pins:");
	for(int i=0; true; i++) {
		IPin* pin = EnumPin(pFilter, PINDIR_OUTPUT, i);
		if(!pin) { break; }

		PIN_INFO pinInfo;
		pin->QueryPinInfo(&pinInfo);
		pinInfo.pFilter->Release();

		wctomb(temp, (wchar_t)pinInfo.achName);
		Log(LOG_INFO, "[out] %s", temp);
	}

//	Log(LOG_INFO, "Input pins:");
	for(int i=0; true; i++) {
		IPin* pin = EnumPin(pFilter, PINDIR_INPUT, i);
		if(!pin) { break; }

		PIN_INFO pinInfo;
		pin->QueryPinInfo(&pinInfo);
		pinInfo.pFilter->Release();

		wctomb(temp, (wchar_t)pinInfo.achName);
		Log(LOG_INFO,  "[in] %s", temp);
	}

	return;
}
*/

#endif
