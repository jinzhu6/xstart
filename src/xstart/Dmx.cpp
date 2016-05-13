#ifdef _WIN32

#include <Windows.h>
#include <stdio.h>
#include "Dmx.h"

#define DHC_SIUDI0				0		// COMMAND
#define DHC_SIUDI1				100		// COMMAND
#define DHC_SIUDI2				200		// COMMAND
#define DHC_SIUDI3				300		// COMMAND
#define DHC_SIUDI4				400		// COMMAND
#define DHC_SIUDI5				500		// COMMAND
#define DHC_SIUDI6				600		// COMMAND
#define DHC_SIUDI7				700		// COMMAND
#define DHC_SIUDI8				800		// COMMAND
#define DHC_SIUDI9				900		// COMMAND
#define DHC_OPEN				1		// COMMAND
#define DHC_CLOSE				2		// COMMAND
#define DHC_DMXOUTOFF			3		// COMMAND
#define DHC_DMXOUT				4		// COMMAND
#define DHC_PORTREAD			5		// COMMAND
#define DHC_PORTCONFIG			6		// COMMAND
#define DHC_VERSION				7		// COMMAND
#define DHC_DMXIN				8		// COMMAND
#define DHC_INIT				9		// COMMAND
#define DHC_EXIT				10		// COMMAND
#define DHC_DMXSCODE			11		// COMMAND
#define DHC_DMX2ENABLE			12		// COMMAND
#define DHC_DMX2OUT				13		// COMMAND
#define DHC_SERIAL				14		// COMMAND
#define DHC_TRANSPORT			15		// COMMAND
#define DHC_DMXENABLE			16		// COMMAND
#define DHC_DMX3ENABLE			17		// COMMAND
#define DHC_DMX3OUT				18		// COMMAND
#define DHC_DMX2IN				19		// COMMAND
#define DHC_DMX3IN				20		// COMMAND
#define DHC_WRITEMEMORY			21		// COMMAND
#define DHC_READMEMORY			22		// COMMAND
#define DHC_SIZEMEMORY			23		// COMMAND

#define DHE_OK					1		// RETURN NO ERROR
#define DHE_NOTHINGTODO			2		// RETURN NO ERROR
#define DHE_ERROR_COMMAND		-1		// RETURN ERROR
#define DHE_ERROR_NOTOPEN		-2		// RETURN ERROR
#define DHE_ERROR_ALREADYOPEN	-12		// RETURN ERROR

typedef struct {
	WORD year;
	WORD month;
	WORD dayOfWeek;
	WORD date;
	WORD hour;
	WORD min;
	WORD sec;
	WORD milliseconds;
} STIME;

#define MAX_CHANNELS 512
#define CMDCALL __cdecl
int (CMDCALL* DasUsbCommand) (int, int, unsigned char*);
int (CMDCALL* DasNetCommand) (int, int, unsigned char*);

static unsigned char dmxblock[MAX_UNIVERSE][512];
static unsigned int mapping[MAX_MAPPING][MAX_MAPPING];
static int offset[MAX_UNIVERSE];
static int dirty[MAX_UNIVERSE];
static int dmx_connected = 0;

void DMX_Init() {
	HMODULE hDLL;

	// load DLL and dmx command
	hDLL = LoadLibraryA("DasHard2006.dll");
	if(hDLL) {
		DasUsbCommand = (int (CMDCALL*)(int, int, unsigned char*))GetProcAddress(hDLL, "DasUsbCommand");
		if(DasUsbCommand) {
			// initialize dmx
			DasUsbCommand(DHC_INIT, 0, 0);

			// open device
			int open = DasUsbCommand(DHC_OPEN, 0, 0);
			if(open <= 0) {
				Log(LOG_ERROR, "Error while connecting to DMX device! (code: %d)", open);
				return;
			}
		} else {
			Log(LOG_ERROR, "Error while binding DMX command!");
			return;
		}
	} else {
		Log(LOG_ERROR, "Error while loading 'DasHard2006.dll'!");
		return;
	}

	for(int n=0, y=0; y<MAX_MAPPING; y++) {
		for(int x=0; x<MAX_MAPPING; x++) {
			mapping[x][y] = n++;
		}
	}

	for(int i=0; i<MAX_UNIVERSE; i++) {
		offset[i] = 0;
		dirty[i] = 0;
	}

	dmx_connected = 1;
	DMX_ClearTo(0,0);
}

void DMX_Exit() {
	if(!dmx_connected) { return; }

	DasUsbCommand(DHC_CLOSE, 0, 0);
	DasUsbCommand(DHC_EXIT, 0, 0);
}

void DMX_ClearTo(unsigned int u, unsigned char v) {
	if(!dmx_connected) { return; }

	// nullify dmxblock
	for(int i=0; i<MAX_CHANNELS; i++) {
		dmxblock[u][i] = v;
	}
	dirty[u] = true;
}

void DMX_SetUniverseOffset(unsigned int u, int ch) {
	if(!dmx_connected) { return; }

	if(u >= MAX_UNIVERSE) {
		return;
	}

	offset[u] = ch;
}

void DMX_SetChannel(unsigned int u, int ch, unsigned char v) {
	if(!dmx_connected) { return; }

	if(u >= MAX_UNIVERSE) { return; }

	ch += offset[u];

	if(ch < 0 || ch >= MAX_CHANNELS) {
		return;
	}

	dmxblock[u][ch] = v;
	dirty[u] = true;
}

void DMX_SetPixelMapping(unsigned int x, unsigned int y, unsigned int ch_u) {
	if(!dmx_connected) { return; }
	if(x >= MAX_MAPPING || y >= MAX_MAPPING) {
		return;
	}
	mapping[y][x] = ch_u;
}

void DMX_SetPixelXY(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b) {
	if(!dmx_connected) { return; }

	if(x >= MAX_MAPPING || y >= MAX_MAPPING) { return; }

	int m = mapping[y][x];
	int u = (m & 0xF0000000) >> 28;
	int ch = m & 0x0FFFFFFF;

	if(ch == DMX_SKIP) {
		return;
	}

	ch += offset[u];
	if(!DasUsbCommand) { return; }
	DMX_SetChannel(u, ch, r);
	DMX_SetChannel(u, ch+1, g);
	DMX_SetChannel(u, ch+2, b);
}

void DMX_Update() {
	if(!dmx_connected) { return; }
	int res;
	if(dirty[0]) {
		res = DasUsbCommand(DHC_DMXOUT, 512, dmxblock[0]);
		if(res < 0) {
			DasUsbCommand(DHC_CLOSE,0,0);
			DasUsbCommand(DHC_OPEN,0,0);
			res = DasUsbCommand(DHC_DMXOUT, 512, dmxblock[0]);
			if(res < 0) { Log(LOG_ERROR, "DMX Device lost, recovery failed!"); }
		}
		dirty[0] = false;
	}
	if(dirty[1]) {
		res = DasUsbCommand(DHC_DMX2OUT, 512, dmxblock[1]);
		if(res < 0) {
			DasUsbCommand(DHC_CLOSE,0,0);
			DasUsbCommand(DHC_OPEN,0,0);
			res = DasUsbCommand(DHC_DMXOUT, 512, dmxblock[0]);
			if(res < 0) { Log(LOG_ERROR, "DMX Device lost, recovery failed!"); }
		}
		dirty[1] = false;
	}
	if(dirty[2]) {
		res = DasUsbCommand(DHC_DMX3OUT, 512, dmxblock[2]);
		if(res < 0) {
			DasUsbCommand(DHC_CLOSE,0,0);
			DasUsbCommand(DHC_OPEN,0,0);
			res = DasUsbCommand(DHC_DMXOUT, 512, dmxblock[0]);
			if(res < 0) { Log(LOG_ERROR, "DMX Device lost, recovery failed!"); }
		}
		dirty[2] = false;
	}
}

#endif
