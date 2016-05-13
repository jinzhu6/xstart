#include "corela.h"
#include "utf8.h"
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>
#include "_SYSFONT.INL"

#define MAX_FONT_SURFACES 256
#define GLYPH_BORDER 4
#define CO_FONT_TEX 512

#define FONT_TEXTURE_FLAGS (TEX_CLEAR)

typedef struct FONTFACE_FORMAT {
	int nAscender;
	int nDescender;
	int nHeight;
	int nMaxAdvance;
} FONTFACE_FORMAT;

typedef struct FONTFACE {
	FT_Face ftFace;
	coByte* pMem;
	coDword dwSize;
	FONTFACE_FORMAT format;
} FONTFACE;

typedef struct GLYPH_FORMAT {
	int nBmpWidth;		/* width (in pixels) of the rendered glyph image */
	int nBmpHeight; 	/* height (in pixels) of the rendered glyph image */
	int nBmpPitch;		/* pitch (in bytes) from one row to the next */
	coByte* pBitmap;	/* in memory rendered bitmap glyph */
	int nBearLeft;		/* */
	int nBearTop;		/* */
	int nAdvanceX;		/* */
	int nAdvanceY;		/* */
	int nChIndex;		/* character index */
} GLYPH_FORMAT;

typedef struct GLYPH {
	FT_Glyph		ftGlyph;
	GLYPH_FORMAT	format;
} GLYPH;

typedef struct FONT {
	FONTFACE* pFace;
} FONT;


FONT* g_pFont;	/* Build-In font */
static FT_Library	g_pFT;		/* FreeType2 library handle (thirdparty) */


/* ----------------------------------------------------------------------------------------
// _FontSysDestroy
//----------------------------------------------------------------------------------------- */
void _FontSysDestroy() {
	if(g_pFont) {
		FontDestroy(g_pFont);
	}
	if(g_pFT) {
		FT_Done_FreeType(g_pFT);
	}
	g_pFT = 0;
	g_pFont = 0;
}


/* ----------------------------------------------------------------------------------------
// _coFontSysInit
//----------------------------------------------------------------------------------------- */
void _FontSysInit() {
	if(!g_pFT) {
		FT_Init_FreeType(&g_pFT);
		atexit(_FontSysDestroy);

		// Create build-in font
		g_pFont = FontCreate(g_SysFont, sizeof(g_SysFont), 18);
	}
}


/* ----------------------------------------------------------------------------------------
// _FontFaceCreate - Create new font face from in-memory data
//----------------------------------------------------------------------------------------- */
static FONTFACE* _FontFaceCreate(coByte* pData, coDword dwSize, int nFontSize, FT_Matrix* pMatrix) {
	_FontSysInit();

	FONTFACE* pFace = (FONTFACE*)malloc(sizeof(FONTFACE));
	memset(pFace, 0, sizeof(FONTFACE));
	pFace->pMem = (coByte*)malloc(dwSize);
	pFace->dwSize = dwSize;
	memcpy(pFace->pMem, pData, dwSize);

	int nErr = FT_New_Memory_Face(g_pFT, pFace->pMem, pFace->dwSize, 0, &pFace->ftFace);

	if(nErr) {
		free(pFace->pMem);
		free(pFace);
		return 0;
	}

	FT_Set_Char_Size(pFace->ftFace, 0, nFontSize << 6, 0, 0);
	pFace->format.nAscender   = pFace->ftFace->size->metrics.ascender >> 6;
	pFace->format.nDescender  = pFace->ftFace->size->metrics.descender >> 6;
	pFace->format.nHeight	  = pFace->ftFace->size->metrics.height >> 6;
	pFace->format.nMaxAdvance = pFace->ftFace->size->metrics.max_advance >> 6;

	// FIX: Font ascender
	pFace->format.nAscender += 3;
	pFace->format.nHeight +=3;

	return pFace;
}


/* ----------------------------------------------------------------------------------------
// _FontFaceDestroy
//----------------------------------------------------------------------------------------- */
static void _FontFaceDestroy(FONTFACE* pFace) {
	FT_Done_Face(pFace->ftFace);
	free(pFace->pMem);
	free(pFace);
}


/* ----------------------------------------------------------------------------------------
// _FontGlyphCreate - Create a glyph
//----------------------------------------------------------------------------------------- */
static GLYPH* _FontGlyphCreate(FONTFACE* pFace, unsigned int ch) {
	GLYPH* pGlyph;
	int nErr;
	FT_UInt nChIndex;

	// Create new glyph object
	pGlyph = (GLYPH*)malloc(sizeof(GLYPH));
	memset(pGlyph, 0, sizeof(GLYPH));

	// Get internal char index for the character
	nChIndex = FT_Get_Char_Index(pFace->ftFace, ch);
	if(nChIndex==0) {
		return pGlyph;
	}

	// Load glyph into internal glyph buffer
	nErr = FT_Load_Glyph(pFace->ftFace, nChIndex, 0); //FT_LOAD_TARGET_LIGHT);
	//nErr = FT_Load_Glyph(pFace->ftFace, nChIndex, 0);
	if(nErr) {
		free(pGlyph);
		return 0;
	}

	// Make copy of internal stored glyph
	nErr = FT_Get_Glyph(pFace->ftFace->glyph, &pGlyph->ftGlyph);
	if(nErr) {
		free(pGlyph);
		return 0;
	}

	// Convert glyph to bitmap glyph
	nErr = FT_Glyph_To_Bitmap(&pGlyph->ftGlyph, FT_RENDER_MODE_LIGHT, 0, 1);
	if(nErr) {
		free(pGlyph);
		return 0;
	}

	// Remember format properties
	pGlyph->format.nBmpWidth  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.width;
	pGlyph->format.nBmpHeight = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.rows;
	pGlyph->format.nBmpPitch  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.pitch;
	pGlyph->format.pBitmap	  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.buffer;
	pGlyph->format.nBearLeft  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->left;
	pGlyph->format.nBearTop   = ((FT_BitmapGlyph)pGlyph->ftGlyph)->top;
	pGlyph->format.nAdvanceX  = pGlyph->ftGlyph->advance.x >> 16;
	pGlyph->format.nAdvanceY  = pGlyph->ftGlyph->advance.y >> 16;
	pGlyph->format.nChIndex   = nChIndex;

	return pGlyph;
}


/* ----------------------------------------------------------------------------------------
// _FontGlyphOutlineCreate - Create a glyph outline
//----------------------------------------------------------------------------------------- */
static GLYPH* _FontGlyphOutlineCreate(FONTFACE* pFace, unsigned int ch, int strokeWidth) {
	GLYPH* pGlyph;
	int nErr;
	FT_UInt nChIndex;

	// Create new glyph object
	pGlyph = (GLYPH*)malloc(sizeof(GLYPH));
	memset(pGlyph, 0, sizeof(GLYPH));

	// Get internal char index for the character
	nChIndex = FT_Get_Char_Index(pFace->ftFace, ch);
	if(nChIndex==0) {
		return pGlyph;
	}

	// Load glyph into internal glyph buffer
	nErr = FT_Load_Glyph(pFace->ftFace, nChIndex, 0); //FT_LOAD_TARGET_LIGHT);
	if(nErr) {
		free(pGlyph);
		return 0;
	}

	// Make copy of internal stored glyph
	nErr = FT_Get_Glyph(pFace->ftFace->glyph, &pGlyph->ftGlyph);
	if(nErr) {
		free(pGlyph);
		return 0;
	}

	// Create stroker
	FT_Stroker stroker;
	FT_Stroker_New(g_pFT, &stroker);
	FT_Stroker_Set(stroker, strokeWidth * 10, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

	// Convert glyph to outline glyph
	nErr = FT_Glyph_Stroke(&pGlyph->ftGlyph, stroker, 1);
	if(nErr) {
		free(pGlyph);
		return 0;
	}

	// Destroy stroker
	FT_Stroker_Done(stroker);

	// Convert glyph to bitmap glyph
	nErr = FT_Glyph_To_Bitmap(&pGlyph->ftGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
	if(nErr) {
		free(pGlyph);
		return 0;
	}

	// Remember format properties
	pGlyph->format.nBmpWidth  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.width;
	pGlyph->format.nBmpHeight = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.rows;
	pGlyph->format.nBmpPitch  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.pitch;
	pGlyph->format.pBitmap	  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->bitmap.buffer;
	pGlyph->format.nBearLeft  = ((FT_BitmapGlyph)pGlyph->ftGlyph)->left;
	pGlyph->format.nBearTop   = ((FT_BitmapGlyph)pGlyph->ftGlyph)->top;
	pGlyph->format.nAdvanceX  = pGlyph->ftGlyph->advance.x >> 16;
	pGlyph->format.nAdvanceY  = pGlyph->ftGlyph->advance.y >> 16;
	pGlyph->format.nChIndex   = nChIndex;

	return pGlyph;
}


/* ----------------------------------------------------------------------------------------
// _FontGlyphDestroy
//----------------------------------------------------------------------------------------- */
inline static void _FontGlyphDestroy(GLYPH* pGlyph) {
	FT_Done_Glyph(pGlyph->ftGlyph);
	free(pGlyph);
}


/* ----------------------------------------------------------------------------------------
// FontLoad
//----------------------------------------------------------------------------------------- */
FONT* FontLoad(const char* szFile, int nFontSize) {
	_FontSysInit();

	FILE* hf;
	FONT* pFont;
	coDword dwSize;
	coByte* pMem;

	hf = fopen(szFile, "rb");
	if(!hf) {
		return 0;
	}

	fseek(hf, 0, SEEK_END);
	dwSize = ftell(hf);
	fseek(hf, 0, SEEK_SET);

	pMem = (coByte*)malloc(dwSize);
	fread(pMem, dwSize, 1, hf);

	fclose(hf);

	pFont = FontCreate(pMem, dwSize, nFontSize);
	free(pMem);

	return pFont;
}


/* ----------------------------------------------------------------------------------------
// FontCreate
//----------------------------------------------------------------------------------------- */
FONT* FontCreate(coByte* pMem, coDword dwSize, int nFontSize) {
	_FontSysInit();

	FONT* pFont;

	pFont = (FONT*)malloc(sizeof(FONT));
	memset(pFont, 0, sizeof(FONT));
	pFont->pFace = _FontFaceCreate(pMem, dwSize, nFontSize, 0);

	return pFont;
}


/* ----------------------------------------------------------------------------------------
// FontDestroy
//----------------------------------------------------------------------------------------- */
void FontDestroy(FONT* pFont) {
	_FontFaceDestroy(pFont->pFace);
	free(pFont);
}


/* ----------------------------------------------------------------------------------------
// _FontGetKerning
//----------------------------------------------------------------------------------------- */
static void _FontGetKerning(FT_Vector* vk, FONT* pFont, int ch, int ch_prev) {
	FT_UInt nIndex, nIndexPrev;

	if(FT_HAS_KERNING(pFont->pFace->ftFace) && ch_prev != -1) {
		nIndex = FT_Get_Char_Index(pFont->pFace->ftFace, ch);
		nIndexPrev = FT_Get_Char_Index(pFont->pFace->ftFace, ch_prev);

		FT_Get_Kerning(pFont->pFace->ftFace, nIndexPrev, nIndex, 0, vk);
	} else {
		vk->x = 0;
		vk->y = 0;
	}
}


/* ----------------------------------------------------------------------------------------
// FontGetHeight
//----------------------------------------------------------------------------------------- */
int FontGetHeight(FONT* pFont) {
	if(!pFont) {
		pFont = g_pFont;
	}
	return pFont->pFace->format.nHeight;
}


/* ----------------------------------------------------------------------------------------
// FontRender
//----------------------------------------------------------------------------------------- */
int FontRender(FONT* pFont, IMAGE* image, int x, int y, int* widthOut, int* heightOut, const char* text, const char* solidColor, int outline, const char* outlineColor, bool rToL) {
	if(widthOut) {
		*widthOut = 1;
	}
	if(heightOut) {
		*heightOut = 0;
		if(pFont) *heightOut = pFont->pFace->format.nHeight;
	}
	if(strlen(text) <= 0) {
		return 0;
	}

	_FontSysInit();
	if(!pFont) {
		pFont = g_pFont;
	}

	// parse solid color for text
	PIXEL pix;
	pix.red = 0;
	pix.green = 0;
	pix.blue = 0;
	pix.alpha = 0xff;
	coDword color = ColorParse(solidColor);
	pix.alpha = (color & 0x000000FF);
	pix.red   = (color & 0xFF000000) >> 24;
	pix.green = (color & 0x00FF0000) >> 16;
	pix.blue  = (color & 0x0000FF00) >> 8;
	BYTE mul_alpha = pix.alpha;

	// parse outline color for text
	PIXEL pixStroke;
	pixStroke.red = 0;
	pixStroke.green = 0;
	pixStroke.blue = 0;
	pixStroke.alpha = 0xff;
	coDword colorStroke = ColorParse(outlineColor);
	pixStroke.alpha = (colorStroke & 0x000000FF);
	pixStroke.red   = (colorStroke & 0xFF000000) >> 24;
	pixStroke.green = (colorStroke & 0x00FF0000) >> 16;
	pixStroke.blue  = (colorStroke & 0x0000FF00) >> 8;
	BYTE mul_alpha_stroke = pixStroke.alpha;

	// loop variables
	FT_Vector vKern;
	unsigned int ch_prev = -1;
	char* i = (char*)&text[0];
	char* end = i + strlen(text) + 1;
	int lines = 1;
	int curX = x;
	int curY = y + pFont->pFace->format.nAscender;
	int totalWidth = 0, totalHeight = 0;

	// unpack utf8 to unicode chars
	coDword* unicode = (coDword*)malloc(strlen(text) * sizeof(coDword) * 4);
	int ucLen = 0;
	do {
		unicode[ucLen++] = utf8::unchecked::next(i);
	} while(i < end);

	// arabian preprocessing
	coDword* unicodeArab = (coDword*)malloc(ucLen * sizeof(coDword) * 4);
	ConvertToArabic(unicode, ucLen, unicodeArab, -1);
	free(unicode);
	unicode = unicodeArab;

	// draw glyph by glyph
	// TODO: can be optimized with OpenMP
	for(int n=0; n<ucLen; n++) {

		// get utf8 character index
		utf8::uint32_t ch = unicode[n];
		if(ch == 0) {
			break;
		}

		// process line-break
		if(ch == '\n') {
			lines++;
			curX = 0;
			curY += pFont->pFace->format.nHeight;
			continue;
		}

		// create bitmap glyph
		GLYPH* glyph = _FontGlyphCreate(pFont->pFace, ch);
		if(!glyph) {
			continue;
		}

		// skip invalid glyphs
		if(glyph->format.nChIndex == 0) {
			_FontGlyphDestroy(glyph);
			continue;
		}

		// fix left side bearing that may case the first character to be drawn into negative x
		if(curX + glyph->format.nBearLeft - outline < 0) {
			curX -= (glyph->format.nBearLeft - outline);
		}

		// right-to-left reading offset
		int rToLOffset = glyph->format.nBmpWidth + outline + glyph->format.nBearLeft;

		// Draw to image
		if(image) {
			for(int ny = 0; ny < glyph->format.nBmpHeight; ny++) {
				for(int nx = 0; nx < glyph->format.nBmpWidth; nx++) {
					float alpha = ((float)glyph->format.pBitmap[nx + ny * glyph->format.nBmpPitch] / 255.0) * ((float)(color & 0x000000FF) / 255.0);
					pix.alpha = (coByte)(alpha * (float)mul_alpha);
					//if(pix.alpha == 0xff) continue;
					if(rToL) {
						ImageMixPixel(image, image->width - (curX + rToLOffset) + nx + glyph->format.nBearLeft, curY + ny - glyph->format.nBearTop, &pix);
					} else {
						ImageMixPixel(image, curX + nx + glyph->format.nBearLeft, curY + ny - glyph->format.nBearTop, &pix);
					}
				}
			}
		}

		// draw outline
		if(outline) {
			_FontGlyphDestroy(glyph);

			// create bitmap glyph
			glyph = _FontGlyphOutlineCreate(pFont->pFace, ch, outline);
			if(!glyph) {
				continue;
			}

			// skip invalid glyphs
			if(glyph->format.nChIndex == 0) {
				_FontGlyphDestroy(glyph);
				continue;
			}

			// Draw to image
			if(image) {
				for(int ny = 0; ny < glyph->format.nBmpHeight; ny++) {
					for(int nx = 0; nx < glyph->format.nBmpWidth; nx++) {
						float alpha = ((float)glyph->format.pBitmap[nx + ny * glyph->format.nBmpPitch] / 255.0) * ((float)(colorStroke & 0x000000FF) / 255.0);
						pixStroke.alpha = (coByte)(alpha * (float)mul_alpha_stroke);
						if(rToL) {
							ImageMixPixel(image, image->width - (curX + rToLOffset) + nx + glyph->format.nBearLeft, curY + ny - glyph->format.nBearTop, &pixStroke);
						} else {
							ImageMixPixel(image, curX + nx + glyph->format.nBearLeft, curY + ny - glyph->format.nBearTop, &pixStroke);
						}
					}
				}
			}
		}


		// return total width of the rendered text
		if(widthOut) {
			int _w = curX + glyph->format.nBmpWidth + glyph->format.nBearLeft;
			if(*widthOut < _w) {
				*widthOut = _w;
			}
//			if(*widthOut < curX) {
//				*widthOut = curX;
		}

		// advance position
		_FontGetKerning(&vKern, pFont, ch, ch_prev);
		ch_prev = ch;
		curX += vKern.x >> 6;
		curX += glyph->format.nAdvanceX;


		_FontGlyphDestroy(glyph);
	}

// return total height of the rendered text
	if(heightOut) {
		*heightOut = lines * pFont->pFace->format.nHeight;
	}

	free(unicode);

	return 0;
}


/* ----------------------------------------------------------------------------------------
// -
//----------------------------------------------------------------------------------------- */
/*void FontPrintf(FONT* pFont, IMAGE* pImage, int x, int y, const char* szFormat, ...)
{
	char szBuffer[4096];
	va_list fparams;

	if(!pFont) pFont = g_pFont;

	if(szFormat)
	{
		va_start(fparams, szFormat);
		_vsnprintf(szBuffer, 4096-1, szFormat, fparams);
		va_end(fparams);
	}
	else
	{
		strcpy(szBuffer, "");
	}

	FontRender(pFont, pImage, x, y, 0, 0, szBuffer);

	return;
}*/

