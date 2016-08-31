#include <string>
#include <iostream>
#include <corela.h>
#include "_machine.h"

#include "Xml.h"
#include "Array.h"
#include "List.h"
#include "Map.h"
#include "File.h"
#include "Frame.h"
#include "Socket.h"
#include "Node.h"
#include "Bitmap.h"
#include "Font.h"
#include "Shader.h"
#include "Text.h"
#include "Camera.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "Recorder.h"
#include "Color.h"
#include "ADDevice.h"
#include "Rect.h"
#include "Canvas.h"
#include "Detector.h"
#include "Video.h"
//#include "Sound.h"
//#include "Model.h"
#include "Astro.h"
#include "Date.h"
#include "Serial.h"
#include "MidiFile.h"
#include "Data.h"
#include "Audio.h"
#include "Logo.h"
#include "GStreamer.h"
#include "Dmx.h"
#include "Webserver.h"
//#include "BaslerCam.h"

#ifdef _WIN32
#include <direct.h>
#define chdir _chdir
#endif


FRAME* g_FRAME = 0;


/******************************************************************************
* IsParam
*******************************************************************************/
int IsParam(const char* _arg) {
	if(_arg[0] != '-'/* && _arg[0] != '/'*/) {
		return 0;
	}
	return 1;
}


/******************************************************************************
* CheckForParam
*******************************************************************************/
int CheckForParam(const char* _arg, const char* param) {
	const char* arg = _arg;
	if(arg[0] == '-' /* || arg[0] == '/'*/) {
		arg++;
	}
	if(arg[0] == '-') {
		arg++;
	}

#ifdef _WIN32
	if(stricmp(arg, param) == 0) {
		return 1;
	}
#else
	if(strcasecmp(arg, param) == 0) {
		return 1;
	}
#endif

	return 0;
}


/******************************************************************************
* ParseArgs
*******************************************************************************/
int ParseArgs(int argc, char* argv[]) {
	int fileArg = -1;
	for(int n=1; n<argc; n++) {
		if(!IsParam(argv[n]) && fileArg == -1) {
			fileArg = n;
			continue;
		}

		if (CheckForParam(argv[n], "silent")) {
			SetLogLevel(0);
		}

		if (CheckForParam(argv[n], "debug")) {
			SetLogLevel(2);
		}

		if(CheckForParam(argv[n], "version") || CheckForParam(argv[n], "v")) {
			Log(LOG_INFO, "%s %s  -  Compiled at %s %s. Debug: %s. OpenMP: %s.", NAME, VERSION, __DATE__, __TIME__,
#ifdef _DEBUG
			    "yes",
#else
			    "no",
#endif
#ifdef _OPENMP
			    "yes");
#else
			    "no");
#endif
			exit(0);
		}
		if(CheckForParam(argv[n], "r") || CheckForParam(argv[n], "realtime")) {
			Log(LOG_INFO, "Realtime.");
		}
		if(CheckForParam(argv[n], "msaa")) {
			if(argc-1 > n) {
				Log(LOG_INFO, "MSAA set to %s.", argv[++n]);
			} else {
				Log(LOG_ERROR, "MSAA NOT SET!");
			}
		}
		if(CheckForParam(argv[n], "log") || CheckForParam(argv[n], "l")) {
			if(argc-1 > n) {
				Log(LOG_INFO, "Log set to '%s'.", argv[++n]);
			} else {
				Log(LOG_ERROR, "LOG TYPE NOT SET!");
			}
		}
		if(CheckForParam(argv[n], "o") || CheckForParam(argv[n], "output")) {
			if(argc-1 > n) {
				Log(LOG_INFO, "Output set to '%s'.", argv[++n]);
			} else {
				Log(LOG_ERROR, "OUT NOT SET!");
			}
		}
		if(CheckForParam(argv[n], "nocolors")) {
			SetLogColors(0);
		}
	}

	return fileArg;
}


/******************************************************************************
* GetFileDirectory - gets the directory part of a file name
*******************************************************************************/
std::string GetFileDirectory(const char* file) {
	char* delimA = strrchr((char*)file, '\\');   // MSVC has no const for first param!?!
	char* delimB = strrchr((char*)file, '/');
	if(!delimA && !delimB) {
		return std::string();
	}
	char* delim = (delimA > delimB) ? delimA : delimB;
	char tmp = *delim;
	*delim = '\0';
	std::string result = file;
	*delim = tmp;
	return result;
}


/******************************************************************************
* main - main entry point
*******************************************************************************/
int main(int _argc, char* _argv[]) {
	argc = _argc;
	argv = _argv;

	int fileArg = ParseArgs(argc, argv);
	char cwd[2048];
	getcwd(cwd, 2047);

	// get xstart folder
#if _WIN32
	char* exePath = argv[0];
#else
	char exePath[2048];
	memset(exePath, 0, 2048);
	readlink("/proc/self/exe", exePath, 2047);
#endif

	// get library directory
	Log(LOG_DEBUG, "xstart is here '%s'.", exePath);
	std::string _libPath = GetFileDirectory(exePath);
	chdir(_libPath.c_str());
	chdir("..");
	if(chdir("lib") != 1) { chdir(".."); chdir("lib"); }
	chdir("..");
	char libPath[1024];
	getcwd(libPath, 1023);
	SetLibraryPath(libPath);
	Log(LOG_DEBUG, "Library path set to '%s'.", libPath);

	// change current path to given script file
	chdir(cwd);
	if(fileArg != -1) {
		Log(LOG_DEBUG, "Current directory set to '%s'.", GetFileDirectory(argv[fileArg]).c_str());
		chdir(GetFileDirectory(argv[fileArg]).c_str());
	}

	// create script engine and register global functions
	MachineCreate(true);
	RegisterCommonAPI();

	// register script classes
	Log(LOG_DEBUG, "Registering classes: ");
	MachineRegisterClass<ArrayObject>("Array");
	MachineRegisterClass<List>("List");
	MachineRegisterClass<Map>("Map");
	MachineRegisterClass<Data>("Data");
	MachineRegisterClass<Vector>("Vector");
	MachineRegisterClass<Color>("Color");
	MachineRegisterClass<Rect>("Rect");
	MachineRegisterClass<Date>("Date");
	MachineRegisterClass<File>("File");
	MachineRegisterClass<Midi>("Midi");
	MachineRegisterClass<XMLDocument>("XMLDocument");
	MachineRegisterClass<XMLNode>("XMLNode");
	MachineRegisterClass<Bitmap>("Bitmap");
	MachineRegisterClass<Font>("Font");
	MachineRegisterClass<Frame>("Frame");
	MachineRegisterClass<Event>("Event");
	MachineRegisterClass<Handler>("Handler");
	MachineRegisterClass<Node>("Node");
	MachineRegisterClass<NodeEx>("NodeEx");
	MachineRegisterClass<Texture>("Texture");
	MachineRegisterClass<Canvas>("Canvas");
	MachineRegisterClass<Text>("Text");
//	MachineRegisterClass<Model>("Model");
	MachineRegisterClass<Shader>("Shader");
	MachineRegisterClass<Framebuffer>("Framebuffer");
	MachineRegisterClass<Recorder>("Recorder");
	MachineRegisterClass<Detector>("Detector");
	MachineRegisterClass<Socket>("Socket");
	MachineRegisterClass<SocketListener>("Listener");
	MachineRegisterClass<HttpServer>("HttpServer");
	MachineRegisterClass<SerialPort>("Serial");
	MachineRegisterClass<Logo>("Logo");
	MachineRegisterClass<AudioDevice>("AudioDevice");
	MachineRegisterClass<AudioDeviceInfo>("AudioDeviceInfo");
	MachineRegisterClass<AudioFile>("AudioFile");
	MachineRegisterClass<AudioFilter>("AudioFilter");
	MachineRegisterClass<AudioPitch>("AudioPitch");
	MachineRegisterClass<AudioDelay>("AudioDelay");

#ifdef _MSC_BUILD
	MachineRegisterClass<Astro>("Astro");
	MachineRegisterClass<ADDevice>("ADDevice");
	MachineRegisterClass<Dmx>("Dmx");
//	MachineRegisterClass<Sound>("Sound");
	MachineRegisterClass<Video>("Video");
	MachineRegisterClass<Camera>("Camera");
//	MachineRegisterClass<BaslerCam>("BaslerCam");
#endif

#ifdef __linux__
	MachineRegisterClass<GStreamer>("GStreamer");
#endif

	// run script or script terminal
	if(fileArg == -1) {
		MachineRun(stdin);
	} else {
		MachineRunFile(argv[fileArg]);
	}

	// exit
	MachineDestroy();
	//Log(LOG_INFO, "Machine exited.");

	printf("\n");
	return 0;
}
