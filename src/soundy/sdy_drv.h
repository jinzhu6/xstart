#ifndef _SDY_DRV_H_
#define _SDY_DRV_H_


#include "sdy_def.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	
	sdyBool 	sdyDrvInit				(long _hWnd, SDY_SETTINGS* pSettings);
	void		sdyDrvRelease			(void);
	void*		sdyDrvCreateBuffer		(SDY_WFX* pWFX, sdyDword dwBufBytes, sdyBool b3DBuffer);
	void		sdyDrvDestroyBuffer 	(void* lpDSB);
	sdyDword	sdyDrvGetWriteCursor	(void* lpDSB);
	sdyDword	sdyDrvGetReadCursor 	(void* lpDSB);
	void		sdyDrvLock				(void* lpDSB, sdyDword dwCursor, sdyDword dwWriteBytes, sdyByte** ppPtr1, sdyDword* pdwSize1, sdyByte** ppPtr2, sdyDword* pdwSize2);
	void		sdyDrvUnlock			(void* lpDSB, sdyPtr pPtr1, sdyDword dwSize1, sdyPtr pPtr2, sdyDword dwSize2);
	void		sdyDrvSetFreq			(void* lpDSB, sdyDword dwFreq);
	void		sdyDrvSetVol			(void* lpDSB, sdyLong lVol);
	void		sdyDrvSetPan			(void* lpDSB, sdyLong lPan);
	sdyBool 	sdyDrvPlay				(void* lpDSB, sdyBool bLoop);
	void		sdyDrvStop				(void* lpDSB);
	sdyBool 	sdyDrvIsPlaying 		(void* lpDSB);
	void*		sdyDrvDuplicate 		(void* lpDSB);
	void		sdyDrvSetCurrentPos 	(void* lpDSB, sdyDword dwPos);
	void		sdyDrvSet3DBuffer		(void* _lpDSB, SDY_CTRL3D* pCtrl);
	void		sdyDrvSetListener		(const SDY_LISTENER* pListener);
	void		sdyDrvUpdate3DBuffers	(void);
	
	
#ifdef __cplusplus
}
#endif


#endif
