#include "corela.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
//#include <varargs.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#endif

static int g_LogUseColors = 1;
static int g_LogLevel = 1;  // 0=silent, 1=normal, 2=debug

#define plat_vsnprintf vsnprintf // _vsnprintf

typedef enum CONSOLE_COLORS { CON_NORMAL=0, CON_WARNING, CON_ERROR, CON_SCRIPT, CON_DEBUG, CON_FATAL, CON_INFO, CON_COMPILE, CON_USER };

double _startTime = 0.0; //TimeGet();

void SetLogLevel(int level) {
	g_LogLevel = level;
}

int GetLogLevel() {
	return g_LogLevel;
}

void SetLogColors(int enable) {
	g_LogUseColors = enable;
}

void SetConsoleTextColor(int color) {
#ifdef _WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE) { return; }
	if(color == CON_NORMAL) { SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); }
	else if(color == CON_WARNING) { SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); }
	else if(color == CON_ERROR) { SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED); }
	else if(color == CON_SCRIPT) { SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); }
	else if(color == CON_DEBUG) { SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); }
	else if(color == CON_FATAL) { SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED); }
	else if(color == CON_INFO) { SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); }
	else if(color == CON_COMPILE) { SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE); }
	else if(color == CON_USER) { SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); }
	return;
#endif
	if(color == CON_NORMAL) { printf("\x1B[0m"); }
	else if(color == CON_WARNING) { printf("\x1B[33m"); }
	else if(color == CON_ERROR) { printf("\x1B[31m"); }
	else if(color == CON_SCRIPT) { printf("\x1B[36m"); }
	else if(color == CON_DEBUG) { printf("\x1B[33m"); }
	else if(color == CON_FATAL) { printf("\x1B[35m"); }
	else if(color == CON_INFO) { printf("\x1B[32m"); }
	else if(color == CON_COMPILE) { printf("\x1B[31m"); }
	else if(color == CON_USER) { printf("\x1B[32m"); }
	return;
}

void StromgReplaceAll(std::string& str, const std::string& from, const std::string& to) {
	if(from.empty()) {
		return;
	}
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

void ErrorBox(const char* _error) {
#ifdef _WIN32
	MessageBox(GetForegroundWindow(), _error, "ERROR", MB_OK | MB_ICONERROR);
#ifdef _DEBUG
	if(g_LogLevel >= 1) { __debugbreak(); }
#endif
#else
	std::string error;
	error = _error;
	StromgReplaceAll(error, "\"", "\\\"");
	StromgReplaceAll(error, "!", "\\!");
	StromgReplaceAll(error, "\n", "\n\\");
	error = std::string("zenity --error --title=\"ERROR\" --text=\"") + error + std::string("\"");
	system(error.c_str());
	raise(SIGTRAP);
#endif
}

void InfoBox(const char* _info) {
#ifdef _WIN32
	MessageBox(GetForegroundWindow(), _info, "Information", MB_OK | MB_ICONINFORMATION);
#else
	std::string info = _info;
	StromgReplaceAll(info, "\"", "\\\"");
	StromgReplaceAll(info, "!", "\\!");
	StromgReplaceAll(info, "\n", "\n\\");
	info = std::string("zenity --info --title=\"Information\" --text=\"") + info + std::string("\"");
	system(info.c_str());
#endif
}

LOG_TYPE _prevLogType = LOG_NOBREAK;
void Log(LOG_TYPE type, const char* format, ...) {
	if (g_LogLevel == 0) { return; }

	// TODO: Currently limit by this size!
	char buffer[2048*8];
	va_list fparams;

	if(format) {
		va_start(fparams, format);
		plat_vsnprintf(buffer, 2047*8, format, fparams);
		va_end(fparams);
	} else {
		strcpy(buffer, "");
	}

	int color = CON_NORMAL;
	char caption[512];

	switch(type) {
	case LOG_INFO:
	case LOG_NOBREAK:
		color = CON_INFO;
		strcpy(caption, "");
		break;
	case LOG_POPUP:
		color = CON_INFO;
		strcpy(caption, "POPUP: ");
		break;
	case LOG_WARNING:
		color = CON_WARNING;
		strcpy(caption, "WARNING: ");
		break;
	case LOG_ERROR:
		color = CON_ERROR;
		strcpy(caption, "ERROR: \n------------------------------------------------------------------------\n");
		strcat(buffer, "\n------------------------------------------------------------------------");
		break;
	case LOG_FATAL:
		color = CON_FATAL;
		strcpy(caption, "FATAL-ERROR: ");
		break;
	case LOG_DEBUG:
		if(g_LogLevel < 2) { return; }
		color = CON_DEBUG;
		strcpy(caption, "DEBUG-INFO: ");
		break;
	case LOG_NOBREAK_DEBUG:
		if(g_LogLevel < 2) { return; }
		color = CON_INFO;
		strcpy(caption, "");
		break;
	case LOG_SCRIPT:
		color = CON_SCRIPT;
		strcpy(caption, "SCRIPT: ");
		break;
	case LOG_COMPILE:
		color = CON_COMPILE;
		strcpy(caption, "COMPILE: ");
		break;
	case LOG_USER:
		color = CON_USER;
		strcpy(caption, "USER: ");
		break;
	}

	double elapsed = TimeGet() - _startTime;

//	time_t t = time(0);
//	struct tm lt = *localtime(&t);

	if(g_LogUseColors) { SetConsoleTextColor(color); }

	if(type == LOG_NOBREAK || type == LOG_NOBREAK_DEBUG) {
		printf("%s%s", caption, buffer);
	} else {
		//printf("%02d-%02d-%02d |  %s%s\n", lt.tm_hour, lt.tm_min, lt.tm_sec, caption, buffer);
		if(_prevLogType == LOG_NOBREAK || _prevLogType == LOG_NOBREAK_DEBUG) { printf("\n"); }
		printf("%.3f |  %s%s\n", elapsed, caption, buffer);
	}

	fflush(stdout);

	_prevLogType = type;

	if(g_LogUseColors) { SetConsoleTextColor(0); }

	switch(type) {
	case LOG_ERROR:
#ifdef _DEBUG
		ErrorBox(buffer);
#endif
		break;
	case LOG_FATAL:
		ErrorBox(buffer);
		exit(-1);
		break;
	case LOG_POPUP:
		InfoBox(buffer);
		break;
	}

}
