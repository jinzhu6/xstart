#if 0

#ifdef _WIN32
#include "ds_utils.h"


typedef struct VIDEO {
	IGraphBuilder* graphBuilder;
	IMediaControl* mediaControl;
	IMediaEvent* mediaEvent;
	IMediaPosition* mediaPosition;
	IMediaSeeking* mediaSeeking;
	IBasicAudio* audio;
	ISampleGrabber* grabber;
	IBaseFilter* grabFilter;
	IBaseFilter* sourceFilter;
	IBaseFilter* nullFilter;
} VIDEO;


void* VideoOpen(const char* _file) {
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
//	hr += video->graphBuilder->QueryInterface(IID_IBasicAudio, (void**)&video->audio);
	if(FAILED(hr)) { strcpy(failedReason, "MediaControl"); goto FAILED; }

	// create sample grabber
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&video->grabFilter);
	if (FAILED(hr)) { strcpy(failedReason, "SampleGrabber"); goto FAILED; }

	// add base filter to graph
	hr = video->graphBuilder->AddFilter(video->grabFilter, L"Sample Grabber");
	if (FAILED(hr)) { strcpy(failedReason, "Add SampleGrabber"); goto FAILED; }

	// create BaseFilter for grabber
	hr = video->grabFilter->QueryInterface(IID_ISampleGrabber, (void**)&video->grabber);
	if (FAILED(hr)) { strcpy(failedReason, "IID_ISampleGrabber"); goto FAILED; }

	// set callback
	hr = video->grabber->SetCallback(0, 0);
	if (FAILED(hr)) { strcpy(failedReason, "SetCallback"); goto FAILED; }

	// set media type for sample grabber
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24; // MEDIASUBTYPE_ARGB32
	mt.formattype = FORMAT_VideoInfo;
	hr = video->grabber->SetMediaType(&mt);
	if (FAILED(hr)) { strcpy(failedReason, "SetMediaType"); goto FAILED; }

	// convert file name to wide char string
	wchar_t file[256];
	MultiByteToWideChar(CP_ACP, 0, _file, -1, file, 256);

	// render file
	hr = video->graphBuilder->RenderFile(file, NULL);
	if (FAILED(hr)) { strcpy(failedReason, "RenderFile"); goto FAILED; }

	// set grabber to one-shot mode
	video->grabber->SetOneShot(TRUE);
	video->grabber->SetBufferSamples(TRUE);

	// create null renderer
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&video->nullFilter);
	if (FAILED(hr)) { strcpy(failedReason, "CLSID_NullRenderer"); goto FAILED; }

	// add null renderer filter to graph
	hr = video->graphBuilder->AddFilter(video->nullFilter, L"Render");
	if (FAILED(hr)) { strcpy(failedReason, "Add CLSID_NullRenderer"); goto FAILED; }

	// find video renderer
	IBaseFilter* videoRenderer;
	hr = video->graphBuilder->FindFilterByName(L"Video Renderer", &videoRenderer);
	if (FAILED(hr)) { strcpy(failedReason, "FindFilterByName Video Renderer"); goto FAILED; }

	// disconnect output pin
	IPin* pinOut = 0;
	hr = video->grabFilter->FindPin(L"Out", &pinOut);
	ListPins(video->grabFilter);
	if (FAILED(hr)) { strcpy(failedReason, "FindPin Out"); goto FAILED; }
	hr = pinOut->Disconnect();
	if (FAILED(hr)) { strcpy(failedReason, "Disconnect Out"); goto FAILED; }

	// remove and release video renderer
	hr = video->graphBuilder->RemoveFilter(videoRenderer);
	if (FAILED(hr)) { strcpy(failedReason, "RemoveFilter Video Renderer"); goto FAILED; }
	hr = videoRenderer->Release();
	if (FAILED(hr)) { strcpy(failedReason, "Release Video Renderer"); goto FAILED; }

	// connect output to input pin
	IPin* pinIn = 0;
	hr = video->nullFilter->FindPin(L"In", &pinIn);	pinIn->Disconnect();
	if (FAILED(hr)) { strcpy(failedReason, "FindPin In"); goto FAILED; }
	ListPins(video->nullFilter);
	hr = pinOut->Connect(pinIn, NULL);
	if (FAILED(hr)) { strcpy(failedReason, "Connect Out In"); goto FAILED; }

	// 
	hr += video->mediaControl->Run();
	hr += video->mediaControl->Stop();

	return video;

FAILED:
	Log(LOG_ERROR, "Error while creating filter graph for video '%s', reason is: '%s' (%x)", _file, failedReason, hr);
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

coBool VideoSetSpeed(void* _video, double speed) {
	VIDEO* video = (VIDEO*)_video;
	HRESULT hr = video->mediaSeeking->SetRate(speed);
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


#endif

