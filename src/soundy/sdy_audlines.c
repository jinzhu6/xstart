#include "sdy_audlines.h"
#include "sdy_drv.h"
#include "sdy_manage.h"
#include "sdy_cont.h"

static CONT* g_cAudLines;	/* playing audio lines */ 
sdyWord g_wMaxAudLines;


/* -------------------------------------------------------------------------------------
// _sdyALDuplicate
//-------------------------------------------------------------------------------------
// When playing a sound more than once at time a duplicate is needed
//------------------------------------------------------------------------------------- */
SDY_SOUND* _sdyALDuplicate(SDY_SOUND* pSound)
{
	SDY_SOUND* pDuplicate;
	
	assert(pSound);
	if(!pSound)
		return sdyNull;
	
	if(pSound->dwNumDuplicates >= pSound->dwMaxDuplicates)
		return sdyNull;
	
	pSound->dwNumDuplicates++;
	
	if(pSound->bStatic)
	{
		/* duplicate static buffer */ 
		/* a copy is made from the static buffer memory directly */ 
		pDuplicate = (SDY_SOUND*)malloc(sizeof(SDY_SOUND));
		memcpy(pDuplicate, pSound, sizeof(SDY_SOUND));
		pDuplicate->bDuplicate		= sdyTrue;
		pDuplicate->pBuffer 		= sdyDrvDuplicate(pSound->pBuffer);
	}
	else
	{
		/* duplicate streaming buffer */ 
		/* another stream of the same file is opened as another stream */ 
		pDuplicate = sdyManOpenSound(pSound->szFile, sdyFalse, pSound->b3DSound);
		pDuplicate->bDuplicate		= sdyTrue;		
	}
	
	pDuplicate->pParent = pSound;
	
	return pDuplicate;
}

/* -------------------------------------------------------------------------------------
// _sdyALDestroyDuplicate
//------------------------------------------------------------------------------------- */
sdyBool _sdyALDestroyDuplicate(SDY_SOUND* pSound)
{
	assert(pSound);
	if(!pSound)
		return sdyFalse;
	
	/* destroy internal duplicates */ 
	if(pSound->bDuplicate)
	{
		assert(pSound->pParent);
		if(pSound->pParent)
		{
			((SDY_SOUND*)(pSound->pParent))->dwNumDuplicates--;
		}
		
		if(pSound->bStatic)
		{
			sdyDrvDestroyBuffer(pSound->pBuffer);
			pSound->pBuffer = sdyNull;
			free(pSound);
			return sdyTrue;
		}
		else
		{
			sdyManCloseSound(pSound);
			return sdyTrue;
		}
	}
	
	return sdyFalse;
}




/* -------------------------------------------------------------------------------------
// _sdyALPop
//------------------------------------------------------------------------------------- */
sdyBool SDYAPI _sdyALPop(SDY_SOUND* pSoundPop)
{
	assert(pSoundPop);
	assert(g_cAudLines);
	
	/* do not try to pop a non playing sound */ 
	assert(csearch(g_cAudLines, pSoundPop));
	if(!csearch(g_cAudLines, pSoundPop))
		return sdyFalse;
	
	/* remove from play-list */ 
	crndpop(g_cAudLines, pSoundPop);
	
	/* TODO: The sound should be already stopped here! */ 
	/* stop the sound by the driver */ 
	if(sdyDrvIsPlaying(pSoundPop->pBuffer))
		sdyDrvStop(pSoundPop->pBuffer);
	
	/* rewind sound for the next playback */ 
	sdyManRewindSound(pSoundPop);
	
	/* destroy internal duplicates */ 
	if(pSoundPop->bDuplicate)
	{
		_sdyALDestroyDuplicate(pSoundPop);
		return sdyTrue;
	}
	
	/* close "AutoRemove" sounds */ 
	if(pSoundPop->bAutoRemove)
	{
		sdyManCloseSound(pSoundPop);
		return sdyTrue;
	}
	
	return sdyTrue;
}

/* -------------------------------------------------------------------------------------
// _sdyALPushMakeFree - kick a sound to make room for a new one
//------------------------------------------------------------------------------------- */
sdyBool SDYAPI _sdyALPushMakeFree(SDY_SOUND* pSoundPush)
{
	CONT_ITEM* i;
	SDY_SOUND* pSound;
	
	if(ccount(g_cAudLines) >= g_wMaxAudLines)
	{
		/* find a sound to kick out */ 
		i = cfirst(g_cAudLines);
		while(i)
		{
			pSound = (SDY_SOUND*)cidata(i);
			i = cinext(i);
			if(pSound->nPriority <= pSoundPush->nPriority)
			{
				//_sdyALPop(pSound);
				sdyALStop(pSound);
				return sdyTrue;
			}
		}
		return sdyFalse; /* not playable at the moment */ 
	}
	
	return sdyTrue;
}

/* -------------------------------------------------------------------------------------
// _sdyALPush
//-------------------------------------------------------------------------------------
// Ensures that only up to "g_wMaxAudLines" sounds are playing
//------------------------------------------------------------------------------------- */
SDY_SOUND* SDYAPI _sdyALPush(SDY_SOUND* pSoundPush, sdyBool bLoop)
{
	assert(pSoundPush);
	assert(g_cAudLines);
	if(!pSoundPush || !g_cAudLines)
		return sdyFalse;
	
	/* Make a duplicate if the sound is already playing */ 
	if(sdyALIsPlaying(pSoundPush))
		pSoundPush = _sdyALDuplicate(pSoundPush);
	
	if(pSoundPush)
	{
		if(!pSoundPush->bStatic)
		{
			pSoundPush->dwStrmPlayed = 0;
			pSoundPush->bLoopStream = bLoop;
		}
		
		if(_sdyALPushMakeFree(pSoundPush))
		{
			if(sdyDrvPlay(pSoundPush->pBuffer, bLoop | !pSoundPush->bStatic))
			{
				cpush(g_cAudLines, pSoundPush);
				return pSoundPush;
			}
		}
	}
	
	return sdyNull;
}

/* -------------------------------------------------------------------------------------
// _sdyALIsPlaying
//-------------------------------------------------------------------------------------
// Returns "sdyTrue" if a sound is playing "sdyFalse" otherwise.
//------------------------------------------------------------------------------------- */
sdyBool SDYAPI sdyALIsPlaying(SDY_SOUND* pSound)
{
	assert(pSound);
	assert(g_cAudLines);
	
	if(!pSound || !g_cAudLines)
		return sdyFalse;
	
	if(csearch(g_cAudLines, pSound))
		return sdyTrue;
	
	return sdyDrvIsPlaying(pSound->pBuffer);
}

/* -------------------------------------------------------------------------------------
// sdyALPlay
//------------------------------------------------------------------------------------- */
SDY_SOUND* SDYAPI sdyALPlay(SDY_SOUND* pSound, sdyBool bLoop)
{
	return _sdyALPush(pSound, bLoop);
}

/* -------------------------------------------------------------------------------------
// sdyALStop
//------------------------------------------------------------------------------------- */
void SDYAPI sdyALStop(SDY_SOUND* pSound)
{
	assert(g_cAudLines);
	assert(pSound);
	if(!g_cAudLines || !pSound)
		return;
	
	sdyDrvStop(pSound->pBuffer);
}

/* -------------------------------------------------------------------------------------
// sdyALInitialize
//------------------------------------------------------------------------------------- */
void SDYAPI sdyALInitialize(SDY_SETTINGS* pSettings)
{
	assert(!g_cAudLines);
	
	g_wMaxAudLines = pSettings->wAudLines;
	
	if(!g_cAudLines)
		g_cAudLines = cnew();
	
	sdyManInitialize(pSettings->wBufSec);
}

/* -------------------------------------------------------------------------------------
// sdyALShutdown
//------------------------------------------------------------------------------------- */
void SDYAPI sdyALShutdown(void)
{
	SDY_SOUND* pSound;
	CONT_ITEM* i;
	
	assert(g_cAudLines);
	
	if(g_cAudLines)
	{
		i = cfirst(g_cAudLines);
		while(i)
		{
			pSound = (SDY_SOUND*)cidata(i);
			i = cinext(i);
			_sdyALPop(pSound);
		}
		cdel(g_cAudLines);
		g_cAudLines = 0;
	}
	
	sdyManShutdown();
}

/* -------------------------------------------------------------------------------------
// _sdyALStream
//------------------------------------------------------------------------------------- */
void SDYAPI _sdyALStream(SDY_SOUND* pSound)
{
	sdyDword dwLastCur;
	sdyDword dwReadCur;
	sdyDword dwToStream;
	sdyDword dwStreamed;
	
	assert(pSound);
	assert(g_cAudLines);
	if(!pSound || !g_cAudLines) return;
	
	/* Get current read and last update cursor */ 
	dwReadCur = sdyDrvGetReadCursor(pSound->pBuffer);
	dwLastCur = pSound->dwLastUpdate;
	if(dwReadCur == dwLastCur)
		return;
	
	/* Calculate number of bytes to stream (trivial version) */ 
	if(dwReadCur > dwLastCur)
		dwToStream = dwReadCur - dwLastCur;
	else if(dwReadCur < dwLastCur)
		dwToStream = pSound->dwBufSize - (dwLastCur - dwReadCur);
	
	/* advance streaming cursor (of total played samples) */ 
	pSound->dwStrmPlayed += dwToStream;
	
	/* Eventually stop non-looping streams */ 
	if(pSound->dwSndSize)
	{
		if(pSound->dwStrmPlayed >= pSound->dwSndSize)
		{
			pSound->dwStrmPlayed -= pSound->dwSndSize;
			/*printf("Streamed: %d\n", pSound->dwStreamed);*/
			if(!pSound->bLoopStream)
			{
				sdyDrvStop(pSound->pBuffer);
				return;
			}
		}
	}
	
	/* Stream */ 
	dwStreamed = sdyManStream(pSound, dwLastCur, dwToStream, pSound->bLoopStream);
	pSound->dwStreamed += dwStreamed;
	
	/* Fill remaining bytes with silence */ 
	if(dwStreamed < dwToStream)
	{
		sdyManSilence(pSound, (dwLastCur + dwStreamed) % pSound->dwBufSize, dwToStream - dwStreamed);
		
		/* if the sound size was unknown - we know it now! */ 
		if(pSound->dwSndSize == 0xFFFFFFFF)
		{
			pSound->dwSndSize = pSound->dwStreamed;
		}
	}
	
	/* Set "LastUpdate" cursor to read cursor */ 
	pSound->dwLastUpdate = dwReadCur;
}

/* -------------------------------------------------------------------------------------
// sdyALUpdate
//------------------------------------------------------------------------------------- */
void SDYAPI sdyALUpdate(void)
{
	SDY_SOUND* pSound;
	CONT_ITEM* i;
	
	/* check */ 
	assert(g_cAudLines);
	if(!g_cAudLines) return;
	
	/* Update playing sound */ 
	i = cfirst(g_cAudLines);
	while(i)
	{
		pSound = (SDY_SOUND*)cidata(i);
		i = cinext(i);
		
		if(!sdyDrvIsPlaying(pSound->pBuffer))
		{
			_sdyALPop(pSound);
			continue;
		}
		
		if(!pSound->bStatic)
			_sdyALStream(pSound);
	}
}
