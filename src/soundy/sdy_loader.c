#include <string.h>
#include <stdio.h>
#include "sdy_loader.h"
#include "sdy_cont.h"

static CONT* g_cLoaders;
static SDY_IO g_io = { (sdyio_fopen)fopen, (sdyio_fread)fread, (sdyio_fseek)fseek, (sdyio_fclose)fclose, (sdyio_ftell)ftell, (sdyio_fwrite)fwrite };

/* -------------------------------------------------------------------------------------
// sdySetIO
//------------------------------------------------------------------------------------- */
SDY_INTERFACE void SDYAPI sdySetIO(SDY_IO* io)
{
	if(!io)
	{
		g_io.open = (sdyio_fopen)fopen;
		g_io.read = (sdyio_fread)fread;
		g_io.seek = (sdyio_fseek)fseek;
		g_io.close= (sdyio_fclose)fclose;
		g_io.write= (sdyio_fwrite)fwrite;
	}
	else memcpy(&g_io, io, sizeof(SDY_IO));
}

/* -------------------------------------------------------------------------------------
// sdyRegLoader
//------------------------------------------------------------------------------------- */
sdyDword SDYAPI sdyRegLoader(SDY_CB_OPEN open, SDY_CB_STREAM stream, SDY_CB_CLOSE close)
{
	SDY_LOADER* pLoader;
	
	assert(open);
	assert(stream);
	assert(close);
	
	if(!g_cLoaders) g_cLoaders = cnew();
	
	pLoader = (SDY_LOADER*)malloc(sizeof(SDY_LOADER));
	pLoader->open	= open;
	pLoader->stream = stream;
	pLoader->close	= close;
	
	cpush(g_cLoaders, (void*)pLoader);
	
	return (sdyDword)pLoader;
}

/* -------------------------------------------------------------------------------------
// sdyUnregAllLoaders
//------------------------------------------------------------------------------------- */
void SDYAPI sdyUnregAllLoaders(void)
{
	if(!g_cLoaders) return;
	
	while(ccount(g_cLoaders))
	{
		free(cpop(g_cLoaders));
	}
	cdel(g_cLoaders);
	g_cLoaders = 0;
}

/* -------------------------------------------------------------------------------------
// _sdyNewStream
//------------------------------------------------------------------------------------- */
SDY_STREAM* SDYAPI _sdyStreamNew(SDY_LOADER* pLoader, sdyPtr pInfo, const sdyString szFile)
{
	SDY_STREAM* pStream;
	
	assert(pLoader);
	assert(pInfo);
	
	pStream = (SDY_STREAM*)malloc(sizeof(SDY_STREAM));
	pStream->info			= pInfo;
	pStream->loader.open	= pLoader->open;
	pStream->loader.stream	= pLoader->stream;
	pStream->loader.close	= pLoader->close;
	strcpy(pStream->szFile, szFile);
	
	return pStream;
}

/* -------------------------------------------------------------------------------------
// sdyOpenStream
//------------------------------------------------------------------------------------- */
SDY_STREAM* SDYAPI sdyStreamOpen(const sdyString szFile, SDY_WFX* wfx)
{
	sdyPtr pInfo;
	SDY_LOADER* pLoader;
	CONT_ITEM*	i;
	
	if(!g_cLoaders)
		return sdyNull;
	
	i = cfirst(g_cLoaders);
	
	while(i)
	{
		pLoader = (SDY_LOADER*)cidata(i);
		
		pInfo = pLoader->open(szFile, wfx, &g_io, 0);
		if(pInfo)
			return _sdyStreamNew(pLoader, pInfo, szFile);
		
		i = cinext(i);
	}
	
	return sdyNull;
}

/* -------------------------------------------------------------------------------------
// sdyCloseStream
//------------------------------------------------------------------------------------- */
sdyBool SDYAPI sdyStreamClose(SDY_STREAM* pStream)
{
	sdyBool bSuccess;
	
	assert(pStream);
	if(!pStream) return sdyFalse;
	if(!pStream->info) return sdyFalse;
	
	bSuccess = pStream->loader.close(pStream->info);
	
	free(pStream);
	
	return sdyTrue;
}

/* -------------------------------------------------------------------------------------
// _sdyRewindStream
//------------------------------------------------------------------------------------- */
sdyBool SDYAPI sdyStreamRewind(SDY_STREAM* pStream)
{
	SDY_WFX wfx;
	
	pStream->loader.close(pStream->info);
	pStream->info = pStream->loader.open(pStream->szFile, &wfx, &g_io, 0);
	
	if(!pStream->info)
		return sdyFalse;
	
	return sdyTrue;
}

/* -------------------------------------------------------------------------------------
// sdyStreamStream
//------------------------------------------------------------------------------------- */
sdyDword SDYAPI sdyStreamStream(SDY_STREAM* pStream, sdyByte* pAdress, sdyDword dwByteSize, sdyBool bWrap)
{
	sdyDword dwTotal;
	sdyDword dwReaded;
	
	assert(pStream);
	if(!pStream) return 0;
	
	dwTotal = 0;
	
Restream:
	dwReaded = pStream->loader.stream(pStream->info, pAdress, dwByteSize);
	dwTotal += dwReaded;
	
	/* if the read size is smaller than requested    		     */ 
	/* rewind the stream and stream the rest from the beginning. */ 
	if(dwReaded < dwByteSize && bWrap)
	{
		if(!sdyStreamRewind(pStream))
			return dwTotal;
		dwByteSize -= dwReaded;
		pAdress += dwReaded;
		goto Restream;
	}
	
	return dwTotal;
}
