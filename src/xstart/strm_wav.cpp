/* Bastian von Halem	05.March.2002 - 15.May.2016 */
/* streams uncompressed WAV files with external io callbacks */

#include "soundy/sdy_def.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>

#define INLINE __inline


/* *******************************************************************
// SEEK_SET, SEEK_CUR, SEEK_END
//******************************************************************* */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif


/* *******************************************************************
// INTERNAL STRUCTURES
//******************************************************************* */
typedef struct {
	sdyChar 	szID[4];
	sdyDword	Size;
} RIFF_SUBCHUNK;

typedef struct {
	sdyChar 	szID[4];
	sdyDword	Size;
	sdyChar 	szType[4];
} RIFF_HEADER;

typedef struct {
	sdyPtr		hFile;
	SDY_IO		io;
	sdyDword	dwFlags;
	sdyDword	dwBytesLeft;
} PCM_INFO;


/* ----------------------------------------------------------------------------------------
// FOURCC_Cmp
//-----------------------------------------------------------------------------------------
// Compares two FOURCC strings (must not be terminated by 0)
//-------------------------------------------------------------------------------------- */
INLINE sdyBool PCM_FOURCC_Cmp(sdyString szFourCC1, sdyString szFourCC2) {
	if( szFourCC1[0]==szFourCC2[0] )
		if( szFourCC1[1]==szFourCC2[1] )
			if( szFourCC1[2]==szFourCC2[2])
				if( szFourCC1[3]==szFourCC2[3] )
					return sdyTrue;
	return sdyFalse;
}


/* ----------------------------------------------------------------------------------------
// PCM_RIFF_Goto
//-----------------------------------------------------------------------------------------
// Desc.:	Decents (seek's) into a subchunk
// Return:	returns the bytes of data in the chunk
//-------------------------------------------------------------------------------------- */
INLINE sdyDword PCM_RIFF_Goto(sdyPtr hFile, sdyString szSubChunk, SDY_IO* pIO) {
	RIFF_HEADER 	rh;
	RIFF_SUBCHUNK	rsc;
	sdyDword		readed;

	// reset file position
	pIO->seek( hFile, 0, SEEK_SET );

	// read header
	pIO->read( &rh, 12/*sizeof(RIFF_HEADER)*/, 1, hFile );

	// search
search: {
		readed = pIO->read(&rsc, 8/*sizeof(rsc)*/, 1, hFile);
		if(PCM_FOURCC_Cmp(&rsc.szID[0], szSubChunk))
			return rsc.Size;
		if(readed == 0)
			return 0;
		pIO->seek(hFile, rsc.Size, SEEK_CUR);
	}
	goto search;

	return 0;
}


/*-----------------------------------------------------------------------------------------
// PCM_Stream
//-----------------------------------------------------------------------------------------
// ...
//-------------------------------------------------------------------------------------- */
sdyDword SDYSTRMAPI PCM_Stream(
    void* _pInfo,
    sdyByte *lpbyBuffer,		/* already allocated buffer 				*/
    sdyDword dwBytes) {		/* bytes to read							*/
	sdyDword dwBytesRead;
	PCM_INFO* pInfo;

	pInfo = (PCM_INFO*)_pInfo;

	if(!pInfo)			return 0;
	if(!pInfo->hFile)	return 0;

	if(pInfo->dwBytesLeft < dwBytes)
		/* end of sound will be reached */
		dwBytes = pInfo->dwBytesLeft;

	/* read bytes from file */
	dwBytesRead = pInfo->io.read(lpbyBuffer, 1, dwBytes, pInfo->hFile);

	/* subtract readed bytes */
	pInfo->dwBytesLeft -= dwBytesRead;

	return dwBytesRead;
}


//-----------------------------------------------------------------------------------------
// PCM_Open
//-----------------------------------------------------------------------------------------
// ...
//-----------------------------------------------------------------------------------------
void* SDYSTRMAPI PCM_Open(
    const char* 		szFile,
    SDY_WFX*			pwfx,
    SDY_IO* 			pIO,
    sdyDword			dwFlags) {
	SDY_WFX wfx;
	PCM_INFO* pSWI;

	assert(szFile);
	assert(pIO);


	memset(&wfx, 0, sizeof(SDY_WFX));

	pSWI = (PCM_INFO*)malloc(sizeof(PCM_INFO));


	/* remember current callbacks and flags */
	pSWI->dwFlags = dwFlags;
	memcpy(&pSWI->io, pIO, sizeof(SDY_IO));


	/* open file */
	pSWI->hFile = pSWI->io.open(szFile, "rb");

	if(!pSWI->hFile) {
		free(pSWI);
		printf("Audio-File '%s' not found!\n", szFile);
		return sdyNull;
	}


	/* read WAVEFORMATEX, SSInfo is used here for temporary use! */
	wfx.cbSize = PCM_RIFF_Goto(pSWI->hFile, "fmt ", &pSWI->io);

	if(wfx.cbSize == 0) {
		free(pSWI);
		printf("Audio-File '%s' 'fmt' tag not found!\n", szFile);
		return sdyNull;
	}

	wfx.cbSize = 20 /*sizeof(SDY_WFX)*/; // TODO: CHECK THIS HOTFIX!

	//pSWI->io.read(&wfx, wfx.cbSize, 1, pSWI->hFile);
	pSWI->io.read(&wfx.wFormatTag, 2, 1, pSWI->hFile);
	pSWI->io.read(&wfx.nChannels, 2, 1, pSWI->hFile);
	pSWI->io.read(&wfx.nSamplesPerSec, 4, 1, pSWI->hFile);
	pSWI->io.read(&wfx.nAvgBytesPerSec, 4, 1, pSWI->hFile);
	pSWI->io.read(&wfx.nBlockAlign, 2, 1, pSWI->hFile);
	pSWI->io.read(&wfx.wBitsPerSample, 2, 1, pSWI->hFile);
	pSWI->io.read(&wfx.cbSize, 4, 1, pSWI->hFile);

	/* set SSInfo.dwByteSize to the correct size */
//	wfx.cbSize = pSWI->dwBytesLeft;

	/* copy info to output */
	if(pwfx) {
/*		printf("sizeof(SDY_WFX): %d\n", sizeof(SDY_WFX));
		printf("Format: %d\n", wfx.wFormatTag);
		printf("Channels: %d\n", wfx.nChannels);
		printf("SamplesPerSec: %d\n", wfx.nSamplesPerSec);
		printf("AvgBytesPerSec: %d\n", wfx.nAvgBytesPerSec);
		printf("BlockAlign: %d\n", wfx.nBlockAlign);
		printf("BitsPerSample: %d\n", wfx.wBitsPerSample);
		printf("cbSize: %d\n", wfx.cbSize);*/
		memcpy(pwfx, &wfx, sizeof(SDY_WFX));
		pwfx->cbSize = 0;
	}

	/* goto data chunk */
	pSWI->dwBytesLeft = PCM_RIFF_Goto(pSWI->hFile, "data", &pSWI->io);
	
	return pSWI;
}


//-----------------------------------------------------------------------------------------
// PCM_Close
//-----------------------------------------------------------------------------------------
// ...
//-----------------------------------------------------------------------------------------
sdyBool SDYSTRMAPI PCM_Close(void* _pInfo) {
	PCM_INFO* pInfo;

	pInfo = (PCM_INFO*)_pInfo;

	assert(pInfo);
	if(!pInfo) return sdyFalse;

	if (pInfo->hFile) {
		//fix
		fflush((FILE*)pInfo->hFile);
		pInfo->io.close(pInfo->hFile);
		pInfo->hFile = 0;
	}

	free(pInfo);

	return sdyTrue;
}
