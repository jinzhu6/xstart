#ifndef _OPTRIS_H_
#define _OPTRIS_H_

// ONLY FOR REFERENCE, NOT WORKING, NOR SUPPORTED YET!


#include "ScriptObject.h"
#include "../hw_optris/ImagerIPC2.h"
//#pragma comment(lib, "../../../src/hw_optris/ImagerIPC2.lib")


class Optris : public ScriptObject {
public:

	Optris() : ScriptObject() {
		id = "Optris";
		help = "Optris IR-Camera interface";

		HRESULT hr = 0;

		hr = SetLogFile(L"optris.log", 10000, false);
		if(hr) { Log(LOG_ERROR, "Error while initializing Optris-Device with function '%s'. Error code is: 0x%x.", "SetLogFile", hr); }

		hr = SetImagerIPCCount(1);
		if(hr) { Log(LOG_ERROR, "Error while initializing Optris-Device with function '%s'. Error code is: 0x%x.", "SetImageIPCCount", hr); }

		hr = InitImagerIPC(0);
		if(hr) { Log(LOG_ERROR, "Error while initializing Optris-Device with function '%s'. Error code is: 0x%x.", "InitImagerIPC", hr); }

		SetCallback_OnServerStopped(0, Optris::OnServerStopped);
		SetCallback_OnFrameInit(0, Optris::OnFrameInit);
		SetCallback_OnNewFrame(0, Optris::OnNewFrame);
		SetCallback_OnNewFrameEx(0, Optris::OnNewFrameEx);
		SetCallback_OnInitCompleted(0, Optris::OnInitCompleted);
		SetCallback_OnConfigChanged(0, Optris::OnConfigChanged);
		SetCallback_OnFileCommandReady(0, Optris::OnFileCommandReady);

		hr = RunImagerIPC(0);
		if(hr) { Log(LOG_ERROR, "Error while initializing Optris-Device with function '%s'. Error code is: 0x%x.", "RunImagerIPC", hr); }
	}

	static HRESULT WINAPI OnServerStopped(int reason) {
		return 0;
	}

	static HRESULT WINAPI OnFrameInit(int a, int b, int c) {
		return 0;
	}

	static HRESULT WINAPI OnNewFrame(char* a, int b) {
		return 0;
	}

	static HRESULT WINAPI OnNewFrameEx(void* a, FrameMetadata* b) {
		return 0;
	}

	static HRESULT WINAPI OnInitCompleted(void) {
		return 0;
	}

	static HRESULT WINAPI OnConfigChanged(long reserved) {
		return 0;
	}

	static HRESULT WINAPI OnFileCommandReady(wchar_t* path) {
		return 0;
	}



public:

};


#endif
