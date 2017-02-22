#include "config.h"
#include <corela.h>
#include <strtrim.h>
#include <time.h>
#include <math.h>
#include "machine.h"
#include "Rect.h"

int argc;
char **argv;

#ifndef _WIN32
#define _popen popen
#define _pclose pclose
#endif

int GM_CDECL Script_GetVersion(gmThread* a_thread) {
	a_thread->PushNewString(VERSION);
	return GM_OK;
}

int GM_CDECL Script_LoadConfig(gmThread* a_thread) {
	GM_CHECK_STRING_PARAM(key, 0);

	// always use config.ini
	INI* ini = INIOpen("config.ini", true);
	if(!ini) {
		Log(LOG_ERROR, "No config.ini found!");
		a_thread->PushNull();
		return GM_OK;
	}

	// always use first section
	INISECTION* sec = INIEnumSections(ini, 0);
	if(!sec) {
		sec = INIGetSection(ini, "config", true);

		if(!sec) {
			Log(LOG_ERROR, "No [config] section found in config.ini!");
			INIClose(ini, true);
			a_thread->PushNull();
			return GM_OK;
		}
	}

	// get entry
	INIENTRY* ent = INIGetEntry(sec, key, true, "");
	if(!ent) {
		Log(LOG_ERROR, "Entry '%s' not found in config.ini!");
		INIClose(ini, true);
		a_thread->PushNull();
		return GM_OK;
	}
	char entStr[4097];
	INIGetEntryStr(ent, entStr, 4096);

	Log(LOG_INFO, "Loaded config '%s' is '%s'.", key, entStr);

	// return result
	INIClose(ini, false);
	a_thread->PushNewString(entStr);
	return GM_OK;
}

int GM_CDECL Script_SaveConfig(gmThread* a_thread) {
	GM_CHECK_STRING_PARAM(key, 0);
	GM_CHECK_STRING_PARAM(value, 1);

	Log(LOG_INFO, "Saving config '%s' as '%s'.", key, value);

	// always use config.ini
	INI* ini = INIOpen("config.ini", true);
	if(!ini) {
		Log(LOG_ERROR, "Creating config.ini failed!");
		return GM_OK;
	}

	// always use first section
	INISECTION* sec = INIEnumSections(ini, 0);
	if(!sec) {
		sec = INIGetSection(ini, "config", true);

		if(!sec) {
			Log(LOG_ERROR, "Creating section in config.ini failed!");
			INIClose(ini, true);
			a_thread->PushNull();
			return GM_OK;
		}
	}

	// set entry
	INIENTRY* ent = INIGetEntry(sec, key, true, value);
	if(!ent) {
		Log(LOG_ERROR, "Creating entry in config.ini failed!");
		INIClose(ini, true);
		a_thread->PushNull();
		return GM_OK;
	}
	INISetEntryStr(ent, value);

	// save config.ini
	INIClose(ini, true);

	return GM_OK;
}

int GM_CDECL Script_Ascii(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	gmType t = a_thread->ParamType(0);
	if(t == GM_STRING) {
		GM_CHECK_STRING_PARAM(asc, 0);
		a_thread->PushInt(asc[0]);
	} else {
		GM_CHECK_INT_PARAM(c, 0);
		char ascii[4];
		//sprintf(ascii, "%c", c);
		ascii[0] = c;
		ascii[1] = 0;
		a_thread->PushNewString(ascii);
	}
	return GM_OK;
}

int GM_CDECL Script_System(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(sys, 0);
	Log(LOG_DEBUG, "Running system(%s) ...", sys);
	FILE* pipe = _popen(sys, "r");
	if(!pipe) {
		Log(LOG_ERROR, "Error while executing system command '%s'!", sys);
		return GM_OK;
	}

	char buffer[256];
	std::string result;
	/*while(!feof(pipe)) {
		if(fgets(buffer, 255, pipe) != NULL) {
			result += buffer;
		}
	}*/
	int n;
	while(n = fread(buffer, 1, 255, pipe)) {
		buffer[n] = '\0';
		result += buffer;
	}
	_pclose(pipe);
	a_thread->PushNewString(result.c_str());
	return GM_OK;
}

//#include <Windows.h>
int GM_CDECL Script_System_Async(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(sys, 0);
	Log(LOG_DEBUG, "Running start(%s) ...", sys);
	//	CreateProcessA(NULL, sys, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, NULL, NULL);
#ifdef _WIN32
//	std::string cmd = std::string("start ") + sys;
	PROCESS_INFORMATION procInfo;

	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwX = CW_USEDEFAULT;
	startupInfo.dwY = CW_USEDEFAULT;
	startupInfo.dwXSize = CW_USEDEFAULT;
	startupInfo.dwYSize = CW_USEDEFAULT;
	startupInfo.dwXCountChars = STARTF_USESIZE;
	startupInfo.dwYCountChars = STARTF_USESIZE;
	startupInfo.dwFillAttribute = FOREGROUND_BLUE;

	BOOL success = CreateProcessA(NULL, (LPSTR)sys, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &procInfo);
	a_thread->PushInt(success ? 1 : 0);
#else
	std::string cmd = sys + std::string(" &");
	system(cmd.c_str());
#endif
	return GM_OK;
}

int GM_CDECL Script_Exit(gmThread* th) {
	MachineDestroy();
	exit(0);
	return GM_OK;
}

int GM_CDECL Script_Print(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(out, 0);
	printf("%s",out);
	fflush(stdout);
	return GM_OK;
}

int GM_CDECL Script_PrintLine(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(out, 0);
	printf("%s\n", out);
	fflush(stdout);
	return GM_OK;
}

int GM_CDECL Script_Log(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(out, 0);
	Log(LOG_SCRIPT, out);
	return GM_OK;
}

int GM_CDECL Script_Warning(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(out, 0);
	Log(LOG_WARNING, out);
	return GM_OK;
}

int GM_CDECL Script_Error(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(out, 0);
	Log(LOG_ERROR, out);
	return GM_OK;
}

int GM_CDECL Script_Fatal(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(out, 0);
	Log(LOG_FATAL, out);
	exit(-1);
	return GM_OK;
}

int GM_CDECL Script_Popup(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(info, 0);
	Log(LOG_POPUP, info);
	return GM_OK;
}

int GM_CDECL Script_Ask(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(ask, 0);
	printf("%s", ask);
	fflush(stdout);

	char answer[1024*10];
	memset((void*)answer, 0, 1024*10);

	fgets(answer, 1024*10-1, stdin);
	trim(answer);

	a_thread->PushNewString(answer);

	return GM_OK;
}

static float g_TimeOffset = 0.0;
int GM_CDECL Script_GetTime(gmThread* a_thread) {
	float t = (float)TimeGet();
	a_thread->PushFloat(t - g_TimeOffset);
	return GM_OK;
}

int GM_CDECL Script_SetTime(gmThread* a_thread) {
	GM_CHECK_FLOAT_OR_INT_PARAM(nt, 0);
	float t = (float)TimeGet();
	g_TimeOffset = t - nt;
	a_thread->PushFloat(t - g_TimeOffset);
	return GM_OK;
}

int GM_CDECL Script_Time(gmThread* a_thread) {
	if(a_thread->GetNumParams()) {
		return Script_SetTime(a_thread);
	} else {
		return Script_GetTime(a_thread);
	}
}

int GM_CDECL Script_GetMemUsage(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(0);
	a_thread->PushInt(machine->GetCurrentMemoryUsage());
	return GM_OK;
}

int GM_CDECL Script_Collect(gmThread* a_thread) {
	if(a_thread->GetNumParams() == 0) {
		machine->CollectGarbage(true);
	} else {
		GM_CHECK_INT_PARAM(full,0);
		machine->CollectGarbage(full==1);
	}
	return GM_OK;
}

int GM_CDECL Script_Redirect(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(redTo, 0);
	freopen(redTo, "wb", stdout);
	return GM_OK;
}

int GM_CDECL Script_GlobalHelp(gmThread* a_thread) {
	std::string help = MachineGlobalHelp();
	a_thread->PushNewString(help.c_str());
	return GM_OK;
}

int GM_CDECL Script_ClassList(gmThread* a_thread) {
	std::string list = MachineGetClassList();
	a_thread->PushNewString(list.c_str());
	return GM_OK;
}

int GM_CDECL Script_Random(gmThread* a_thread) {
	if(a_thread->ParamType(0) == GM_INT) {
		int scalei;
		a_thread->ParamInt(0, scalei, 1);
		//a_thread->PushInt( ((gmfloat)rand() / (gmfloat)RAND_MAX) * scalei);
		a_thread->PushInt((float)rand() / (float)RAND_MAX * scalei + 0.5f);
		return GM_OK;
	}

	gmfloat scale = 1.0;
	if(a_thread->GetNumParams() >= 1) {
		a_thread->ParamFloatOrInt(0, scale, 1.0);
	}
	a_thread->PushFloat( (gmfloat)rand() / (gmfloat)RAND_MAX * scale);
	return GM_OK;
}

#ifdef _WIN32
int GM_CDECL Script_PlaySound(gmThread* a_thread) {
	GM_CHECK_STRING_PARAM(file, 0);
	SoundSimplePlay(file);
	return GM_OK;
}
#endif

int GM_CDECL Script_CreateInstance(gmThread* a_thread) {
	GM_CHECK_STRING_PARAM(className, 0);
	return MachineCreateClassInstance(a_thread, className);
}

int GM_CDECL Script_Pause(gmThread* a_thread) {
	GM_CHECK_FLOAT_OR_INT_PARAM(s, 0);
	TimeSleep(s);
	return GM_OK;
}

int GM_CDECL Script_Throw(gmThread* a_thread) {
	if(a_thread->GetNumParams() > 0) {
		if(a_thread->ParamType(0) == GM_STRING) {
			const char* error;
			a_thread->ParamString(0, error, "");
			GM_EXCEPTION_MSG(error);
		}
	}
	return GM_EXCEPTION;
}

int GM_CDECL Script_Callstack(gmThread* a_thread) {
	std::string callstack;

	//machine->GetLog().LogEntry(GM_NL"CALLSTACK:");
	gmStackFrame * frame = a_thread->m_frame;
	int base = a_thread->m_base;
	const gmuint8 * ip = a_thread->m_instruction;

	while(frame) {
		// get the function object
		gmVariable * fnVar = &a_thread->m_stack[base - 1];
		if(fnVar->m_type == GM_FUNCTION) {
			gmFunctionObject * fn = (gmFunctionObject *) GM_MOBJECT(a_thread->m_machine, fnVar->m_value.m_ref);
			//m_machine->GetLog().LogEntry("%3d: %s", fn->GetLine(ip), fn->GetDebugName());
			//callstack += "" + fn->GetLine(ip);
			callstack += ":";
			callstack += fn->GetDebugName();
			callstack += "\n";
		}
		base = frame->m_returnBase;
		ip = frame->m_returnAddress;
		frame = frame->m_prev;
	}
	a_thread->PushNewString(callstack.c_str());
	//m_machine->GetLog().LogEntry("");
	return GM_OK;
}

int GM_CDECL Script_Eval(gmThread* a_thread) {
	GM_CHECK_NUM_PARAMS(1);
	GM_CHECK_STRING_PARAM(script, 0);
	GM_INT_PARAM(now, 1, 1); // 2nd param is execute now flag
	gmVariable paramThis = a_thread->Param(2, gmVariable::s_null); // 3rd param is 'this'

	int id = GM_INVALID_THREAD;
	if( script ) {
		int errors = a_thread->GetMachine()->ExecuteString(script, &id, (now) ? true : false, NULL, &paramThis);
		if( errors ) { return GM_EXCEPTION; }
		a_thread->PushInt(id);
	}
	return GM_OK;
}

int GM_CDECL Script_InputFile(gmThread* a_thread) {
	std::string file = RequestFileName();
	if(file.length()) { a_thread->PushNewString(file.c_str()); }
	else { a_thread->PushNull(); }
	return GM_OK;
}

#ifdef _WIN32
int GM_CDECL Script_GetMonitorRect(gmThread* a_thread) {
	GM_CHECK_INT_PARAM(index, 0);
	CORECT _rc;
	GetMonitorRect(index, &_rc);
	Rect* rc = new Rect();
	rc->left = _rc.left;
	rc->top = _rc.top;
	rc->right = _rc.right;
	rc->bottom = _rc.bottom;
	return rc->ReturnThis(a_thread);
}
#endif

int GM_CDECL Script_EnableConsoleDebug(gmThread* a_thread) {
	GM_CHECK_INT_PARAM(level, 0);
	SetLogLevel(level);
	return GM_OK;
}

int GM_CDECL Script_EnableConsoleColors(gmThread* a_thread) {
	GM_CHECK_INT_PARAM(enable, 0);
	SetLogColors(enable);
	return GM_OK;
}

int GM_CDECL Script_GetArguments(gmThread* a_thread) {
	if (a_thread->GetNumParams() == 0) { a_thread->PushInt(argc); return GM_OK; }
	GM_CHECK_INT_PARAM(index, 0);
	if (index >= 0 && index < argc) { a_thread->PushNewString(argv[index]); }
	else { a_thread->PushNewString(""); }
	return GM_OK;
}

int GM_CDECL Script_FcloseAll(gmThread* a_thread) {
	// TODO: Properly fix file handles
	int closed = fcloseall();
	if (closed) {
		Log(LOG_WARNING, "Closed %d open file streams.", closed);
	}
	return GM_OK;
}

#ifdef _WIN32
int GM_CDECL Script_MapKey(gmThread* a_thread) {
	GM_CHECK_INT_PARAM(code, 0);
	GM_CHECK_INT_PARAM(mode, 1);
	a_thread->PushInt(MapKey(code, mode));
	return GM_OK;
}
#endif


//#include <stdio.h>
extern "C" {
#include "markdown/markdown.h"
}
int GM_CDECL Script_Markdown(gmThread* a_thread) {
	GM_CHECK_STRING_PARAM(fileIn, 0);
	GM_CHECK_STRING_PARAM(fileOut, 1);
	
	FILE* fin = fopen(fileIn, "rb");
	FILE* fou = fopen(fileOut, "wb");
	if (!fin) { Log(LOG_ERROR, "Error while opening file '%s'!", fileIn); return GM_OK; }
	if (!fou) { Log(LOG_ERROR, "Error while opening file '%s'!", fileOut); return GM_OK; }
	
	//mkd_initialize();
	Document* md = mkd_in(fin, 0);
	mkd_compile(md, 0);
	mkd_generatehtml(md, fou);
	
	fclose(fin);
	fclose(fou);

	return GM_OK;
}


#include <gm/gmStreamBuffer.h>
#define STDCALL
typedef void(STDCALL* fpError)(int errorNumber, const char* errorMessage);
typedef char* (STDCALL* fpAlloc)(unsigned long memoryNeeded);
char* STDCALL AStyleMain(const char* pSourceIn, const char* pOptions, fpError fpErrorHandler, fpAlloc fpMemoryAlloc);
char* STDCALL ASMemoryAlloc(unsigned long memoryNeeded);
void  STDCALL ASErrorHandler(int errorNumber, const char* errorMessage);
int GM_CDECL Script_Include(gmThread* a_thread) {
	GM_CHECK_STRING_PARAM(file, 0);
	
	// find library file
	std::string libFile = FindFile(file);

	// load file content in a buffer
	coDword fileSize = 0;
	FileReadText(libFile.c_str(), 0, &fileSize);
	char* buffer = (char*)malloc(fileSize);
	FileReadText(libFile.c_str(), buffer, 0);

	// use AStyle on buffer
	const char* options = "--add-one-line-brackets --keep-one-line-blocks --keep-one-line-statements";
	char* textOut = AStyleMain(buffer, options, ASErrorHandler, ASMemoryAlloc);
	free(buffer);
	buffer = textOut;

	// check source for errors
	int errors = machine->CheckSyntax((char*)buffer);

	// show compile errors
	if(errors) {
		bool first = true;
		const char* message;

		while((message = machine->GetLog().GetEntry(first))) {
			char trimmedMessage[2048];
			strtrim(trimmedMessage, message);
			if(strlen(trimmedMessage) > 0) {
				Log(LOG_COMPILE, "%s:%s", libFile.c_str(), trimmedMessage);
			}
		}

		machine->GetLog().Reset();

		//free(buffer);
		delete buffer;
		
		//GM_EXCEPTION_MSG("file not found");
		//return GM_EXCEPTION;
		exit(-1);
		return GM_EXCEPTION;
	}

	// add source to machine
	gmStreamBufferDynamic *stream = new gmStreamBufferDynamic();
	machine->CompileStringToLib(buffer, *stream);
	machine->ExecuteLib(*stream, 0, true, file, 0);
	machine->AddSourceCode(buffer, file);
	delete stream;

	// free buffer
	//free(buffer);
	delete buffer;

	return GM_OK;
}


/*int GM_CDECL Script_Format(gmThread* a_thread) {

}*/

void RegisterCommonAPI() {
	srand(time(0));
	MachineRegisterFunction("_help", Script_GlobalHelp);
	MachineRegisterFunction("_classlist", Script_ClassList);
	MachineRegisterFunction("_freeMemory", Script_Collect);
	MachineRegisterFunction("_getMemoryUsage", Script_GetMemUsage);
	MachineRegisterFunction("_fcloseAll", Script_FcloseAll);
	MachineRegisterFunction("version", Script_GetVersion, " {string} version()", "Returns the version string.");
	MachineRegisterFunction("exit", Script_Exit, "exit()", "Terminates the program immediately.");
	MachineRegisterFunction("throw", Script_Throw, "throw((optional) {string} message)", "Throws a script error.");
	MachineRegisterFunction("ascii", Script_Ascii, " {string} ascii({int} ascii)", "Returns the string character for the given ascii code number.");
	MachineRegisterFunction("system", Script_System, " {string} system({string} command)", "Executes a command and redirects output via pipe so you can catch the output of the command.");
	MachineRegisterFunction("start", Script_System_Async, "{bool} start({string} command)", "Executes a command without waiting for it to finish, thus returning immediately.");
	MachineRegisterFunction("random", Script_Random, "{float} (or) {int} random( (optional) {float} (or) {int} max)", "Returns a random number between 0 and 'max', including 'max'. For example: random(2) may give 0, 1 or 2. If 'max' is a floating point number, the result is a floating point number too, otherwise its an integer.");
	MachineRegisterFunction("time", Script_Time, "{float} time((optional) {float} time)", "Gets or sets the internal timer (in seconds).");
	MachineRegisterFunction("print", Script_Print, "print({string})", "Outputs the string on the console, no newline is added.");
	MachineRegisterFunction("println", Script_PrintLine, "println({string})", "Outputs the string on the console, a newline is added.");
	MachineRegisterFunction("log", Script_Log, "log({string} message)", "Writes a log message to the console.");
	MachineRegisterFunction("warning", Script_Warning, "warning({string} error)", "Reports a warning on the console.");
	MachineRegisterFunction("error", Script_Error, "error({string} error)", "Reports a error on the console.");
	MachineRegisterFunction("fatal", Script_Fatal, "fatal({string} error)", "Reports a fatal error on the console and terminates the program immediately!");
	MachineRegisterFunction("popup", Script_Popup, "popup({string} info)", "Opens a message box with the given information.");
	MachineRegisterFunction("ask", Script_Ask, " {string} ask({string} question)", "Prompts the user for a line of input on the console.");
	MachineRegisterFunction("redirect", Script_Redirect, "redirect({string} file)", "Redirects the console output to a file of the given name.");
	MachineRegisterFunction("load", Script_LoadConfig, " {string} load({string} key)", "Loads a config string from 'config.ini'.");
	MachineRegisterFunction("save", Script_SaveConfig, "save({string} key, {string} value)", "Saves a config string into 'config.ini'. If it does not exist, the file will be created.");
	MachineRegisterFunction("instance", Script_CreateInstance, "{object} instance({string} class)", "Creates an instance from a class type name. Used for introspection.");
	MachineRegisterFunction("markdown", Script_Markdown, "markdown({string} fileIn, {string} fileOut)", "Loads a markdown file and saves the HTML to another file.");
	MachineRegisterFunction("pause", Script_Pause, "pause( {float} s)", "Pauses the execution of the whole process for the given time in seconds. Reduces CPU usage of real-time applications.");
	MachineRegisterFunction("eval", Script_Eval, "eval({string} code)", "Execute given code on the same script context, meaning, it has access to global functions and variables.");
	MachineRegisterFunction("callstack", Script_Callstack, "{string} callstack()", "Returns the current callstack as a string.");
	MachineRegisterFunction("debug", Script_EnableConsoleDebug, "debug({int} level)", "Enable/Disable console debug messages.");
	MachineRegisterFunction("colors", Script_EnableConsoleColors, "colors({int} enable)", "Enable/Disable console colors.");
	MachineRegisterFunction("arg", Script_GetArguments, "{string} arg({int} number)", "Get command-line arguments.");
	MachineRegisterFunction("include", Script_Include, "include({string} file)", "Includes a script file.");
#ifdef _WIN32
	// TODO: implement at least sound() and askFile() for linux too.
	MachineRegisterFunction("sound", Script_PlaySound, "sound({string} file)", "Plays a sound file. (This is a simple and inefficient way to play a sound file)");
	MachineRegisterFunction("askFile", Script_InputFile, "{string} askFile()", "Opens a dialog where the user can select a file.");
	MachineRegisterFunction("monitor", Script_GetMonitorRect, "[Rect] monitor({int} index)", "Gets the rectangle area of the given monitor in virtual screen space.");
	MachineRegisterFunction("_mapKey", Script_MapKey, "{int} _mapKey({int} key, {int} mode)", "Translate from/to scancode to virtual-keycode or vice versa.");
#endif

	// only documentation here
	MachineRegisterFunction("yield", 0, "yield()", "Gives control to the script-runtime and the next thread/coroutine. It is important to yield in a main-loop and in threads/coroutines otherwise the applications may not run correctly and garbage-collection will never take place.");
	MachineRegisterFunction("sleep", 0, "sleep({float} time)", "Same as yield() but continues the current thread/coroutine only after the given time, effectively let the thread/coroutine sleep.");
	MachineRegisterFunction("type", 0, "{string} type({any} any)", "Returns the name of the type of the given parameter.");
	MachineRegisterFunction("format", 0, "{string} format({string} format, ...)", "Like printf() in C. Formats a string by using %d, %s, %f, %c, %b, %x, %e and multiple input parameters.");

	// math functions
	MachineRegisterFunction("abs, floor, ceil, round, cos, sin, tan, acos, asin, deg2rad, rad2deg", 0, "{float} ...({float} p)", "Common math functions with one parameters.");
	MachineRegisterFunction("sqrt, power, atan, atan2, log, min, max", 0, "{float} ...({float} x, {float} y)", "Common math functions two parameters.");
	MachineRegisterFunction("clamp", 0, "{float} clamp({float} min, {float} value, {float} max)", "Returns the clamped value.");

	// string functions
	MachineRegisterFunction("string.length", 0, "{int}  [string].length()", "Returns the length of the given string, use dot notation on the string: local len = myString.length();");
	MachineRegisterFunction("string.upper", 0, "{string}  [string].upper()", "Returns a upper case copy of the string, use dot notation on the string: local myUpper = myString.upper();");
	MachineRegisterFunction("string.lower", 0, "{string}  [string].lower()", "Returns a lower case copy of the string, use dot notation on the string: local myLower = myString.lower();");
	MachineRegisterFunction("string.find", 0, "{int}  [string].find({string} search, {int} startIndex)", "Returns the index of the first occurance of the search-string or -1. Use dot notation on string to call this function.");
	MachineRegisterFunction("string.getAt, string.setAt", 0, "{string}  [string].setAt({int} index, {int} set)", "Gets or sets the character at the given index. Use dot notation on string to call this function.");
	MachineRegisterFunction("string.trimLeft, string.trimRight", 0, "{string}  [string].trimLeft({string} str)", "Returns a trimmed copy of original string. Use dot notation on string to call this function.");
}

