#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "sdy_drv.h"
#include "sdy_loader.h"
#include "sdy_audlines.h"
#include "sdy_cont.h"

static sdyWord g_wBufSec;	// Milliseconds of sound buffer size
static CONT* g_cSounds;		// Container of all SDY_SOUNDs


/* -------------------------------------------------------------------------------------
// sdyManSilence - Generates silence bytes in a buffer
//------------------------------------------------------------------------------------- */
void SDYAPI sdyManSilence(SDY_SOUND* pSound, sdyDword dwPos, sdyDword dwLen)
{
	sdyByte *pPtr1, *pPtr2;
	sdyDword dwSize1, dwSize2;
	
	/* check param */ 
	assert(pSound);
	if(!pSound) return;
	
	/* lock buffer */ 
	sdyDrvLock(pSound->pBuffer, dwPos, dwLen, &pPtr1, &dwSize1, &pPtr2, &dwSize2);
	
	/* write silence */ 
	if(pPtr1)
		memset(pPtr1, pSound->bySilence, dwSize1);
	if(pPtr2)
		memset(pPtr2, pSound->bySilence, dwSize2);
	
	/* unlock buffer */ 
	sdyDrvUnlock(pSound->pBuffer, pPtr1, dwSize1, pPtr2, dwSize2);
	
	return;
}

/* -------------------------------------------------------------------------------------
// sdyManStream
//------------------------------------------------------------------------------------- */
sdyDword SDYAPI sdyManStream(SDY_SOUND* pSound, sdyDword dwPos, sdyDword dwLen, sdyBool bWrap)
{
	sdyByte *pPtr1, *pPtr2;
	sdyDword dwSize1, dwSize2;
	sdyDword dwStreamed;
	
	/* check param */ 
	assert(pSound);
	if(!pSound) return 0;
	
	/* lock buffer */ 
	sdyDrvLock(pSound->pBuffer, dwPos, dwLen, &pPtr1, &dwSize1, &pPtr2, &dwSize2);
	
	/* stream into buffer */ 
	dwStreamed = 0;
	if(pPtr1)
		dwStreamed += sdyStreamStream(pSound->pStream, pPtr1, dwSize1, bWrap);
	if(pPtr2)
		dwStreamed += sdyStreamStream(pSound->pStream, pPtr2, dwSize2, bWrap);
	
	/* unlock buffer */ 
	sdyDrvUnlock(pSound->pBuffer, pPtr1, dwSize1, pPtr2, dwSize2);
	
	return dwStreamed;
}

/* -------------------------------------------------------------------------------------
// _sdyDuplicate
//------------------------------------------------------------------------------------- */
SDY_SOUND* SDYAPI _sdyDuplicate(SDY_SOUND* pSound)
{
	SDY_SOUND*	pDuplicate;
	void*		pBuffer;
	
	/* check param */ 
	assert(pSound);
	if(!pSound) 
		return sdyNull;
	
	/* Only static sounds can be duplicated! */ 
	if(!pSound->bStatic)
		return sdyNull;
	
	/* Try to make a duplicate of the buffer */ 
	pBuffer = sdyDrvDuplicate(pSound->pBuffer);
	if(!pBuffer)
		return sdyNull;
	
	/* Create new sound object */ 
	pDuplicate = (SDY_SOUND*)malloc(sizeof(SDY_SOUND));
	memset(pDuplicate, 0, sizeof(SDY_SOUND));
	pDuplicate->pBuffer 	= pBuffer;
	pDuplicate->pStream 	= sdyNull;
	pDuplicate->dwBufSize	= pSound->dwBufSize;
	pDuplicate->dwSndSize	= pSound->dwSndSize;
	pDuplicate->bStatic 	= pSound->bStatic;
	pDuplicate->bySilence	= pSound->bySilence;
	pDuplicate->b3DSound	= pSound->b3DSound;
	strcpy(pDuplicate->szFile, pSound->szFile);
	
	/* push to managed sounds */ 
	cpush(g_cSounds, pDuplicate);
	
	return pDuplicate;
}

/* -------------------------------------------------------------------------------------
// _sdyFindOpened
//--------------------------------------------------------------------------------------
// Tries to find a opened sound by its file-name
//------------------------------------------------------------------------------------- */
SDY_SOUND* SDYAPI _sdyFindOpened(const sdyString szFile)
{
	SDY_SOUND *pSound;
	CONT_ITEM* i;
	
	/* check param */ 
	assert(szFile);
	if(!szFile)
		return sdyNull;
	
	/* goto first element in list */ 
	i = cfirst(g_cSounds);
	
	/* look through all elements */ 
	while(i)
	{
		/* get data */ 
		pSound = (SDY_SOUND*)cidata(i);
		
		/* compare file-names */ 
		if(strcmp(szFile, pSound->szFile) == 0)
			return pSound; /* return found sound */ 
		
		/* goto next element */ 
		i = cinext(i);
	}
	
	/* sound not opened */ 
	return sdyNull;
}

/* -------------------------------------------------------------------------------------
// sdyManCloseSound
//------------------------------------------------------------------------------------- */
void SDYAPI sdyManCloseSound(SDY_SOUND* pSound)
{
	/* check that manger is initialized */ 
	assert(g_cSounds);
	if(!g_cSounds)
		return;
	
	/* check param */ 
	assert(pSound);
	if(!pSound) 
		return;
	
	/* the sound should not be playing in order to be thread-safe */ 
	if(sdyALIsPlaying(pSound))
	{
		/* NOTE: This is rather a fix than good design */ 
		/* let AL stop the sound and re-call this function again */ 
		//printf("(sdy_manage.c) Warning: FIX USED!\n");
		sdyALStop(pSound);
		pSound->bAutoRemove = sdyTrue;
		return;
		
		//sdyALStop(pSound);
		//sdyManCloseSound(pSound);
		//return;
	}
	
	/* search the sound in the list, */ 
	if(!pSound->bDuplicate)
	{
		if(!csearch(g_cSounds, pSound))
		{
			assert(0);
			return;
		}
	}
	
	/* close stream when its open */ 
	if(pSound->pStream)
	{
		sdyStreamClose(pSound->pStream);
		pSound->pStream = sdyNull;
	}
	
	/* destroy buffer */ 
	if(pSound->pBuffer)
	{
		sdyDrvDestroyBuffer(pSound->pBuffer);
		pSound->pBuffer = sdyNull;
	}
	
	/* remove the sound from the list */ 
	crndpop(g_cSounds, pSound);
	
	/* free object */ 
	free(pSound);
}

/* -------------------------------------------------------------------------------------
// sdyManOpenSound
//------------------------------------------------------------------------------------- */
SDY_SOUND* SDYAPI sdyManOpenSound(const sdyString szFile, sdyBool bStatic, sdyBool b3DBuffer)
{
	sdyDword	dwBufSize;
	sdyDword	dwStreamed;
	void*		pBuffer;
	SDY_SOUND*	pSound;
	SDY_STREAM* pStream;
	SDY_WFX 	wfx;
	
	sdyDword	dwTmpRead;
	sdyByte*	pTmpBuf;
	
	/* check that manger is initialized */ 
	assert(g_cSounds);
	if(!g_cSounds)
		return sdyNull;
	
	/* check param */ 
	assert(szFile);
	if(!szFile)
		return sdyNull;
	if(szFile[0] == 0)
		return sdyNull;
	
	/* Try to find and make a duplicate */ 
	pSound = _sdyFindOpened(szFile);
	if(pSound)
		pSound = _sdyDuplicate(pSound);
	if(pSound)
		return pSound;
	
	/* open stream */ 
	pStream = sdyStreamOpen(szFile, &wfx);
	if(!pStream)
		return sdyNull;
	
	/* some loaders do not provide a size, use magic number */ 
	if(wfx.cbSize == 0)
	{
		wfx.cbSize = 0xFFFFFFFF;
	}
	
	/* if a static buffer for a sound of unknown size is wanted */ 
	/* try to get the size by streaming (worst case!)			*/ 
	if(bStatic && wfx.cbSize == 0xFFFFFFFF)
	{
		wfx.cbSize = 0;
		pTmpBuf = (unsigned char*)malloc(1024);
		do {
			dwTmpRead = sdyStreamStream(pStream, pTmpBuf, 1024, sdyFalse);
			wfx.cbSize += dwTmpRead;
		} while(dwTmpRead);
		free(pTmpBuf);
		sdyStreamRewind(pStream);
	}
	
	/* if the stream is smaller than the default buffer size,	*/ 
	/* make it a static buffer									*/ 
	/* FIX: Changed g_wBufSec to g_wBufSec*4					*/ 
	if( wfx.cbSize != 0xFFFFFFFF && 
		wfx.cbSize <= (g_wBufSec*4) * wfx.nAvgBytesPerSec / 1000)
		bStatic = sdyTrue;
	
	/* set buffer size */ 
	if(bStatic)
		dwBufSize = wfx.cbSize;
	else
		dwBufSize = g_wBufSec * wfx.nAvgBytesPerSec / 1000;
	
	/* create driver buffer */ 
	pBuffer = sdyDrvCreateBuffer(&wfx, dwBufSize, b3DBuffer);
	if(!pBuffer)
	{
		sdyStreamClose(pStream);
		return sdyNull;
	}
	
	/* Create new sound object and setup variables */ 
	pSound = (SDY_SOUND*)malloc(sizeof(SDY_SOUND));
	memset(pSound, 0, sizeof(SDY_SOUND));
	pSound->pBuffer 	= pBuffer;
	pSound->pStream 	= pStream;
	pSound->bStatic 	= bStatic;
	pSound->b3DSound	= b3DBuffer;
	pSound->dwBufSize	= dwBufSize;
	pSound->dwSndSize	= wfx.cbSize;
	pSound->bySilence	= (wfx.wBitsPerSample == 8) ? 128 : 0;
	pSound->dwMaxDuplicates = DEF_MAXDUPLI;
	strcpy(pSound->szFile, szFile);
	
	/* fill buffer */ 
	dwStreamed = sdyManStream(pSound, 0, pSound->dwBufSize, sdyFalse);
	pSound->dwStreamed = pSound->dwBufSize;
	
	/* close stream on static buffers */ 
	if(bStatic)
	{
		sdyStreamClose(pSound->pStream);
		pSound->pStream = sdyNull;
	}
	
	/* push to managed sounds */ 
	cpush(g_cSounds, pSound);
	
	/* */ 
	if(dwStreamed != pSound->dwBufSize)
	{
		/* stream is smaller than expected */ 
		/* force it to be a static sound.  */ 
		sdyManCloseSound(pSound);
		return sdyManOpenSound(szFile, sdyTrue, b3DBuffer);
	}
	
	return pSound;
}

/* -------------------------------------------------------------------------------------
// sdyManRewindSound
//------------------------------------------------------------------------------------- */
sdyBool SDYAPI sdyManRewindSound(SDY_SOUND* pSound)
{
	/* ensure that manager is initialized */ 
	assert(g_cSounds);
	if(!g_cSounds)
		return sdyFalse;
	
	/* check param */ 
	assert(pSound);
	if(!pSound) 
		return sdyFalse;
	
	/* search the sound in the list */ 
	if(!csearch(g_cSounds, pSound))
		return sdyFalse;
	
	/* reopen stream when its open */ 
	if(pSound->pStream)
		sdyStreamRewind(pSound->pStream);
	
	/* rewind buffer pos */ 
	sdyDrvSetCurrentPos(pSound->pBuffer, 0);
	
	/* update vars */ 
	pSound->dwLastUpdate = 0;
	pSound->dwStrmPlayed = 0;
	
	/* prepare next playback by filling the stream buffer */ 
	if(!pSound->bStatic) /* not for static sounds */ 
		sdyManStream(pSound, 0, pSound->dwBufSize, sdyFalse);
	
	return sdyTrue;
}

/* -------------------------------------------------------------------------------------
// sdyManInitialize
//------------------------------------------------------------------------------------- */
void SDYAPI sdyManInitialize(sdyWord wBufSec)
{
	if(!g_cSounds)
		g_cSounds = cnew();
	g_wBufSec = wBufSec;
}

/* -------------------------------------------------------------------------------------
// sdyManShutdown
//-------------------------------------------------------------------------------------
// Shuts down the manager by closing all opened sounds and releasing internal memory
//------------------------------------------------------------------------------------- */
void SDYAPI sdyManShutdown()
{
	CONT_ITEM* i;
	
	assert(g_cSounds);
	
	if(!g_cSounds)
		return;
	
	while(i = cfirst(g_cSounds))
		sdyManCloseSound((SDY_SOUND*)cidata(i));
	
	cdel(g_cSounds);
	g_cSounds = sdyNull;
}

