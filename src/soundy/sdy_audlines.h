#ifndef _SDY_AUDLINES_H_
#define _SDY_AUDLINES_H_

#include "sdy_def.h"


void		SDYAPI	sdyALInitialize (SDY_SETTINGS* pSettings);
void		SDYAPI	sdyALShutdown	(void);
SDY_SOUND*	SDYAPI	sdyALPlay		(SDY_SOUND* pSound, sdyBool bLoop);
void		SDYAPI	sdyALStop		(SDY_SOUND* pSound);
sdyBool		SDYAPI	sdyALIsPlaying	(SDY_SOUND* pSound);
void		SDYAPI	sdyALUpdate 	(void);


#endif
