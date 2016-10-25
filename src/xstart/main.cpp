#include "config.h"
#include <string>
#include <iostream>
#include <corela.h>
#include "machine.h"


#if _API_CONTAINERS
#include "Array.h"
#include "List.h"
#include "Map.h"
#endif

#if _API_STANDARD
#include "File.h"
#include "Date.h"
#include "Data.h"
#endif

#if _API_XML
#include "Xml.h"
#endif

#if _API_GRAPHICS
#include "Frame.h"
#include "Node.h"
#include "Texture.h"
#include "Canvas.h"
#include "Text.h"
#include "Model.h"
#include "Sprite.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "Recorder.h"
#include "Bitmap.h"
#include "Font.h"
#include "Color.h"
#include "Rect.h"
#endif

#if _API_OPENCV
#include "Detector.h"
#endif

#if _API_NETWORK
#include "Socket.h"
#include "Webserver.h"
#endif

#if _API_ADVANCED_AUDIO
#include "Audio.h"
#endif

#if _API_VIDEO
#include "GStreamer.h"
#include "Video.h"
#endif

#if _API_EXTRAS
#include "MidiFile.h"
#include "Sound.h"
//#include "Astro.h"
#endif

#if _API_HARDWARE
#include "Serial.h"
//#include "Logo.h"
#include "ADDevice.h"
#include "Dmx.h"
//#include "BaslerCam.h"
#include "Camera.h"
#endif


#ifdef _WIN32
#include <direct.h>
#define chdir _chdir
#endif


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


void LogVersion() {
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

		if (CheckForParam(argv[n], "silent") || CheckForParam(argv[n], "s")) {
			SetLogLevel(0);
		}

		if (CheckForParam(argv[n], "debug") || CheckForParam(argv[n], "d")) {
			SetLogLevel(2);
		}

		if(CheckForParam(argv[n], "nocolors") || CheckForParam(argv[n], "nc")) {
			SetLogColors(0);
		}

		if(CheckForParam(argv[n], "version") || CheckForParam(argv[n], "v")) {
			LogVersion();
			exit(0);
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
	// parse command line args
	argc = _argc; argv = _argv;
	int fileArg = ParseArgs(argc, argv);
	char cwd[2048];
	getcwd(cwd, 2047);

	// get xstart folder
	// TODO: Get exe-path on Mac
#if _WIN32
	char* exePath = argv[0];
#else // linux
	char exePath[2048];
	memset(exePath, 0, 2048);
	readlink("/proc/self/exe", exePath, 2047);
#endif

	// log version
	TimeSet(TimeGet());
	if (GetLogLevel() > 1) { LogVersion(); }

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

#if _API_CONTAINERS
	MachineRegisterClass<ArrayObject>("Array");
	MachineRegisterClass<List>("List");
	MachineRegisterClass<Map>("Map");
#endif

#if _API_STANDARD
	MachineRegisterClass<File>("File");
	MachineRegisterClass<Date>("Date");
	MachineRegisterClass<Data>("Data");
#endif

#if _API_XML
	MachineRegisterClass<XMLDocument>("XMLDocument");
	MachineRegisterClass<XMLNode>("XMLNode");
#endif

#if _API_GRAPHICS
	MachineRegisterClass<Frame>("Frame");
	MachineRegisterClass<Event>("Event");
	MachineRegisterClass<Handler>("Handler");
	MachineRegisterClass<Node>("Node");
	MachineRegisterClass<NodeEx>("NodeEx");
	MachineRegisterClass<Texture>("Texture");
	MachineRegisterClass<Canvas>("Canvas");
	MachineRegisterClass<Text>("Text");
	MachineRegisterClass<Mesh>("Model");
	MachineRegisterClass<Box>("Box");
	MachineRegisterClass<Shader>("Shader");
	MachineRegisterClass<Framebuffer>("Framebuffer");
	MachineRegisterClass<Recorder>("Recorder");
	MachineRegisterClass<Bitmap>("Bitmap");
	MachineRegisterClass<Font>("Font");
	MachineRegisterClass<Vector>("Vector");
	MachineRegisterClass<Color>("Color");
	MachineRegisterClass<Rect>("Rect");

#endif

#if _API_OPENCV
	MachineRegisterClass<Detector>("Detector");
#endif

#if _API_NETWORK
	MachineRegisterClass<Socket>("Socket");
	MachineRegisterClass<SocketListener>("Listener");
	MachineRegisterClass<HttpServer>("HttpServer");
#endif

#if _API_ADVANCED_AUDIO
	MachineRegisterClass<AudioDevice>("AudioDevice");
	MachineRegisterClass<AudioDeviceInfo>("AudioDeviceInfo");
	MachineRegisterClass<AudioFile>("AudioFile");
	MachineRegisterClass<AudioFilter>("AudioFilter");
	MachineRegisterClass<AudioPitch>("AudioPitch");
	MachineRegisterClass<AudioDelay>("AudioDelay");
#endif

#if _API_VIDEO
	#ifdef __linux__
	MachineRegisterClass<GStreamer>("GStreamer");
	#endif
	#ifdef _MSC_BUILD
	MachineRegisterClass<Video>("Video");
	#endif
#endif

#if _API_EXTRAS
	MachineRegisterClass<Midi>("Midi");
	#ifdef _MSC_BUILD
	MachineRegisterClass<Sound>("Sound");
	//MachineRegisterClass<Astro>("Astro");  // TODO: Port libastro to Linux.
	#endif
#endif

#if _API_HARDWARE
	MachineRegisterClass<SerialPort>("Serial");
	//MachineRegisterClass<Logo>("Logo"); // TODO: Fix Siemens Logo (interface to libnodave)
	#ifdef _MSC_BUILD
	MachineRegisterClass<ADDevice>("ADDevice"); // requires libad.dll on Windows, needs Linux porting
	MachineRegisterClass<Dmx>("Dmx"); // requires dashard.dll
	MachineRegisterClass<Camera>("Camera"); // unfinished directshow implementation
	//MachineRegisterClass<BaslerCam>("BaslerCam"); // Works, but is disabled by default because it has too many dependencies.
	#endif
#endif


	// run script or script console
	if(fileArg == -1) { MachineRun(stdin); }
	else { MachineRunFile(argv[fileArg]); }

	// exit
	MachineDestroy();
	Log(LOG_DEBUG, "Machine destroyed.");
	return 0;
}
