#ifndef _STRM_OGG_H_
#define _STRM_OGG_H_


#include "sdy_def.h"

sdyDword	SDYSTRMAPI	OGG_Stream		(void* pInfo, sdyByte *lpbyBuffer, sdyDword dwBytes);
void*		SDYSTRMAPI	OGG_Open		(const char* szFile, SDY_WFX* pWfx, SDY_IO* pIO, sdyDword dwFlags);
sdyBool 	SDYSTRMAPI	OGG_Close		(void* pInfo);


#endif
