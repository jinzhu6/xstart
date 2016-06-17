#ifndef _CORELA_T_
#define _CORELA_T_

#include "cont.h"

#ifndef _OPENMP
#define _OPENMP 1
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <GL/gl.h>

#ifdef CO_LINK_DLL
#ifdef CO_BUILDING
#define CO_INTERFACE		__declspec(dllexport)
#else
#define CO_INTERFACE		__declspec(dllimport)
#endif
#else
#define CO_INTERFACE
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif


typedef bool coBool;
typedef unsigned char coByte;
typedef unsigned long coDword;
typedef unsigned long coPointer;
typedef char* cstring;

#define coTrue 1
#define coFalse 0

typedef enum LOG_TYPE {
	LOG_INFO = 0,
	LOG_POPUP,
	LOG_NOBREAK,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL,
	LOG_DEBUG,
	LOG_NOBREAK_DEBUG,
	LOG_SCRIPT,
	LOG_COMPILE,
	LOG_USER
} LOG_TYPE;

#ifdef _MSC_VER
#pragma pack(push,1)
typedef unsigned char BYTE;
#pragma pack(pop)
#pragma pack(push,4)
struct PIXEL {
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
};
#pragma pack(pop)
#else
typedef unsigned char BYTE  __attribute__((aligned (1)));
struct __attribute__ ((__packed__)) PIXEL {
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
} __attribute__((aligned (4)));
#endif

typedef struct IMAGE {
	PIXEL* data;
	int width, height;
} IMAGE;

typedef struct CORECT {
	long left;
	long top;
	long right;
	long bottom;
} CORECT;

/*
typedef struct MONITOR_RECT {
	long x, y, w, h;
} MONITOR_RECT;

typedef struct MONITOR_RESOLUTION {
	long w, h, bpp, hz;
} MONITOR_RESOLUTION;

typedef struct MONITOR_INFO {
	unsigned long handle;
	MONITOR_RECT vdesk;
	MONITOR_RESOLUTION res[256];
	int numRes;
	int primary;
	char device[256];
} MONITOR_INFO;*/

struct FRAME_EVENT;

typedef int(*EVENT_CALLBACK)(FRAME_EVENT*);

typedef struct FRAME {
	void* hwnd;
#ifdef _WIN32
	HDC hdc;
	HGLRC rc;
#endif
	void* user;
	float width;
	float height;
	float vwidth;
	float vheight;
	EVENT_CALLBACK cb;
	bool showCursor;
} FRAME;

typedef enum EVENT_ID {
	EVENT_CLOSE,
	EVENT_UPDATE,
	EVENT_RENDER,
	EVENT_MOUSE_MOVE,
	EVENT_MOUSE_BUTTON_DOWN,
	EVENT_MOUSE_BUTTON_UP,
	EVENT_KEY_DOWN,
	EVENT_KEY_UP,
	EVENT_END
} EVENT_ID;

typedef struct FRAME_EVENT {
	EVENT_ID id;
	FRAME* sender;
	void* user;
	double x, y;
	int button, key;
} FRAME_EVENT;


#define TEX_CLAMP 1
#define TEX_NOFILTER 2
#define TEX_SHARP 4
#define TEX_NOMIPMAP 8
#define TEX_COMPRESS 16
#define TEX_NOALPHA 32  // TODO: Check if this works correct with PBO.
#define TEX_TARGET 64
#define TEX_CLEAR 128
#define TEX_PBO 256

#define TEX_FORMAT_RGB 0x1907
#define TEX_FORMAT_RGBA 0x1908
#define TEX_FORMAT_BGR 0x80E0
#define TEX_FORMAT_BGRA 0x80E1
#define TEX_FORMAT_RED8 0x1903
#define TEX_FORMAT_INTENSITY 0x804B

typedef struct TEXTURE_RECT {
	double left,top,right,bottom;
} TEXTURE_RECT;

typedef struct TEXTURE {
	unsigned int handle;
	coDword target;
	coDword flags;
	coDword internalFormat;
	coDword vwidth, vheight;
	coDword width, height;
	TEXTURE_RECT drawRect;
	GLuint pbo;
	coByte* pLockMem;
} TEXTURE;

#ifndef _FONT_CPP
typedef struct COFONT {
	coPointer p;
} COFONT;
#endif

struct FONT;

typedef struct GLSLSHADER {
	int vs;
	int fs;
	int prg;
	int dwRef;
} GLSLSHADER;

#define MAX_INI_FILE 4096
#define MAX_INI_KEY 64
#define MAX_INI_VALUE 4096
typedef struct INI {
	bool		isDirty;
	CONT*		cSections;
	char		szFile[MAX_INI_FILE];
} INI;
typedef struct INISECTION {
	INI*		ini;
	bool		isDirty;
	CONT*		cEntry;
	char		szName[MAX_INI_KEY];
} INISECTION;
typedef struct INIENTRY {
	INI*		ini;
	INISECTION* sec;
	char		szKey[MAX_INI_KEY];
	char		szValue[MAX_INI_VALUE];
	coBool      bInQuotes;                  // value was in quotes when loaded
} INIENTRY;


#endif
