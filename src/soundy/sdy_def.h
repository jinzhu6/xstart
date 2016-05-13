#ifndef _SDY_DEF_H_
#define _SDY_DEF_H_

#include <stdlib.h>

/* *******************************************************************
// DEFINITIONS
//******************************************************************* */
#define MAX_FILENAME		1024
#define MAX_AUDLINES		128
#define MAX_BUFSEC			20000		/* max. buffer size for streaming sounds */
#define SLEEPING_THREAD 	1			/* sleeping thread for auto update */

/* default settings: */
#define DEF_SAMPLES 		44100		/* default samples per second */
#define DEF_CHANNELS		2			/* speakers */
#define DEF_BPS 			16			/* bits per sample */
#define DEF_BUFSEC			480 		/* buffer size in ms */
#define DEF_AUDLINES		16			/* max audio lines */
#define DEF_MAXDUPLI		8			/* max sound duplicates */
#define DEF_MANUALUPDATE	0			/* manual update */
#define DEF_3DSOUND 		1			/* 3d sound */


/* *******************************************************************
// CALLING CONVENTION
//******************************************************************* */
#define SDYSTRMAPI
#define SDYAPI
#define CAPI		__cdecl


/* *******************************************************************
// BUILDING SPECIFICATION
//******************************************************************* */
//#define SDY_DLL
//#define _SDY_BUILDING
#ifdef SDY_DLL
#ifndef SDY_INTERFACE
#ifdef _SDY_BUILDING
#define SDY_INTERFACE		__declspec(dllexport)
#else
#define SDY_INTERFACE		__declspec(dllimport)
#endif
#endif
#else
#ifndef SDY_INTERFACE
#define SDY_INTERFACE
#endif
#endif


/* *******************************************************************
// FLAGS
//******************************************************************* */
//#define SDY_LOOP 1


/* *******************************************************************
// SOUND PRIORITY LEVEL (you may use any number instead)
//******************************************************************* */
typedef enum SDY_PLAYPRIORITY {
	VERY_LOW	= 0,
	LOW 		= 1,
	NORMAL		= 2,
	HIGH		= 3,
	VERY_HIGH	= 4

} SDY_PLAYPRIORITY;


/* *******************************************************************
// TYPES
//******************************************************************* */
#define sdyByte 	unsigned char
#define sdyWord 	unsigned short
#define sdyDword	unsigned long
#define sdyChar 	char
#define sdyShort	signed short
#define sdyInt		int /* signed long */
#define sdyLong 	long
#define sdyPtr		void*
#define sdyString	char*
#define sdyFloat	float
#define sdyBool 	int
#define sdyTrue 	1
#define sdyFalse	0
#define sdyNull 	0


/* *******************************************************************
// SDY_WFX structure
// (WARNING: Uses a DWORD instead of an WORD for the last element,
//	unlike the WAVEFORMATEX structure, but this should be safely
//	passed to any function that uses the WAVEFORMATEX struct.)
//******************************************************************* */
typedef struct SDY_WFX {
	sdyWord 	wFormatTag; 		/* format type										*/
	sdyWord 	nChannels;			/* number of channels (i.e. mono, stereo...)		*/
	sdyDword	nSamplesPerSec; 	/* sample rate										*/
	sdyDword	nAvgBytesPerSec;	/* for buffer estimation							*/
	sdyWord 	nBlockAlign;		/* block size of data								*/
	sdyWord 	wBitsPerSample; 	/* Number of bits per sample of mono data			*/
	sdyDword	cbSize; 			/* The count in bytes of the size of				*/
	/* extra information (after cbSize) 				*/
} SDY_WFX;
#define SDY_FMT_S16 0x0001
#define SDY_FMT_F32 0x0003


/* *******************************************************************
// IO CALLBACKS
//******************************************************************* */
//typedef unsigned int size_t;
typedef void*	(*sdyio_fopen)	(const char* filename, const char* mode);
typedef size_t	(*sdyio_fread)	(void *buffer, size_t size, size_t nmemb, void *handle);
typedef int 	(*sdyio_fseek)	(void *handle, long offset, int whence);
typedef int 	(*sdyio_fclose) (void *handle);
typedef long	(*sdyio_ftell)	(void *handle);
typedef size_t	(*sdyio_fwrite) (void *buffer, size_t size, size_t count, void *handle);

typedef struct SDY_IO {
	sdyio_fopen 	open;
	sdyio_fread 	read;
	sdyio_fseek 	seek;
	sdyio_fclose	close;
	sdyio_ftell 	tell;
	sdyio_fwrite	write;

} SDY_IO;


/* *******************************************************************
// STREAM CALLBACKS AND STRUCT
//******************************************************************* */
typedef sdyPtr	(SDYSTRMAPI *SDY_CB_OPEN)		(const sdyString, SDY_WFX*, SDY_IO*, sdyDword);
typedef sdyInt	(SDYSTRMAPI *SDY_CB_CLOSE)		(sdyPtr);
typedef sdyDword(SDYSTRMAPI *SDY_CB_STREAM) 	(sdyPtr, sdyByte*, sdyDword);

typedef struct SDY_LOADER {
	SDY_CB_OPEN 		open;
	SDY_CB_STREAM		stream;
	SDY_CB_CLOSE		close;

} SDY_LOADER;


/* *******************************************************************
// SETTINGS STRUCTURE
//******************************************************************* */
typedef struct {
	sdyWord 	wChannels;			/* channels/speakers (mono=1; stereo=2; default: 2) */
	sdyDword	dwSamplesPerSec;	/* samples per second (default: 44100) */
	sdyWord 	wBitsPerSample; 	/* bits per sample (default: 16) */
	sdyWord 	wBufSec;			/* stream-buffer size (default: 480) in milliseconds */
	sdyWord 	wAudLines;			/* number of audio lines (channels) that can be played simultaneously (default: 16) */
	sdyBool 	b3DSound;			/* enable 3D sound */
	sdyBool 	bManualUpdate;		/* if true the API handles updates iteself via thread - otherwise use sdyUpdate() */

} SDY_SETTINGS;


/* *******************************************************************
// STREAM STRUCTURE
//******************************************************************* */
typedef struct SDY_STREAM {
	sdyPtr			info;
	SDY_LOADER		loader;
	sdyChar 		szFile[MAX_FILENAME];

} SDY_STREAM;


/* *******************************************************************
// VECTOR STRUCTURE (3D sound)
//******************************************************************* */
typedef struct SDY_VEC3 {
	sdyFloat x, y, z;

} SDY_VEC3;


/* *******************************************************************
// LISTENER STRUCTURE (3D sound)
//******************************************************************* */
typedef struct SDY_LISTENER {
	SDY_VEC3		vPos;					/* position */
	SDY_VEC3		vVel;					/* velocity */
	SDY_VEC3		vFront; 				/* front orientation */
	SDY_VEC3		vTop;					/* upside orientation */
	sdyFloat		fDistMul;				/* distance factor */
	sdyFloat		fRollMul;				/* rolloff factor */
	sdyFloat		fDoppMul;				/* doppler factor */

} SDY_LISTENER;


/* *******************************************************************
// CTRL3D STRUCTURE (3D sound)
//******************************************************************* */
typedef struct SDY_CTRL3D {
	SDY_VEC3		vPos;
	SDY_VEC3		vVel;
	/* cone properties are unsupported */
	/*	sdyDword		dwInConeAngle;
		sdyDword		dwOutConeAngle;
		SDY_VEC3		vConeOrientation;
		sdyLong 		lConeOutVolume; */
	sdyFloat		fMinDist;
	sdyFloat		fMaxDist;
	/* pos and vel are always in world coordinates */
	/*	sdyDword		dwMode; */

} SDY_CTRL3D;


/* *******************************************************************
// SOUND STRUCTURE
//******************************************************************* */
typedef struct SDY_SOUND {
	SDY_STREAM* 	pStream;				/* streaming information and callbacks (see above) */
	sdyBool 		bStatic;				/* is a static sound (no streaming) with all data buffered */
	sdyBool 		bLoopStream;			/* set by each call to sdyPlay (only valid for streaming sounds) */
	sdyDword		dwSndSize;				/* length of song (in bytes?), may not be set */
	sdyDword		dwLastUpdate;			/* the last update that has been made (in buffer position) */
	sdyDword		dwStrmPlayed;			/* absolute stream cursor */
	sdyInt			nPriority;				/* priority of the playback (any number) */

	sdyChar 		szFile[MAX_FILENAME];	/* (place in SDY_STREAM) file-name where the sound comes from */
	sdyPtr			pBuffer;				/* pointer to a buffer */
	sdyDword		dwBufSize;				/* (obsolete) size of the buffer */
	sdyByte 		bySilence;				/* (obsolete) silence byte (0 for 8 bit, 128 for 16 bit) */

//	SDY_WFX 		wfx;					/* format */
//	SDY_CB_STREAM	fnStream;				/* */
//	sdyDword		dwRefCount; 			/* ?? reference count */
	sdyBool 		bAutoRemove;			/* sound will be closed/destroyed when not active/playing */
	sdyDword		dwStreamed; 			/* internal counter */

	sdyBool 		bDuplicate; 			/* is a (internally) automatically duplicated sound */
	void*			pParent;				/* parent sound (if duplicated) */
	sdyDword		dwMaxDuplicates;		/* max allowed number of duplicates */
	sdyDword		dwNumDuplicates;		/* current number of duplicates */

	sdyBool 		b3DSound;				/* is 3d sound (NOTE: 3d-sounds must be mono) */
	SDY_CTRL3D		ctrl3d;					/* 3d sound settings properties */

} SDY_SOUND;


#endif
