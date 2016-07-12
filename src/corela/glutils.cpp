#ifndef _WIN32
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include "glext.h"
#endif

#include <string.h>
#include <math.h>
#include <stdio.h>
#include "glutils.h"

#define PI 3.14159265
#define MIN_TEXTURE_SIZE 4

GL_CAPS glCaps;
static GLuint g_iFBO;
static TEXTURE* g_activeRenderTarget = 0;
static GLint g_WindowViewport[4];


#ifdef _WIN32
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffers;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1D;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3D;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;

PFNGLGENBUFFERSARBPROC glGenBuffers;
PFNGLBINDBUFFERARBPROC glBindBuffer;
PFNGLBUFFERDATAARBPROC glBufferData;
PFNGLBUFFERSUBDATAARBPROC glBufferSubData;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameteriv;
PFNGLMAPBUFFERARBPROC glMapBuffer;
PFNGLUNMAPBUFFERARBPROC glUnmapBuffer;
#endif


inline void _LogGLError(const char* error, const char* fn, const char* file, int line) {
	Log(LOG_ERROR, "OpenGL error %s in %s, function %s on line %d!", error, file, fn, line);
}

int _glCapsNonPowerOfTwo() {
	if(glUtilsHasExtension("IMG_texture_npot")) {
		return 1;
	}
	if(glUtilsHasExtension("ARB_texture_non_power_of_two")) {
		return 1;
	}

	const char* _ver = (char*)glGetString(GL_VERSION);
	if(_ver[0] != '1') {
		return 1;    // expect all opengl versions > 1.x to support NPOT.
	}

	return 0;
}

void glUtilsReportError(GLenum err, const char* source, const char* file, int line) {
	switch(err) {
	case GL_NO_ERROR:
		return;

	case GL_INVALID_ENUM:
		_LogGLError("GL_INVALID_ENUM", source, file, line);
		break;

	case GL_INVALID_VALUE:
		_LogGLError("GL_INVALID_VALUE", source, file, line);
		break;

	case GL_INVALID_OPERATION:
		_LogGLError("GL_INVALID_OPERATION", source, file, line);
		break;

	case GL_STACK_OVERFLOW:
		_LogGLError("GL_STACK_OVERFLOW", source, file, line);
		break;

	case GL_STACK_UNDERFLOW:
		_LogGLError("GL_STACK_UNDERFLOW", source, file, line);
		break;

	case GL_OUT_OF_MEMORY:
		_LogGLError("GL_OUT_OF_MEMORY", source, file, line);
		break;

	case GL_TABLE_TOO_LARGE:
		_LogGLError("GL_TABLE_TOO_LARGE", source, file, line);
		break;

	default:
		_LogGLError("unknown", source, file, line);
		break;
	}
}

void glUtilsGenerateMipmap(GLenum target) {
	if(glGenerateMipmap) {
		glGenerateMipmap(target);
	}
}

int glUtilsHasExtension(const char* e) {
	const char* _ext = (char*)glGetString(GL_EXTENSIONS);
	if(strstr(_ext, e) != 0) {
		return 1;
	}
	return 0;
}

int glCapsCompatTextureSize(int dim) {
	return dim;

	// with NPOT support only clamp max and min texture sizes
#ifdef _WIN32
	if(glCaps.nonPowerOfTwo) {
		if(dim > glCaps.maxTexDim) {
			return glCaps.maxTexDim;
		}
		if(dim < MIN_TEXTURE_SIZE) {
			return MIN_TEXTURE_SIZE;
		}
		return dim;
	}
#endif

	// adjust here for power-of-two
	// scales the dimension up instead of down to give better textures (if possible)
	int nMaxTex = glCaps.maxTexDim;
	do {
		nMaxTex = nMaxTex >> 1;
		if(dim > nMaxTex) {
			return nMaxTex << 1;
		}
	} while(nMaxTex > MIN_TEXTURE_SIZE);

	// requested a tiny texture dimension, use minimum texture size
	return MIN_TEXTURE_SIZE;
}

void glUtilsInit() {
	// check for mipmap generation by opengl
#ifdef _WIN32
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	if(!glGenerateMipmap) {
		Log(LOG_ERROR, "glGenerateMipmap is not supported, please implement manual mipmap generation!");
	}

	// load FBO extension and create a FBO object (will be reused)
	glDeleteRenderbuffers    = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffers");
	glBindFramebuffer        = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebuffer");
	glGenRenderbuffers       = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffers");
	glBindRenderbuffer       = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbuffer");
	glRenderbufferStorage    = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorage");
	glFramebufferRenderbuffer= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbuffer");
	glFramebufferTexture1D   = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)wglGetProcAddress("glFramebufferTexture1D");
	glFramebufferTexture2D   = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2D");
	glFramebufferTexture3D   = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)wglGetProcAddress("glFramebufferTexture3D");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatus");
	glGenFramebuffers        = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffers");
	if(!glBindFramebuffer) {
		Log(LOG_ERROR, "No FBO support in OpenGL implementation! This may cause problems!");
	} else {
		glGenFramebuffers(1, &g_iFBO);
	}

	// load PBO extension
	glGenBuffers = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
	glBindBuffer = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
	glBufferData = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
	glBufferSubData = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
	glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
	glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
	glMapBuffer = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
	glUnmapBuffer = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");
	if(!glGenBuffers) {
		Log(LOG_ERROR, "No PBO support in OpenGL implementation! This may cause stability or performance issues!");
	}
#endif

	// non power of two textures allowed?
	glCaps.nonPowerOfTwo = _glCapsNonPowerOfTwo();

	// get maximum texture dimension
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glCaps.maxTexDim);
#ifndef _WIN32
	glCaps.maxTexDim = 128;
#endif
	Log(LOG_DEBUG, "Maximum texture dimension is %d.", glCaps.maxTexDim);

	// check for anisotropic filtering by hardware
	glCaps.anisoFilter = glUtilsHasExtension("EXT_texture_filter_anisotropic");

	// get maximum anisotropy
	if(glCaps.anisoFilter) {
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glCaps.maxAnisotropy);
	} else {
		glCaps.maxAnisotropy = 0;
	}

	// set pixel store alignment to 4 bytes
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// enable alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// disable depth test
	glDisable(GL_DEPTH_TEST);

	// disable alpha test
	glDisable(GL_ALPHA_TEST);

	// disable face backface culling
	glDisable(GL_CULL_FACE);

	// disable lighting
	glDisable(GL_LIGHTING);
}

void glUtilsTo2D(double left, double top, double right, double bottom, double nearPlane, double farPlane) {
	GLint oldMatrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, nearPlane, farPlane);
	glMatrixMode(oldMatrixMode);
}

void glUtilsTo3D(double fov, double aspect, double nearPlane, double farPlane) {
	GLint oldMatrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
	double left, right, bottom, top;
	top = tan(fov * PI / 360.0) * nearPlane;
	bottom = -top;
	left = aspect * bottom;
	right = aspect * top;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(left, right, bottom, top, nearPlane, farPlane);
	glMatrixMode(oldMatrixMode);
}

bool glUtilsSetRenderTarget(TEXTURE* texture) {
	if(g_activeRenderTarget == texture) { return false; }

	// remember current viewport dimension, so we can restore it later
	if(g_activeRenderTarget == 0) {
		glGetIntegerv(GL_VIEWPORT, g_WindowViewport);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	// restore normal backbuffer output
	if(!texture) {
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		glViewport(g_WindowViewport[0], g_WindowViewport[1], g_WindowViewport[2], g_WindowViewport[3]);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		g_activeRenderTarget = 0;
		return true;
	} else {
		// set texture framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, g_iFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture->handle, 0);

		// check status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
		if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			Log(LOG_ERROR, "An OpenGL FBO error occurred (%x):", status);
			if (status == GL_FRAMEBUFFER_UNDEFINED) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_UNDEFINED"); }
			if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); }
			if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); }
			if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); }
			if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); }
			if (status == GL_FRAMEBUFFER_UNSUPPORTED) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_UNSUPPORTED"); }
			if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE	"); }
			if (status == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) { Log(LOG_ERROR, "\t GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); }
			return false;
		}

		// set viewport to texture size
		glViewport(0, 0, texture->width, texture->height);
		glUtilsTo2D(0, texture->height, texture->width, 0.0, 0.0, 1.0);

		// remember last render target
		g_activeRenderTarget = texture;

		return true;
	}
}

TEXTURE* glUtilsGetRenderTarget() {
	return g_activeRenderTarget;
}

