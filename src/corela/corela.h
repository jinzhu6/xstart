#ifndef _CORELA_H_
#define _CORELA_H_

#ifndef _WIN32
#define GL_GLEXT_PROTOTYPES
#endif

#include "corela_t.h"
#include <string>


CO_INTERFACE FRAME* FrameCreate(const char* title, EVENT_CALLBACK callback, void* user, long x=0x80000000, long y=0x80000000, long w=0x80000000, long h=0x80000000);
CO_INTERFACE void FrameActivate(FRAME* frame);
CO_INTERFACE void FrameClose(FRAME* frame);
CO_INTERFACE void FrameShow(FRAME* frame, int show);
CO_INTERFACE void FrameMaximize(FRAME* frame);
CO_INTERFACE void FrameMinimize(FRAME* frame);
CO_INTERFACE void FrameToggleFull(FRAME* frame);
CO_INTERFACE bool FrameUpdate();
CO_INTERFACE void FrameSelect(FRAME* frame);
CO_INTERFACE void FrameFlip(FRAME* frame);
CO_INTERFACE void FrameShowCursor(FRAME* frame, coBool show);
CO_INTERFACE void FrameSetScissor(FRAME* frame, int left, int top, int right, int bottom);
CO_INTERFACE int FrameGetMultitouchCount(FRAME* frame);
CO_INTERFACE bool FrameGetMultitouch(FRAME* frame, int index, float* x, float* y);
CO_INTERFACE void SoundSimplePlay(const char* file);
CO_INTERFACE bool GetMonitorRect(int i, CORECT* rcOut);
CO_INTERFACE std::string RequestFileName();
CO_INTERFACE int MapKey(int code, int mode);

CO_INTERFACE IMAGE* ImageCreate(int width, int height);
CO_INTERFACE IMAGE* ImageLoad(const char* filename);
CO_INTERFACE void ImageSave(IMAGE* image, const char* filename);
CO_INTERFACE void ImageSavePNG(IMAGE* image, const char* filename);
CO_INTERFACE void ImageDestroy(IMAGE* image);
CO_INTERFACE PIXEL* ImageGetPixel(IMAGE* image, int x, int y);
CO_INTERFACE void ImageSetPixel(IMAGE* image, int x, int y, PIXEL* pixel);
CO_INTERFACE void ImageMixPixel(IMAGE* image, int x, int y, PIXEL* src);
CO_INTERFACE IMAGE* ImageGetCut(IMAGE* src, int px, int py, int width, int height);
CO_INTERFACE void ImagePaste(IMAGE* dst, IMAGE* src, int px, int py, bool alpha);
CO_INTERFACE void ImageSwapRB(IMAGE* image);
CO_INTERFACE void ImageFlipX(IMAGE* src);
CO_INTERFACE void ImageFlipY(IMAGE* src);
CO_INTERFACE void ImageSoftenEdge(IMAGE* image, int edge);
CO_INTERFACE void ImageCopy(IMAGE* src, IMAGE* dst);
CO_INTERFACE void ImageCopyBilinear(IMAGE* src, IMAGE* dst);
CO_INTERFACE void ImageNormalMap(IMAGE* srcImage, IMAGE* dstImage, float scale, int specularAlpha);
CO_INTERFACE void ImageResize(IMAGE* src, int w, int h, bool bilinear);
CO_INTERFACE void ImageMix(IMAGE* dst, IMAGE* src, float f);
CO_INTERFACE void ImageMultiply(IMAGE* dst, IMAGE* mask, float offset, float scale);
CO_INTERFACE void ImageSaturateMask(IMAGE* dst, IMAGE* mask, float offset, float scale);
CO_INTERFACE void ImageMedian(IMAGE* srcImage, IMAGE* dstImage, int filterSize);
CO_INTERFACE void ImageFill(IMAGE* srcImage, coDword color);
CO_INTERFACE void ImageGreyscale(IMAGE* srcImage);
CO_INTERFACE void ImageSetAlpha(IMAGE* srcImage, BYTE a);
CO_INTERFACE void ImageSharpen(IMAGE* srcImage, IMAGE* dstImage, float sharpen);
CO_INTERFACE void ImageSavePNG(IMAGE* image, const char* filename);
CO_INTERFACE void ImageClampGray(IMAGE* srcImage, BYTE min, BYTE max);
CO_INTERFACE void ImageAlphaNoiseRemoval(IMAGE* srcImage, IMAGE* dstImage);

CO_INTERFACE bool ColorParse(const char* color, float* r, float* g, float* b, float* a);
CO_INTERFACE coDword ColorParse(const char* color);

CO_INTERFACE TEXTURE* TextureCreate(coDword width, coDword height, coDword flags, const char* clearColor);
CO_INTERFACE TEXTURE* TextureDuplicate(TEXTURE* source);
CO_INTERFACE void TextureDestroy(TEXTURE* tex);
CO_INTERFACE TEXTURE* TextureLoad(const char* file, coDword flags);
CO_INTERFACE void TextureSave(TEXTURE* tex, const char* file, int width, int height);
CO_INTERFACE coByte* TextureLock(TEXTURE* tex, coBool read, int format = TEX_FORMAT_RGBA);
CO_INTERFACE void TextureUnlock(TEXTURE* tex, coBool write, int format = TEX_FORMAT_RGBA);
CO_INTERFACE void TextureUpload(TEXTURE* tex, coByte* data, int format = TEX_FORMAT_RGBA);
CO_INTERFACE void TextureClear(TEXTURE* tex, coDword color = 0x80FF00FF);
CO_INTERFACE void TextureGetImage(TEXTURE* tex, IMAGE* im, int format = TEX_FORMAT_RGBA);
CO_INTERFACE void TextureSetImage(TEXTURE* tex, IMAGE* im, int format = TEX_FORMAT_RGBA);

CO_INTERFACE void* CaptureDevice(int index, const wchar_t* deviceName, int width, int height, int bits);
CO_INTERFACE void CaptureRelease(void* _device);
CO_INTERFACE void CaptureConfig(void* _device);
CO_INTERFACE coBool CaptureGetImage(void* _device, IMAGE* image, coDword* width, coDword* height);
CO_INTERFACE void CaptureSetFocus(void* _device, int focus);
CO_INTERFACE void CaptureSetZoom(void* _device, int zoom);
CO_INTERFACE void CaptureSetExposure(void* _device, int exp);
CO_INTERFACE void CaptureSetWhiteBalance(void* _device, int balance);
CO_INTERFACE void CaptureSetPowerlineFrequency(void* _device, int freq);
CO_INTERFACE void CaptureSetBrightness(void* _device, int brightness);
CO_INTERFACE void CaptureSetContrast(void* _device, int contrast);
CO_INTERFACE void CaptureSetSaturation(void* _device, int saturation);

CO_INTERFACE void* VideoOpen(const char* file);
CO_INTERFACE coBool VideoStart(void* video);
CO_INTERFACE coBool VideoStop(void* video);
CO_INTERFACE coBool VideoPause(void* _video);
CO_INTERFACE coBool VideoSetPosition(void* video, double position);
CO_INTERFACE double VideoGetPosition(void* _video);
CO_INTERFACE coBool VideoSetSpeed(void* _video, double speed);
CO_INTERFACE double VideoGetDuration(void* _video);
CO_INTERFACE coBool VideoGetImage(void* video, IMAGE* image, coDword* _width, coDword* _height);
CO_INTERFACE void VideoClose(void* video);

CO_INTERFACE FONT* FontLoad(const char* szFile, int nFontSize);
CO_INTERFACE FONT* FontCreate(coByte* pMem, coDword dwSize, int nFontSize);
CO_INTERFACE void FontDestroy(FONT* pFont);
CO_INTERFACE int FontMeasureWidth(FONT* pFont, const char* szText);
CO_INTERFACE int FontGetHeight(FONT* pFont);
CO_INTERFACE int FontRender(FONT* pFont, IMAGE* image, int x, int y, int* widthOut, int* heightOut, const char* text, const char* solidColor = "#ffffff", int outline = 0, const char* outlineColor = "#00", bool rToL = false);

CO_INTERFACE void GLSLInit();
CO_INTERFACE GLSLSHADER* GLSLShaderCreate(const char* szVS, const char* szFS, bool bFromFile);
CO_INTERFACE void GLSLShaderDestroy(GLSLSHADER* shader);
CO_INTERFACE void GLSLSetShader(GLSLSHADER* shader);
CO_INTERFACE GLSLSHADER* GLSLGetShader();
CO_INTERFACE int GLSLGetUniform(GLSLSHADER* shader, const char* szName);
CO_INTERFACE void GLSLSetUniform1i(int uni, int f);
CO_INTERFACE void GLSLSetUniform1f(int uni, float f);
CO_INTERFACE void GLSLSetUniform2f(int uni, float f1, float f2);
CO_INTERFACE void GLSLSetUniform3f(int uni, float f1, float f2, float f3);
CO_INTERFACE void GLSLSetUniform4f(int uni, float f1, float f2, float f3, float f4);
CO_INTERFACE void GLSLSetUniformMatrix(int uni, float* mx);

CO_INTERFACE INI* INIOpen(const char* szFile, coBool bCreate);
CO_INTERFACE void INIClose(INI* ini, int bSave);
CO_INTERFACE INISECTION* INIEnumSections(INI* ini, coDword dwNum);
CO_INTERFACE INIENTRY* INIEnumEntries(INISECTION* sec, coDword dwNum);
CO_INTERFACE INISECTION* INIGetSection(INI* ini, const char* szSec, coBool bCreate);
CO_INTERFACE INIENTRY* INIGetEntry(INISECTION* iniSec, const char* szKey, coBool bCreate, const char* szCreationValue);
CO_INTERFACE char* INIGetEntryStr(INIENTRY* iniEnt, char* str, int nMaxLen);
CO_INTERFACE long INIGetEntryNum(INIENTRY* iniEnt);
CO_INTERFACE coDword INIGetEntryHex(INIENTRY* iniEn);
CO_INTERFACE float INIGetEntryFloat(INIENTRY* iniEnt);
CO_INTERFACE void INISetEntryStr(INIENTRY* iniEnt, const char* str);
CO_INTERFACE void INISetEntryNum(INIENTRY* iniEnt, int n);
CO_INTERFACE void INISetEntryHex(INIENTRY* iniEnt, coDword h);
CO_INTERFACE void INISetEntryFloat(INIENTRY* iniEnt, float f);

CO_INTERFACE double TimeGet();
CO_INTERFACE void TimeSet(double t);
CO_INTERFACE void TimeSleep(double t);
CO_INTERFACE coBool KeyGet(int code);
CO_INTERFACE void Log(LOG_TYPE t, const char* format, ...);
CO_INTERFACE void SetLogLevel(int level);
CO_INTERFACE int GetLogLevel();
CO_INTERFACE void SetLogColors(int enable);

CO_INTERFACE std::string WebHttpGet(std::string &url);

CO_INTERFACE bool FileExists(const char* file);
CO_INTERFACE char* FileReadBuffer(const char* file, char* buffer, coDword* maxSize);
CO_INTERFACE coDword FileGetSize(const char* file);
CO_INTERFACE coDword FileReadText(const char* file, char* bufferOut, coDword* sizeInOut);
CO_INTERFACE void FileSaveBuffer(const char* file, const char* buffer, coDword length);
CO_INTERFACE void GetMousePosition(int* x, int* y);

CO_INTERFACE void RenderTextureQuad(TEXTURE* tx, double px, double py, double cx=0.0, double cy=0.0, double w=0.0, double h=0.0, TEXTURE_RECT* rc=0, float rotate=0, float scale=1.0, float alpha=1.0, bool flipX=false, bool flipY=false);
CO_INTERFACE void RenderMultiTextureQuad(TEXTURE* tx0, TEXTURE* tx1, double px, double py, double cx=0.0, double cy=0.0, double w=0.0, double h=0.0, TEXTURE_RECT* rc=0, float rotate=0, float scale=1.0, float alpha=1.0, bool flipX=false, bool flipY=false);
CO_INTERFACE void RenderVertices(coDword polyCount, GLfloat* positions, GLfloat* normals = 0, GLfloat* texCoords = 0, TEXTURE* tx0 = 0, TEXTURE* tx1 = 0, TEXTURE* tx2 = 0, TEXTURE* tx3 = 0);

CO_INTERFACE void* DLLGetFunctionPtr(const char* dll, const char* function);

CO_INTERFACE coDword ConvertToArabic(coDword* normal, int length, coDword* result, int maxResult);


#endif
