#ifndef _STRM_WAV_H_
#define _STRM_WAV_H_


#include "sdy_def.h"

sdyDword	SDYSTRMAPI	PCM_Stream		(void* pInfo, sdyByte *lpbyBuffer, sdyDword dwBytes);
void*		SDYSTRMAPI	PCM_Open		(const char* szFile, SDY_WFX* pwfx, SDY_IO* pIO, sdyDword dwFlags);
sdyBool 	SDYSTRMAPI	PCM_Close		(void* pInfo);


#endif
