#include <windows.h> /* for sleep */
#include <process.h>
#include <assert.h>
#include "sdy_drv.h"
#include "sdy_audlines.h"
#include "sdy_manage.h"

#include "strm_wav.h"
//#include "strm_mod.h"
#include "strm_ogg.h"
//#include "strm_flac.h"
//#include "strm_speex.h"

#include "sdy_loader.h"

volatile static sdyBool g_bInit;
volatile static sdyBool g_bInUpdate;
static sdyWord g_wBufSec;

/* -------------------------------------------------------------------------------------
// sdyRelease
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdyRelease(void) {
	if(g_bInit) {
		g_bInit = sdyFalse;

		while(g_bInUpdate) { }

		sdyALShutdown();
		sdyUnregAllLoaders();
		sdyDrvRelease();
	}
}

/* -------------------------------------------------------------------------------------
// sdyOpen
//------------------------------------------------------------------------------------- */
SDY_INTERFACE SDY_SOUND* SDYAPI sdyOpen(const sdyString szFile, sdyBool bStatic, sdyBool b3dSound) {
	SDY_SOUND* pSound;

	pSound = 0;

	if(g_bInit) {
		pSound = sdyManOpenSound(szFile, bStatic, b3dSound);
	}

	return pSound;
}

/* -------------------------------------------------------------------------------------
// sdyClose
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdyClose(SDY_SOUND* pSound) {
	assert(pSound);
	if(!pSound) {
		return;
	}

	sdyManCloseSound(pSound);
}

/* -------------------------------------------------------------------------------------
// sdyPlay
//------------------------------------------------------------------------------------- */
SDY_INTERFACE SDY_SOUND* SDYAPI sdyPlay(SDY_SOUND* pSound, sdyBool bLoop) {
	assert(pSound);
	if(!pSound) {
		return sdyFalse;
	}

	return sdyALPlay(pSound, bLoop);
}

/* -------------------------------------------------------------------------------------
// sdySetFreq
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdySetFreq(SDY_SOUND* pSound, sdyDword dwFreq) {
	sdyDrvSetFreq(pSound->pBuffer, dwFreq);
}

/* -------------------------------------------------------------------------------------
// sdySetVol
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdySetVol(SDY_SOUND* pSound, sdyDword dwVol) {
	sdyDrvSetVol(pSound->pBuffer, dwVol);
}

/* -------------------------------------------------------------------------------------
// sdySetPan
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdySetPan(SDY_SOUND* pSound, long nPan) {
	sdyDrvSetPan(pSound->pBuffer, nPan);
}

/* -------------------------------------------------------------------------------------
// sdySetMaxCopies
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdySetMaxCopies(SDY_SOUND* pSound, sdyDword dwMaxDupli) {
	pSound->dwMaxDuplicates = dwMaxDupli;
}

/* -------------------------------------------------------------------------------------
// sdyStop
//------------------------------------------------------------------------------------- */
SDY_INTERFACE sdyBool SDYAPI sdyStop(SDY_SOUND* pSound) {
	assert(pSound);
	if(!pSound) {
		return sdyFalse;
	}

	sdyALStop(pSound);

	return sdyTrue;
}

/* -------------------------------------------------------------------------------------
// sdyIsPlaying
//------------------------------------------------------------------------------------- */
SDY_INTERFACE sdyBool SDYAPI sdyIsPlaying(SDY_SOUND* pSound) {
	return sdyALIsPlaying(pSound);
}

/* -------------------------------------------------------------------------------------
// sdySetPriority
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdySetPriority(SDY_SOUND* pSound, sdyInt nPriority) {
	assert(pSound);
	if(!pSound) {
		return;
	}
	pSound->nPriority = nPriority;
}

/* -------------------------------------------------------------------------------------
// sdyUpdate
//------------------------------------------------------------------------------------- */
sdyBool SDYAPI sdyUpdate(void) {
	if(!g_bInit) {
		return sdyFalse;
	}

	while(g_bInUpdate);

	g_bInUpdate = sdyTrue;

	sdyALUpdate();
	sdyDrvUpdate3DBuffers();

	g_bInUpdate = sdyFalse;

	return sdyTrue;
}

/* -------------------------------------------------------------------------------------
// thread_soundy
//------------------------------------------------------------------------------------- */
void thread_soundy(void* p) {
	while(sdyUpdate()) {
#ifndef SLEEPING_THREAD
#pragma message("*** Specify SLEEPING_THREAD to either 0 or 1 ***")
#endif
#if SLEEPING_THREAD == 1
		Sleep(g_wBufSec / 4);
#endif
	}
}

/* -------------------------------------------------------------------------------------
// sdyInit
//------------------------------------------------------------------------------------- */
SDY_INTERFACE sdyBool SDYAPI sdyInit(long _hWnd, SDY_SETTINGS* pSettings) {
	SDY_SETTINGS defSettings;

	if(pSettings == 0) {
		defSettings.wChannels			= DEF_CHANNELS;
		defSettings.dwSamplesPerSec 	= DEF_SAMPLES;
		defSettings.wBitsPerSample		= DEF_BPS;
		defSettings.wBufSec 			= DEF_BUFSEC; /* 240 */
		defSettings.wAudLines			= DEF_AUDLINES;
		defSettings.b3DSound			= DEF_3DSOUND;
		defSettings.bManualUpdate		= DEF_MANUALUPDATE;
		pSettings = &defSettings;
	}
	if(pSettings->wBufSec == 0 || pSettings->wBufSec > MAX_BUFSEC) {
		pSettings->wBufSec = MAX_BUFSEC;
	}
	if(pSettings->wAudLines == 0 || pSettings->wAudLines > MAX_AUDLINES) {
		pSettings->wBufSec = MAX_AUDLINES;
	}

	g_bInit = sdyDrvInit(_hWnd, pSettings);

	if(g_bInit) {
		sdyALInitialize(pSettings);
		sdyRegLoader(PCM_Open, PCM_Stream, PCM_Close);
//		sdyRegLoader(MOD_Open, MOD_Stream, MOD_Close);
		sdyRegLoader(OGG_Open, OGG_Stream, OGG_Close);
//		sdyRegLoader(FLAC_Open,FLAC_Stream,FLAC_Close);
//		sdyRegLoader(SPEEX_Open,SPEEX_Stream,SPEEX_Close);

		if(!pSettings->bManualUpdate) {
			_beginthread(thread_soundy, 0, 0);
		}
	}

	g_wBufSec = pSettings->wBufSec;

	return g_bInit;
}
