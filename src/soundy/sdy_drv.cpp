#include <dsound.h>
#include <stdio.h>
#include <assert.h>
#include "sdy_drv.h"

#define SAFE_RELEASE(x) {if(x){(x)->Release();(x)=0;}}

static LPDIRECTSOUND				lpDS;
static LPDIRECTSOUNDBUFFER			lpDSBPrimary;
static LPDIRECTSOUND3DLISTENER		lpDS3DListener;

static int		nBufCount;
static sdyBool	g_b3DSound;


/* -------------------------------------------------------------------------------------
// _sdyDrvSetPrimaryFormat
//------------------------------------------------------------------------------------- */
static int _sdyDrvSetPrimaryFormat(sdyWord nChannels, sdyDword nSamples, sdyWord wBPS)
{
	WAVEFORMATEX wfx;
	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag		= WAVE_FORMAT_PCM;
	wfx.nChannels		= nChannels;
	wfx.nSamplesPerSec	= nSamples;
	wfx.wBitsPerSample	= wBPS;
	wfx.nBlockAlign 	= wfx.wBitsPerSample / 8 * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	
	if FAILED(lpDSBPrimary->SetFormat(&wfx))
		return 0;
	
	return 1;
}


/* -------------------------------------------------------------------------------------
// sdyDrvInit
//------------------------------------------------------------------------------------- */
sdyBool sdyDrvInit(long _hWnd, SDY_SETTINGS* pSettings)
{
	HWND hWnd = (HWND)_hWnd;
	
	// Obtain window handle
	if(!hWnd)
		hWnd = GetForegroundWindow();
	if(!hWnd)
		hWnd = GetDesktopWindow();
	
	// Initialize COM object
	if FAILED(DirectSoundCreate(0, &lpDS, NULL))
	{	
		return sdyFalse;
	}
	
	// Request cooperative level
	if FAILED(lpDS->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE | DSSCL_PRIORITY))
	{
		lpDS->Release();
		return sdyFalse;
	}
	
	// Obtain primary buffer
	DSBUFFERDESC dsbd;
	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_STICKYFOCUS;
	if(pSettings->b3DSound)
	{
		dsbd.dwFlags |= DSBCAPS_CTRL3D;
//		dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;
	}
	if FAILED(lpDS->CreateSoundBuffer(&dsbd, &lpDSBPrimary, NULL))
	{
		lpDS->Release();
		return sdyFalse;
	}
	
	// Set primary buffer format
	if(!_sdyDrvSetPrimaryFormat(pSettings->wChannels, pSettings->dwSamplesPerSec, pSettings->wBitsPerSample))
	{
		if(!_sdyDrvSetPrimaryFormat(2, 44100, 16))
		{
			if(!_sdyDrvSetPrimaryFormat(2, 22050, 16))
			{
				if(!_sdyDrvSetPrimaryFormat(2, 22050, 8))
				{
					lpDSBPrimary->Release();
					lpDS->Release();
					return sdyFalse;
				}
				else
				{
					pSettings->wChannels = 1;
					pSettings->dwSamplesPerSec = 22050;
					pSettings->wBitsPerSample = 8;				
				}
			}
			else
			{
				pSettings->wChannels = 2;
				pSettings->dwSamplesPerSec = 22050;
				pSettings->wBitsPerSample = 16;
			}
		}
		else
		{
			pSettings->wChannels = 2;
			pSettings->dwSamplesPerSec = 44100;
			pSettings->wBitsPerSample = 16; 		
		}
	}
	
	// 3D Sound: Get handle to listener
	if(pSettings->b3DSound)
	{
		lpDSBPrimary->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&lpDS3DListener);
	}
	
	return sdyTrue;
}


/* -------------------------------------------------------------------------------------
// sdyDrvRelease
//------------------------------------------------------------------------------------- */
void sdyDrvRelease(void)
{
	assert(!nBufCount);
	
	SAFE_RELEASE(lpDS3DListener);
	SAFE_RELEASE(lpDSBPrimary);
	SAFE_RELEASE(lpDS);
	
	nBufCount = 0;
}


/* -------------------------------------------------------------------------------------
// sdyDrvCreateBuffer
//------------------------------------------------------------------------------------- */
void* sdyDrvCreateBuffer(SDY_WFX* pWFX, DWORD dwBufBytes, sdyBool b3DBuffer)
{
	LPDIRECTSOUNDBUFFER lpDSB;
	DSBUFFERDESC dsbd;
	
	assert(lpDS);
	if(!lpDS) return 0;
	
	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize 		= sizeof(DSBUFFERDESC);
	dsbd.dwBufferBytes	= dwBufBytes;
	dsbd.lpwfxFormat	= (WAVEFORMATEX*)pWFX;
	
	if(b3DBuffer)
		dsbd.dwFlags	= /*DSBCAPS_LOCSOFTWARE | */DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
	else
		dsbd.dwFlags	= /*DSBCAPS_LOCSOFTWARE | */DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;
	
	if FAILED(lpDS->CreateSoundBuffer(&dsbd, &lpDSB, 0))
	{
		return 0;
	}
	else 
	{
		nBufCount++;
		return (void*)lpDSB;
	}
}


/* -------------------------------------------------------------------------------------
// sdyDrvDestroyBuffer
//------------------------------------------------------------------------------------- */
void sdyDrvDestroyBuffer(void* _lpDSB)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
	sdyDrvStop(lpDSB);
	lpDSB->Release();
	nBufCount--;
}


/* -------------------------------------------------------------------------------------
// sdyDrvGetWriteCursor
//------------------------------------------------------------------------------------- */
DWORD sdyDrvGetWriteCursor(void* _lpDSB)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return 0;
	
	DWORD dwCursor;
	lpDSB->GetCurrentPosition(NULL, &dwCursor);
	
	return dwCursor;
}


/* -------------------------------------------------------------------------------------
// sdyDrvGetReadCursor
//------------------------------------------------------------------------------------- */
DWORD sdyDrvGetReadCursor(void* _lpDSB)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return 0;
	
	DWORD dwCursor;
	lpDSB->GetCurrentPosition(&dwCursor, NULL);
	
	return dwCursor;
}


/* -------------------------------------------------------------------------------------
// sdyDrvLock
//------------------------------------------------------------------------------------- */
void sdyDrvLock(void* _lpDSB, sdyDword dwCursor, sdyDword dwWriteBytes, sdyByte** pPtr1, sdyDword* pdwSize1, sdyByte** pPtr2, sdyDword* pdwSize2)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
Retry:
	HRESULT hres = lpDSB->Lock(dwCursor, dwWriteBytes, (void**)pPtr1, pdwSize1, (void**)pPtr2, pdwSize2, 0);
	
	switch(hres)
	{
	case DS_OK:
		return;
	case DSERR_BUFFERLOST:
		lpDSB->Restore();
		goto Retry;
	}
	
	assert(0);
}


/* -------------------------------------------------------------------------------------
// sdyDrvUnlock
//------------------------------------------------------------------------------------- */
void sdyDrvUnlock(void* _lpDSB, LPVOID pPtr1, DWORD dwSize1, LPVOID pPtr2, DWORD dwSize2)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
	lpDSB->Unlock(pPtr1, dwSize1, pPtr2, dwSize2);
}


/* -------------------------------------------------------------------------------------
// sdyDrvSetFreq
//------------------------------------------------------------------------------------- */
void sdyDrvSetFreq(void* _lpDSB, DWORD dwFreq)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
	lpDSB->SetFrequency(dwFreq);
}


/* -------------------------------------------------------------------------------------
// sdyDrvSetVol
//------------------------------------------------------------------------------------- */
void sdyDrvSetVol(void* _lpDSB, LONG lVol)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
	lpDSB->SetVolume(lVol-10000);
}


/* -------------------------------------------------------------------------------------
// sdyDrvSetPan
//------------------------------------------------------------------------------------- */
void sdyDrvSetPan(void* _lpDSB, LONG lPan)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
	lpDSB->SetPan(lPan);
}


/* -------------------------------------------------------------------------------------
// sdyDrvPlay
//------------------------------------------------------------------------------------- */
BOOL sdyDrvPlay(void* _lpDSB, BOOL bLoop)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return FALSE;
	
Retry:
	HRESULT hres = lpDSB->Play(0, 0, bLoop ? DSBPLAY_LOOPING : 0);
	
	switch(hres)
	{
	case DS_OK:
		return TRUE;
	case DSERR_BUFFERLOST:
		lpDSB->Restore();
		goto Retry;
	}
	
	return FALSE;
}


/* -------------------------------------------------------------------------------------
// sdyDrvStop
//------------------------------------------------------------------------------------- */
void sdyDrvStop(void* _lpDSB)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
	lpDSB->Stop();
}


/* -------------------------------------------------------------------------------------
// sdyDrvIsPlaying
//------------------------------------------------------------------------------------- */
BOOL sdyDrvIsPlaying(void* _lpDSB)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return FALSE;
	
Retry:
	DWORD dwStatus;
	lpDSB->GetStatus(&dwStatus);
	
	if(dwStatus & DSBSTATUS_BUFFERLOST)
	{
		lpDSB->Restore();
		goto Retry;
	}
	
	if(dwStatus & DSBSTATUS_PLAYING)
		return TRUE;
	
	return FALSE;
}


/* -------------------------------------------------------------------------------------
// sdyDrvDuplicate
//------------------------------------------------------------------------------------- */
void* sdyDrvDuplicate(void* _lpDSB)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	HRESULT hres;
	LPDIRECTSOUNDBUFFER lpDSBDuplicate;
	
	assert(lpDSB);
	if(!lpDSB) return sdyNull;

	hres = lpDS->DuplicateSoundBuffer(lpDSB, &lpDSBDuplicate);
	
	if(hres == DS_OK)
	{
		nBufCount++;
		lpDSBDuplicate->SetCurrentPosition(0);
		return lpDSBDuplicate;
	}
	else
		return sdyNull;
}


/* -------------------------------------------------------------------------------------
// sdyDrvSetCurrentPos
//------------------------------------------------------------------------------------- */
void sdyDrvSetCurrentPos(void* _lpDSB, sdyDword dwPos)
{
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;
	
	assert(lpDSB);
	if(!lpDSB) return;
	
	lpDSB->SetCurrentPosition(dwPos);
}


/* -------------------------------------------------------------------------------------
// sdyDrvSet3DBuffer
//------------------------------------------------------------------------------------- */
void sdyDrvSet3DBuffer(void* _lpDSB, SDY_CTRL3D* pCtrl)
{
	LPDIRECTSOUND3DBUFFER lpDS3DB;
	DS3DBUFFER ds3dBuffer;
	ds3dBuffer.dwSize = sizeof(DS3DBUFFER);
	
	LPDIRECTSOUNDBUFFER lpDSB = (LPDIRECTSOUNDBUFFER)_lpDSB;	
	assert(lpDSB);
	if(!lpDSB)
		return;
	
	if FAILED(lpDSB->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&lpDS3DB))
		return;
	
	lpDS3DB->GetAllParameters(&ds3dBuffer);
	
	if(pCtrl->fMinDist <= 0.0f) pCtrl->fMinDist = 0.000001f;
	if(pCtrl->fMaxDist <= 0.0f) pCtrl->fMaxDist = 10000.0f;
	if(pCtrl->fMaxDist < pCtrl->fMinDist) pCtrl->fMaxDist = pCtrl->fMinDist;
	
	ds3dBuffer.dwSize = sizeof(DS3DBUFFER);
	ds3dBuffer.vPosition.x = pCtrl->vPos.x;
	ds3dBuffer.vPosition.y = pCtrl->vPos.y;
	ds3dBuffer.vPosition.z = pCtrl->vPos.z;
	ds3dBuffer.vVelocity.x = pCtrl->vVel.x;
	ds3dBuffer.vVelocity.y = pCtrl->vVel.y;
	ds3dBuffer.vVelocity.z = pCtrl->vVel.z;
	ds3dBuffer.dwInsideConeAngle = DS3D_DEFAULTCONEANGLE;
	ds3dBuffer.dwOutsideConeAngle = DS3D_DEFAULTCONEANGLE;
	ds3dBuffer.vConeOrientation.x = 0.0f;
	ds3dBuffer.vConeOrientation.y = 0.0f;
	ds3dBuffer.vConeOrientation.z = -1.0f;
	ds3dBuffer.lConeOutsideVolume = DS3D_DEFAULTCONEOUTSIDEVOLUME;
	ds3dBuffer.flMinDistance = pCtrl->fMinDist;
	ds3dBuffer.flMaxDistance = pCtrl->fMaxDist;
	ds3dBuffer.dwMode = DS3DMODE_NORMAL;
	
	lpDS3DB->SetAllParameters(&ds3dBuffer, DS3D_DEFERRED);
	
	lpDS3DB->Release();
}


/* -------------------------------------------------------------------------------------
// sdyDrvSetListener
//------------------------------------------------------------------------------------- */
void sdyDrvSetListener(const SDY_LISTENER* pListener)
{
	DS3DLISTENER ds3dListener;
	if(lpDS3DListener)
	{
		ds3dListener.dwSize = sizeof(DS3DLISTENER);
		ds3dListener.vPosition.x = pListener->vPos.x;
		ds3dListener.vPosition.y = pListener->vPos.y;
		ds3dListener.vPosition.z = pListener->vPos.z;
		ds3dListener.vVelocity.x = pListener->vVel.x;
		ds3dListener.vVelocity.y = pListener->vVel.y;
		ds3dListener.vVelocity.z = pListener->vVel.z;
		ds3dListener.vOrientFront.x = pListener->vFront.x;
		ds3dListener.vOrientFront.y = pListener->vFront.y;
		ds3dListener.vOrientFront.z = pListener->vFront.z;
		ds3dListener.vOrientTop.x = pListener->vTop.x;
		ds3dListener.vOrientTop.y = pListener->vTop.y;
		ds3dListener.vOrientTop.z = pListener->vTop.z;
		ds3dListener.flDistanceFactor = pListener->fDistMul;
		ds3dListener.flRolloffFactor = pListener->fRollMul;
		ds3dListener.flDopplerFactor = pListener->fDoppMul;
		
		lpDS3DListener->SetAllParameters(&ds3dListener, DS3D_DEFERRED);
	}
	
	if(lpDS3DListener)
	{
		lpDS3DListener->CommitDeferredSettings();
	}
}


/* -------------------------------------------------------------------------------------
// sdyDrvUpdate3DBuffers
//------------------------------------------------------------------------------------- */
void sdyDrvUpdate3DBuffers(void)
{
	if(lpDS3DListener)
	{
		lpDS3DListener->CommitDeferredSettings();
	}
}
