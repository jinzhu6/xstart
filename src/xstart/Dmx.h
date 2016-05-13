#ifdef _WIN32

#ifndef _DMX_H_
#define _DMX_H_

#include <corela.h>
#include "ScriptObject.h"

#define MAX_UNIVERSE 3
#define MAX_MAPPING 256
#define DMX_SKIP 0x8000

#define DMX_UNIVERSE0 0
#define DMX_UNIVERSE1 1
#define DMX_UNIVERSE2 2

extern void DMX_Init(void);
extern void DMX_Exit(void);
extern void DMX_ClearTo(unsigned int u, unsigned char v);
extern void DMX_Update();
extern void DMX_SetUniverseOffset(unsigned int u, int ch);
extern void DMX_SetChannel(unsigned int u, int ch, unsigned char v);
extern void DMX_SetPixelMapping(unsigned int x, unsigned int y, unsigned int ch);
extern void DMX_SetPixelXY(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);

class Dmx : public ScriptObject {
public:

	Dmx() : ScriptObject() {
		id = "Dmx";
		help = "USB-DMX (DMX-512) device connection through DasHard DLL. Allows optional mapping of channel to X,Y coordinates (setMapping, setPixel), or direct access (setChannel). Initialization happens on object creation, no parameters are needed.";
		DMX_Init();

		BindFunction("clearTo", (SCRIPT_FUNCTION)&Dmx::gm_clearTo);
		BindFunction("setOffset", (SCRIPT_FUNCTION)&Dmx::gm_setOffset);
		BindFunction("setChannel", (SCRIPT_FUNCTION)&Dmx::gm_setChannel);
		BindFunction("setMapping", (SCRIPT_FUNCTION)&Dmx::gm_setMapping);
		BindFunction("setPixel", (SCRIPT_FUNCTION)&Dmx::gm_setPixel);
		BindFunction("update", (SCRIPT_FUNCTION)&Dmx::gm_update);
	}

	~Dmx() { DMX_Exit(); }

	void clearTo(unsigned int u, unsigned char v) { DMX_ClearTo(u,v); }
	void update() { DMX_Update(); }
	void setOffset(unsigned int u, int ch) { DMX_SetUniverseOffset(u,ch); }
	void setChannel(unsigned int u, int ch, unsigned char v) { DMX_SetChannel(u,ch,v); }
	void setMapping(unsigned int x, unsigned int y, unsigned int ch) { DMX_SetPixelMapping(x,y,ch); }
	void setPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b) { DMX_SetPixelXY(x,y,r,g,b); }

	int gm_clearTo(gmThread* a_thread) { GM_CHECK_INT_PARAM(u,0); GM_CHECK_INT_PARAM(v,1); clearTo(u,v); return GM_OK; }
	int gm_setOffset(gmThread* a_thread) { GM_CHECK_INT_PARAM(u,0); GM_CHECK_INT_PARAM(ch,1); setOffset(u,ch); return GM_OK; }
	int gm_setChannel(gmThread* a_thread) { GM_CHECK_INT_PARAM(u,0); GM_CHECK_INT_PARAM(ch,1); GM_CHECK_INT_PARAM(v,2); setChannel(u,ch,v); return GM_OK; }
	int gm_setMapping(gmThread* a_thread) { GM_CHECK_INT_PARAM(x,0); GM_CHECK_INT_PARAM(y,1); GM_CHECK_INT_PARAM(ch,2); setMapping(x,y,ch); return GM_OK; }
	int gm_setPixel(gmThread* a_thread) { GM_CHECK_INT_PARAM(x,0); GM_CHECK_INT_PARAM(y,1); GM_CHECK_INT_PARAM(r,2); GM_CHECK_INT_PARAM(g,3); GM_CHECK_INT_PARAM(b,4); DMX_SetPixelXY(x,y,r,g,b); return GM_OK; }
	int gm_update(gmThread* a_thread) { update(); return GM_OK; }
};

#endif

#endif
