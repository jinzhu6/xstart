#ifdef _WIN32


#include "ds_utils.h"

typedef struct VIDEO {
	IGraphBuilder* graphBuilder;

	IMediaControl* mediaControl;
	IMediaEvent* mediaEvent;
	IMediaPosition* mediaPosition;
	IMediaSeeking* mediaSeeking;

	ISampleGrabber* grabber;
	IBaseFilter* grabFilter;

	IBaseFilter* sourceFilter;
	IBaseFilter* nullFilter;
} VIDEO;


coBool _VideoCreateSampleGrabber(VIDEO* video) {
	HRESULT hr;

	// create sample grabber
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_ISampleGrabber, (void**)&video->grabber);
	if (FAILED(hr)) {
		return coFalse;
	}

	// create BaseFilter for grabber
	IBaseFilter* pGrab = NULL;
	hr = video->grabber->QueryInterface(IID_IBaseFilter, (void**)&video->grabFilter);
	if (FAILED(hr)) {
		return coFalse;
	}

	// set media type for sample grabber
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_ARGB32; // MEDIASUBTYPE_RGB24
	hr = video->grabber->SetMediaType(&mt);
	if (FAILED(hr)) {
		return coFalse;
	}

	// set grabber to one-shot mode
	video->grabber->SetBufferSamples(TRUE);
	video->grabber->SetOneShot(TRUE);

	// add base filter to graph
	hr = video->graphBuilder->AddFilter(video->grabFilter, L"Sample Grabber");
	if (FAILED(hr)) {
		return coFalse;
	}

	return coTrue;
}

coBool _VideoCreateSourceFilter(VIDEO* video, const char* _file) {
	// convert file name to wide char string
	wchar_t file[256];
	MultiByteToWideChar(CP_ACP, 0, _file, -1, file, 256);

	// add source filter to graph
	HRESULT hr;
	hr = video->graphBuilder->AddSourceFilter(file, L"Source", &video->sourceFilter);
	if(FAILED(hr)) {
		return coFalse;
	}

//	video->sourceFilter->Run(NULL);

	// connect filters
	IEnumPins* pEnum = NULL;
	IPin* pPin = NULL;
	hr = video->sourceFilter->EnumPins(&pEnum);
	if (FAILED(hr)) {
		return coFalse;
	}

	while (S_OK == pEnum->Next(1, &pPin, NULL)) {
		hr = ConnectFilters(video->graphBuilder, pPin, video->grabFilter);
		if(pPin != NULL) { pPin->Release(); }
		if (SUCCEEDED(hr)) {
			goto ok;
		}
	}

	Log(LOG_ERROR, "No compatible output pin found for '%s'.", _file);

	/*IPin* pPin = NULL;
	GUID guid = MEDIASUBTYPE_ARGB32;
	if(filterHasCompatibleOutputPin(video->sourceFilter, &guid, &pPin)) {
		ConnectFilters(video->graphBuilder, pPin, video->grabFilter);
	} else {
		Log(LOG_ERROR, "No compatible output pin found in video!");
	}*/


ok:
//	video->sourceFilter->Run(1000.0);
	return coTrue;
}

coBool _VideoCreateNullRenderer(VIDEO* video) {
	HRESULT hr;
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&video->nullFilter);
	if (FAILED(hr)) {
		return coFalse;
	}

	hr = video->graphBuilder->AddFilter(video->nullFilter, L"Null Filter");
	if (FAILED(hr)) {
		return coFalse;
	}

	hr = ConnectFilters(video->graphBuilder, video->grabFilter, video->nullFilter);
	if (FAILED(hr)) {
		return coFalse;
	}

	return coTrue;
}

void* VideoOpen(const char* file) {
	char failedReason[256];
	CoInitialize(NULL);

	// create video object
	VIDEO* video = (VIDEO*)malloc(sizeof(VIDEO));
	memset(video, 0, sizeof(VIDEO));

	// create graph builder
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&video->graphBuilder);
	if(FAILED(hr)) { strcpy(failedReason, "GraphBuilder"); goto FAILED; }

	// obtain MediaControl, MediaEvent and MediaPosition interfaces
	hr  = video->graphBuilder->QueryInterface(IID_IMediaControl, (void**)&video->mediaControl);
	hr += video->graphBuilder->QueryInterface(IID_IMediaEvent, (void**)&video->mediaEvent);
	hr += video->graphBuilder->QueryInterface(IID_IMediaPosition, (void**)&video->mediaPosition);
	hr += video->graphBuilder->QueryInterface(IID_IMediaSeeking, (void**)&video->mediaSeeking);
	if(FAILED(hr)) { strcpy(failedReason, "MediaControl"); goto FAILED; }

	// create sample grabber
	_VideoCreateSampleGrabber(video);

	// create source filter
	_VideoCreateSourceFilter(video, file);

	// create null renderer filter
	_VideoCreateNullRenderer(video);

	// set null clock
	/*IMediaFilter* pMediaFilter = 0;
	hr = video->graphBuilder->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
	if(FAILED(hr)) { strcpy(failedReason, "MediaFilter"); goto FAILED; }
	pMediaFilter->SetSyncSource(NULL);
	pMediaFilter->Release();*/

	// create reference clock
	IReferenceClock* pClock;
	hr = CoCreateInstance (CLSID_SystemClock, NULL, CLSCTX_INPROC_SERVER, IID_IReferenceClock, (void**)&pClock);
	if(FAILED(hr)) { strcpy(failedReason, "ReferenceClock"); goto FAILED; }

	// set the graph clock
	IMediaFilter* pMediaFilter = 0;
	video->graphBuilder->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
	pMediaFilter->SetSyncSource(pClock);
	pClock->Release();
	pMediaFilter->Release();

	video->mediaSeeking->SetRate(00.1);

	return video;


#if 0
	// create graph for video file (render file)
	wchar_t newWideString[256];
	MultiByteToWideChar(CP_ACP, 0, file, -1, newWideString, 256);
	HRESULT hres = video->graphBuilder->RenderFile(newWideString, NULL);
	if(FAILED(hr)) { strcpy(failedReason, "RenderFile"); goto FAILED; }

	// find renderer pin
	IBaseFilter* filter;
	IEnumFilters* filterList;
	ULONG tmp;
	hr = video->graphBuilder->EnumFilters(&filterList);
	if(FAILED(hr)) {strcpy(failedReason, "EnumFilters"); goto FAILED; }
	filterList->Reset();
	while(filterList->Next(1, &filter, &tmp) == S_OK) {
		Log(LOG_ERROR, "FILTER");
//		PrintFilter(filter);
		/*if (isRenderer(filt)) {
			renderers.add(filt);
		}
		else filt->Release();*/
		filter->Release();
	}
	filterList->Release();

	// create sample grabber and base-filter
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&video->grabFilter);
	if(FAILED(hr)) { strcpy(failedReason, "SampleGrabber"); goto FAILED; }
	hr = video->grabFilter->QueryInterface(IID_ISampleGrabber, (void**)&video->grabber);
	if(FAILED(hr)) { strcpy(failedReason, "Grabber"); goto FAILED; }

	// set media type for sample grabber
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_ARGB32; //MEDIASUBTYPE_ARGB32; //MEDIASUBTYPE_ARGB32; // MEDIASUBTYPE_RGB24
	mt.formattype = FORMAT_VideoInfo;
	hr = video->grabber->SetMediaType(&mt);
	if(FAILED(hr)) { strcpy(failedReason, "SetMediaType"); goto FAILED; }

	// add grabber filter to graph
	hr = video->graphBuilder->AddFilter(video->grabFilter, L"Sample Grabber");
	if (FAILED(hr)) { strcpy(failedReason, "AddFilter"); goto FAILED; }

	// obtain MediaControl, MediaEvent and MediaPosition interfaces
	hr  = video->graphBuilder->QueryInterface(IID_IMediaControl, (void**)&video->mediaControl);
	hr += video->graphBuilder->QueryInterface(IID_IMediaEvent, (void**)&video->mediaEvent);
	hr += video->graphBuilder->QueryInterface(IID_IMediaPosition, (void**)&video->mediaPosition);
	if(FAILED(hr)) { strcpy(failedReason, "MediaControl"); goto FAILED; }

	// set grabber to one-shot mode
	video->grabber->SetBufferSamples(TRUE);
	video->grabber->SetOneShot(TRUE);

	// start stream
	hr = video->mediaControl->Run();
	if(FAILED(hr)) { strcpy(failedReason, "Run"); goto FAILED; }

	// wait for completion
	/*long evCode;
	hr = video->mediaEvent->WaitForCompletion(INFINITE, &evCode);
	if (FAILED(hr)) { strcpy(failedReason, "WaitForCompletion"); goto FAILED; }*/

	// get video information
	hr = video->grabber->GetConnectedMediaType(&mt);
	if(FAILED(hr)) { strcpy(failedReason, "GetConnectedMediaType"); goto FAILED; }
	VIDEOINFOHEADER* videoInfo = (VIDEOINFOHEADER*) mt.pbFormat;
#endif

FAILED:
	Log(LOG_ERROR, "Error while creating filter graph for video '%s', reason is: '%s' (%x)", file, failedReason, hr);
	VideoClose(video);
	return 0;
}

coBool VideoStart(void* _video) {
	VIDEO* video = (VIDEO*)_video;
	HRESULT hr = video->mediaControl->Run();
//	video->mediaControl->Pause();
	if (FAILED(hr)) { return coFalse; }
	return coTrue;
}

coBool VideoPause(void* _video) {
	VIDEO* video = (VIDEO*)_video;
	HRESULT hr = video->mediaControl->Pause();
	if (FAILED(hr)) { return coFalse; }
	return coTrue;
}

coBool VideoStop(void* _video) {
	VIDEO* video = (VIDEO*)_video;
	HRESULT hr = video->mediaControl->Stop();
	video->mediaPosition->put_CurrentPosition(0.0);
	if (FAILED(hr)) { return coFalse; }
	return coTrue;
}

coBool VideoSetPosition(void* _video, double position) {
	VIDEO* video = (VIDEO*)_video;
	HRESULT hr = video->mediaPosition->put_CurrentPosition(position);
	if (FAILED(hr)) { return coFalse; }
	return coTrue;
}

double VideoGetPosition(void* _video) {
	VIDEO* video = (VIDEO*)_video;
	double position;
	HRESULT hr = video->mediaPosition->get_CurrentPosition(&position);
	if (FAILED(hr)) { return 0.0; }
	return position;
}

double VideoGetDuration(void* _video) {
	VIDEO* video = (VIDEO*)_video;
	double duration;
	HRESULT hr = video->mediaPosition->get_Duration(&duration);
	if (FAILED(hr)) { return 0.0; }
	return duration;
}

coBool VideoGetImage(void* _video, IMAGE* image, coDword* _width, coDword* _height) {
	VIDEO* video = (VIDEO*)_video;
	return DSGrabberGetImage(video->grabber, image, _width, _height);
}

void VideoClose(void* _video) {
	VIDEO* video = (VIDEO*)_video;
	if(video->mediaControl) { video->mediaControl->Stop(); }
	SAFE_RELEASE(video->nullFilter);
	SAFE_RELEASE(video->sourceFilter);
	SAFE_RELEASE(video->grabFilter);
	SAFE_RELEASE(video->grabber);
	SAFE_RELEASE(video->mediaControl);
	SAFE_RELEASE(video->mediaEvent);
	SAFE_RELEASE(video->mediaPosition);
	SAFE_RELEASE(video->graphBuilder);
	free(video);
}



#if 0

VideoPlay::VideoPlay() {
	CoInitialize(NULL);
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&_graphBuilder);
	hr = _graphBuilder->QueryInterface(IID_IMediaControl, (void**)&_mediaControl);
	hr = _graphBuilder->QueryInterface(IID_IMediaEvent, (void**)&_mediaEvent);
	hr = _graphBuilder->QueryInterface(IID_IMediaPosition, (void**)&_mediaPosition);
//	hr = _graphBuilder->QueryInterface(IID_IVideoWindow, (void**)&_videoWindow);
}

VideoPlay::~VideoPlay() {
	while(_graphBuilder->Release()) {  };
}

bool VideoPlay::OpenVideo(std::string vidFile) {
	if(vidFile.length() > 255) {
		return false;
	}
	wchar_t newWideString[256];
	MultiByteToWideChar(CP_ACP, 0, vidFile.c_str(), -1, newWideString, 256);
	HRESULT hr = _graphBuilder->RenderFile(newWideString, NULL);
	hr = _graphBuilder->QueryInterface(IID_IVideoWindow, (void**)&_videoWindow);
	return true;
}

bool VideoPlay::SetWindow(HWND hwnd, RECT* _rc) {
	_videoWindow->put_Owner((OAHWND)hwnd);
	_videoWindow->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	//_videoWindow->put_Visible(OAFALSE);

	RECT rc;
	if(_rc) {
		rc.top = _rc->top;
		rc.left = _rc->left;
		rc.right = _rc->right;
		rc.bottom = _rc->bottom;
	} else {
		GetClientRect(hwnd, &rc);
	}
	HRESULT hr = _videoWindow->SetWindowPosition(rc.left, rc.top, rc.right, rc.bottom);
	return true;
}

bool VideoPlay::PlayVideo() {
	HRESULT hr = _mediaControl->Run();
	return true;
}

bool VideoPlay::StopVideo() {
	HRESULT hr = _mediaControl->Stop();
	_videoWindow->put_Owner(NULL);
	_videoWindow->Release();
	return true;
}

bool VideoPlay::PauseVideo() {
	HRESULT hr = _mediaControl->Pause();
	return true;
}

double VideoPlay::GetPosition() {
	double c;
	_mediaPosition->get_CurrentPosition(&c);
	return c;
}

bool VideoPlay::SetPosition(double p) {
	_mediaPosition->put_CurrentPosition(p);
	return true;
}

double VideoPlay::GetDuration() {
	double d;
	_mediaPosition->get_Duration(&d);
	return d;
}

bool VideoPlay::IsAtEnd() {
	double c,d;
	d = GetDuration();
	c = GetPosition();
	if(c >= d) {
		return true;
	}
	return false;
}

#endif


#endif
