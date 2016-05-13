/* Manages the creation, destruction, duplication and streaming (file->memory) of sound objects */ 

#ifndef _SDY_MANAGE_H_
#define _SDY_MANAGE_H_

#include "sdy_def.h"


void		SDYAPI	sdyManInitialize	(sdyWord wBufSec);
void		SDYAPI	sdyManShutdown		(void);
SDY_SOUND*	SDYAPI	sdyManOpenSound 	(const sdyString szFile, sdyBool bStatic, sdyBool b3DBuffer);
sdyDword	SDYAPI	sdyManStream		(SDY_SOUND* pSound, sdyDword dwPos, sdyDword dwLen, sdyBool bWrap);
void		SDYAPI	sdyManCloseSound	(SDY_SOUND* pSound);
/* sdyManSilence - Called by _sdyALStream to fill the rest of a streaming buffer with silence */
void		SDYAPI	sdyManSilence		(SDY_SOUND* pSound, sdyDword dwPos, sdyDword dwLen);
sdyBool 	SDYAPI	sdyManRewindSound	(SDY_SOUND* pSound);


#endif
