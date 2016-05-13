#ifndef _SOUNDY_H_
#define _SOUNDY_H_

#include "sdy_def.h"


#ifdef __cplusplus
extern "C" {
#endif

SDY_INTERFACE	sdyBool 	SDYAPI		sdyInit 		(long _hWnd, SDY_SETTINGS* pSettings);
SDY_INTERFACE	void		SDYAPI		sdySetIO		(SDY_IO* io);
SDY_INTERFACE	void		SDYAPI		sdyRelease		(void);
SDY_INTERFACE	SDY_SOUND* 	SDYAPI		sdyPlay 		(SDY_SOUND* pSound, sdyBool bLoop);
SDY_INTERFACE	sdyBool 	SDYAPI		sdyStop 		(SDY_SOUND* pSound);
SDY_INTERFACE	sdyBool 	SDYAPI		sdyUpdate		(void);
SDY_INTERFACE	SDY_SOUND*	SDYAPI		sdyOpen 		(const sdyString szFile, sdyBool bStatic, sdyBool b3dSound);
SDY_INTERFACE	void		SDYAPI		sdyClose		(SDY_SOUND* pSound);
SDY_INTERFACE	sdyBool 	SDYAPI		sdyIsPlaying	(SDY_SOUND* pSound);
SDY_INTERFACE	void		SDYAPI		sdySetPriority	(SDY_SOUND* pSound, sdyInt nPriority);
SDY_INTERFACE	void		SDYAPI		sdySetMaxCopies (SDY_SOUND* pSound, sdyDword dwMaxDupli);

SDY_INTERFACE	void		SDYAPI		sdySetVol		(SDY_SOUND* pSound, sdyDword dwVol);
SDY_INTERFACE	void		SDYAPI		sdySetPan		(SDY_SOUND* pSound, long nPan);
SDY_INTERFACE	void		SDYAPI		sdySetFreq		(SDY_SOUND* pSound, sdyDword dwFreq);

SDY_INTERFACE	void		SDYAPI		sdy3dSetPos 	(SDY_SOUND* pSound, sdyFloat x, sdyFloat y, sdyFloat z);
SDY_INTERFACE	void		SDYAPI		sdy3dSetVel 	(SDY_SOUND* pSound, sdyFloat x, sdyFloat y, sdyFloat z);
SDY_INTERFACE	void		SDYAPI		sdy3dSetDist	(SDY_SOUND* pSound, sdyFloat min, sdyFloat max);

SDY_INTERFACE	void		SDYAPI		sdy3dListener	(const SDY_LISTENER* pListener);


#ifdef __cplusplus
}
#endif


#endif
