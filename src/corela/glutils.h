#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#ifdef _WIN32
  #define _WINSOCKAPI_
  #include <Windows.h>
#endif

#include <GL/gl.h>
#ifndef _WIN32
  #define GL_GLEXT_PROTOTYPES
#endif
#include "glext.h"
#include "corela.h"

#define CHECK_GL_ERROR(s) {glGetError();glUtilsReportError(glGetError(),s,__FILE__,__LINE__);}
#define BEGIN_CHECK_GL_ERROR() {glGetError();}
#define END_CHECK_GL_ERROR(s) {glUtilsReportError(glGetError(),s,__FILE__,__LINE__);}

typedef struct GL_CAPS {
	int nonPowerOfTwo;
	int anisoFilter;
	GLint maxTexDim;
	int maxAnisotropy;
} GL_CAPS;


#ifdef _WIN32
extern PFNGLGENBUFFERSARBPROC glGenBuffers;
extern PFNGLBINDBUFFERARBPROC glBindBuffer;
extern PFNGLBUFFERDATAARBPROC glBufferData;
extern PFNGLBUFFERSUBDATAARBPROC glBufferSubData;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
extern PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameteriv;
extern PFNGLMAPBUFFERARBPROC glMapBuffer;
extern PFNGLUNMAPBUFFERARBPROC glUnmapBuffer;
#endif


extern GL_CAPS glCaps;

void glUtilsInit();
void glUtilsTo2D(double left, double top, double right, double bottom, double nearPlane, double farPlane);
void glUtilsTo3D(double fov, double aspect, double nearPlane, double farPlane);
bool glUtilsSetRenderTarget(TEXTURE* texture);
TEXTURE* glUtilsGetRenderTarget();
void glUtilsReportError(GLenum err, const char* source = "unknown", const char* file = "unknown", int line = -1);
int glUtilsHasExtension(const char* e);
void glUtilsGenerateMipmap(GLenum target);
int glCapsCompatTextureSize(int dim);


#endif
