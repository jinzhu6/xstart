/* Bastian von Halem	30.March.2005 */
/* streams OGG files */

#include "sdy_def.h"

#include <malloc.h>
#include <memory.h>
#include <assert.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

/*#ifdef _MSC_VER

	#ifdef _MT
		#pragma comment(lib, "ogg_static_MT.lib")
		#pragma comment(lib, "vorbis_static_MT.lib")
		#pragma comment(lib, "vorbisfile_static_MT.lib")
	#else
		#pragma comment(lib, "ogg_static_ST.lib")
		#pragma comment(lib, "vorbis_static_ST.lib")
		#pragma comment(lib, "vorbisfile_static_ST.lib")
	#endif

#endif*/
/*#pragma comment(lib, "ogg_static_mt.lib")
#pragma comment(lib, "vorbis_static_mt.lib")
#pragma comment(lib, "vorbisfile_static_mt.lib")*/

#define INLINE __inline

typedef int(*ogg_seek)(void*,__int64,int);

typedef struct OGG_INFO {
	void*		vf; 		/* (OggVorbis_File*) */
	sdyDword	dwFlags;
	int 		section;

} OGG_INFO;

/* ----------------------------------------------------------------------------------------
// OGG_Stream
//----------------------------------------------------------------------------------------- */
sdyDword SDYSTRMAPI OGG_Stream (
    OGG_INFO* hInfo,	// file/stream information, must
    // contain a pointer to an open file
    sdyByte* lpbyBuffer,	// already allocated buffer
    sdyDword dwBytes		// bytes to read
) {
	sdyDword dwBytesRead = 0;
	sdyDword dwOneRead = 0;

	if(!hInfo) { return 0; }

	while(dwBytesRead < dwBytes) {
		dwOneRead = ov_read(
		                hInfo->vf,
		                (char*)&lpbyBuffer[dwBytesRead],
		                dwBytes - dwBytesRead, 0, 2, 1,
		                &hInfo->section);

		if(!dwOneRead) { break; }
		dwBytesRead += dwOneRead;
	}

	return dwBytesRead;
}


/* ----------------------------------------------------------------------------------------
// _OGG_Seek - Disables seeking in OGG
//----------------------------------------------------------------------------------------- */
int _OGG_Seek(void* datasource, ogg_int64_t offset, int whence) {
	return -1;
}


//-----------------------------------------------------------------------------------------
// OGG_Open
//-----------------------------------------------------------------------------------------
OGG_INFO* SDYSTRMAPI OGG_Open(
    const char* szFile,
    SDY_WFX* pWfx,
    SDY_IO* pSCallbacks,
    sdyDword dwFlags) {
	OGG_INFO* pSWI;
	ov_callbacks ovcallbacks;
	vorbis_info* vinfo;
	void* hFile;

	// set callbacks
	ovcallbacks.read_func = pSCallbacks->read;
	//ovcallbacks.seek_func = (ogg_seek)pSCallbacks->seek_func;
	ovcallbacks.seek_func = (ogg_seek)_OGG_Seek;  /* NOTE: I had some problems with a valid seeking function!! */
	ovcallbacks.close_func = pSCallbacks->close;
	ovcallbacks.tell_func = pSCallbacks->tell;

	// allocate OGG_INFO
	pSWI = (OGG_INFO*)malloc( sizeof(OGG_INFO) );
	memset( pSWI, 0, sizeof(OGG_INFO) );

	// allocate OggVorbis_File
	pSWI->vf = (OggVorbis_File*)malloc(sizeof(OggVorbis_File));
	memset( pSWI->vf, 0, sizeof(OggVorbis_File) );

	// open file
	hFile = pSCallbacks->open(szFile, "rb");

	if( !hFile ) {
		// unable to open file
		free( pSWI->vf );
		free( pSWI );
		return 0;
	}

	// setup internal ogg callbacks
	if( ov_open_callbacks( hFile, pSWI->vf, 0, 0, ovcallbacks )!=0 ) {
		free( pSWI->vf );
		free( pSWI );
		return 0;
	}
	//ov_open(hFile, &pSWI->vf, 0, 0);

	// set flags
	pSWI->dwFlags = dwFlags;

	// get ogg-vorbis info
	vinfo = ov_info( pSWI->vf, -1 );

	// setup WAVEFORMATEX for the sound-server
	if(pWfx) {
		pWfx->nAvgBytesPerSec = vinfo->rate * vinfo->channels * 2;
		pWfx->nChannels = (unsigned short)vinfo->channels;
		pWfx->nSamplesPerSec = vinfo->rate;
		pWfx->wBitsPerSample = 16;
		pWfx->nBlockAlign = (unsigned short)(vinfo->channels * 2);
		pWfx->wFormatTag = 1;
		pWfx->cbSize = 0; // cannot find out bytesize for ogg
	}

	return pSWI;
}


//-----------------------------------------------------------------------------------------
// OGG_Close
//-----------------------------------------------------------------------------------------
int SDYSTRMAPI OGG_Close(OGG_INFO* pSWI) {
	if( !pSWI ) { return 0; }

	ov_clear( pSWI->vf );

	free( pSWI->vf );
	free( pSWI );

	pSWI = 0;

	return 1;
}
