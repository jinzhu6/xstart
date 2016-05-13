/* accesses loaders and manages streaming */ 

#ifndef _SDY_LOADER_H_
#define _SDY_LOADER_H_


#include "sdy_def.h"

sdyDword	SDYAPI sdyRegLoader 		(SDY_CB_OPEN open, SDY_CB_STREAM stream, SDY_CB_CLOSE close);
void		SDYAPI sdyUnregAllLoaders	(void);
SDY_STREAM* SDYAPI sdyStreamOpen		(const sdyString szFile, SDY_WFX* wfx);
/* sdyStreamRewind - Rewind the stream by closing->opening (for looping or size-estimating) */
sdyBool 	SDYAPI sdyStreamRewind		(SDY_STREAM* pStream);
sdyBool 	SDYAPI sdyStreamClose		(SDY_STREAM* pStream);
sdyDword	SDYAPI sdyStreamStream		(SDY_STREAM* pStream, sdyByte* pAdress, sdyDword dwByteSize, sdyBool bWrap);


#endif
