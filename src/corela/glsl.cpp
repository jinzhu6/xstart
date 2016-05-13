#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#else
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/gl.h>
#include "glext.h"
#include <stdio.h>
#include "corela.h"
#include "strtrim.h"


#ifdef _WIN32
PFNGLATTACHSHADERPROC				glAttachShader;
PFNGLBINDATTRIBLOCATIONPROC			glBindAttribLocation;
PFNGLCOMPILESHADERPROC				glCompileShader;
PFNGLCREATEPROGRAMPROC				glCreateProgram;
PFNGLCREATESHADERPROC				glCreateShader;
PFNGLDELETEPROGRAMPROC				glDeleteProgram;
PFNGLDELETESHADERPROC				glDeleteShader;
PFNGLDETACHSHADERPROC				glDetachShader;
PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray;
PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray;
PFNGLGETACTIVEATTRIBPROC			glGetActiveAttrib;
PFNGLGETACTIVEUNIFORMPROC			glGetActiveUniform;
PFNGLGETATTACHEDSHADERSPROC			glGetAttachedShaders;
PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation;
PFNGLGETPROGRAMIVPROC				glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog;
PFNGLGETSHADERIVPROC				glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog;
PFNGLGETSHADERSOURCEPROC			glGetShaderSource;
PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation;
PFNGLGETUNIFORMFVPROC				glGetUniformfv;
PFNGLGETUNIFORMIVPROC				glGetUniformiv;
PFNGLGETVERTEXATTRIBDVPROC			glGetVertexAttribdv;
PFNGLGETVERTEXATTRIBFVPROC			glGetVertexAttribfv;
PFNGLGETVERTEXATTRIBIVPROC			glGetVertexAttribiv;
PFNGLGETVERTEXATTRIBPOINTERVPROC	glGetVertexAttribPointerv;
PFNGLISPROGRAMPROC					glIsProgram;
PFNGLISSHADERPROC					glIsShader;
PFNGLLINKPROGRAMPROC				glLinkProgram;
PFNGLSHADERSOURCEPROC				glShaderSource;
PFNGLUSEPROGRAMPROC					glUseProgram;
PFNGLUNIFORM1FPROC					glUniform1f;
PFNGLUNIFORM2FPROC					glUniform2f;
PFNGLUNIFORM3FPROC					glUniform3f;
PFNGLUNIFORM4FPROC					glUniform4f;
PFNGLUNIFORM1IPROC					glUniform1i;
PFNGLUNIFORM2IPROC					glUniform2i;
PFNGLUNIFORM3IPROC					glUniform3i;
PFNGLUNIFORM4IPROC					glUniform4i;
PFNGLUNIFORM1FVPROC					glUniform1fv;
PFNGLUNIFORM2FVPROC					glUniform2fv;
PFNGLUNIFORM3FVPROC					glUniform3fv;
PFNGLUNIFORM4FVPROC					glUniform4fv;
PFNGLUNIFORM1IVPROC					glUniform1iv;
PFNGLUNIFORM2IVPROC					glUniform2iv;
PFNGLUNIFORM3IVPROC					glUniform3iv;
PFNGLUNIFORM4IVPROC					glUniform4iv;
PFNGLUNIFORMMATRIX2FVPROC			glUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC			glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv;
PFNGLVALIDATEPROGRAMPROC			glValidateProgram;
PFNGLVERTEXATTRIB1DPROC				glVertexAttrib1d;
PFNGLVERTEXATTRIB1DVPROC			glVertexAttrib1dv;
PFNGLVERTEXATTRIB1FPROC				glVertexAttrib1f;
PFNGLVERTEXATTRIB1FVPROC			glVertexAttrib1fv;
PFNGLVERTEXATTRIB1SPROC				glVertexAttrib1s;
PFNGLVERTEXATTRIB1SVPROC			glVertexAttrib1sv;
PFNGLVERTEXATTRIB2DPROC				glVertexAttrib2d;
PFNGLVERTEXATTRIB2DVPROC			glVertexAttrib2dv;
PFNGLVERTEXATTRIB2FPROC				glVertexAttrib2f;
PFNGLVERTEXATTRIB2FVPROC			glVertexAttrib2fv;
PFNGLVERTEXATTRIB2SPROC				glVertexAttrib2s;
PFNGLVERTEXATTRIB2SVPROC			glVertexAttrib2sv;
PFNGLVERTEXATTRIB3DPROC				glVertexAttrib3d;
PFNGLVERTEXATTRIB3DVPROC			glVertexAttrib3dv;
PFNGLVERTEXATTRIB3FPROC				glVertexAttrib3f;
PFNGLVERTEXATTRIB3FVPROC			glVertexAttrib3fv;
PFNGLVERTEXATTRIB3SPROC				glVertexAttrib3s;
PFNGLVERTEXATTRIB3SVPROC			glVertexAttrib3sv;
PFNGLVERTEXATTRIB4NBVPROC			glVertexAttrib4Nbv;
PFNGLVERTEXATTRIB4NIVPROC			glVertexAttrib4Niv;
PFNGLVERTEXATTRIB4NSVPROC			glVertexAttrib4Nsv;
PFNGLVERTEXATTRIB4NUBPROC			glVertexAttrib4Nub;
PFNGLVERTEXATTRIB4NUBVPROC			glVertexAttrib4Nubv;
PFNGLVERTEXATTRIB4NUIVPROC			glVertexAttrib4Nuiv;
PFNGLVERTEXATTRIB4NUSVPROC			glVertexAttrib4Nusv;
PFNGLVERTEXATTRIB4BVPROC			glVertexAttrib4bv;
PFNGLVERTEXATTRIB4DPROC				glVertexAttrib4d;
PFNGLVERTEXATTRIB4DVPROC			glVertexAttrib4dv;
PFNGLVERTEXATTRIB4FPROC				glVertexAttrib4f;
PFNGLVERTEXATTRIB4FVPROC			glVertexAttrib4fv;
PFNGLVERTEXATTRIB4IVPROC			glVertexAttrib4iv;
PFNGLVERTEXATTRIB4SPROC				glVertexAttrib4s;
PFNGLVERTEXATTRIB4SVPROC			glVertexAttrib4sv;
PFNGLVERTEXATTRIB4UBVPROC			glVertexAttrib4ubv;
PFNGLVERTEXATTRIB4UIVPROC			glVertexAttrib4uiv;
PFNGLVERTEXATTRIB4USVPROC			glVertexAttrib4usv;
PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;
#endif


GLSLSHADER* g_activeShader = 0;


#ifdef _WIN32
void* _GLSLGetFuncProc(const char* szProc) {
	void* proc = (void*)wglGetProcAddress(szProc);
	if(proc) { return proc; }

	char szProcNoARB[256];
	strcpy(szProcNoARB, szProc);
	szProcNoARB[ strlen(szProc)-3 ] = '\0';

	proc = (void*)wglGetProcAddress(szProcNoARB);
	if(proc) { return proc; }

	//waDbgPopup2("Error while searching for GLSL function: %s", szProc);
	return 0;
}
#endif


void GLSLInit() {
#ifdef _WIN32
	glAttachShader 				= (PFNGLATTACHSHADERPROC) _GLSLGetFuncProc("glAttachShaderARB");
	glBindAttribLocation 		= (PFNGLBINDATTRIBLOCATIONPROC) _GLSLGetFuncProc("glBindAttribLocationARB");
	glCompileShader				= (PFNGLCOMPILESHADERPROC) _GLSLGetFuncProc("glCompileShaderARB");
	glCreateProgram				= (PFNGLCREATEPROGRAMPROC) _GLSLGetFuncProc("glCreateProgramARB");
	glCreateShader				= (PFNGLCREATESHADERPROC) _GLSLGetFuncProc("glCreateShaderARB");
	glDeleteProgram				= (PFNGLDELETEPROGRAMPROC) _GLSLGetFuncProc("glDeleteProgramARB");
	glDeleteShader				= (PFNGLDELETESHADERPROC) _GLSLGetFuncProc("glDeleteShaderARB");
	glDetachShader				= (PFNGLDETACHSHADERPROC) _GLSLGetFuncProc("glDetachShaderARB");
	glDisableVertexAttribArray	= (PFNGLDISABLEVERTEXATTRIBARRAYPROC) _GLSLGetFuncProc("glDisableVertexAttribArrayARB");
	glEnableVertexAttribArray	= (PFNGLENABLEVERTEXATTRIBARRAYPROC) _GLSLGetFuncProc("glEnableVertexAttribArrayARB");
	glGetActiveAttrib			= (PFNGLGETACTIVEATTRIBPROC) _GLSLGetFuncProc("glGetActiveAttribARB");
	glGetActiveUniform			= (PFNGLGETACTIVEUNIFORMPROC) _GLSLGetFuncProc("glGetActiveUniformARB");
	glGetAttachedShaders		= (PFNGLGETATTACHEDSHADERSPROC) _GLSLGetFuncProc("glGetAttachedShadersARB");
	glGetAttribLocation			= (PFNGLGETATTRIBLOCATIONPROC) _GLSLGetFuncProc("glGetAttribLocationARB");
	glGetProgramiv				= (PFNGLGETPROGRAMIVPROC) _GLSLGetFuncProc("glGetProgramivARB");
	glGetProgramInfoLog			= (PFNGLGETPROGRAMINFOLOGPROC) _GLSLGetFuncProc("glGetProgramInfoLogARB");
	glGetShaderiv				= (PFNGLGETSHADERIVPROC) _GLSLGetFuncProc("glGetShaderivARB");
	glGetShaderInfoLog			= (PFNGLGETSHADERINFOLOGPROC) _GLSLGetFuncProc("glGetShaderInfoLogARB");
	glGetShaderSource			= (PFNGLGETSHADERSOURCEPROC) _GLSLGetFuncProc("glGetShaderSourceARB");
	glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONPROC) _GLSLGetFuncProc("glGetUniformLocationARB");
	glGetUniformfv				= (PFNGLGETUNIFORMFVPROC) _GLSLGetFuncProc("glGetUniformfvARB");
	glGetUniformiv				= (PFNGLGETUNIFORMIVPROC) _GLSLGetFuncProc("glGetUniformivARB");
	glGetVertexAttribdv			= (PFNGLGETVERTEXATTRIBDVPROC) _GLSLGetFuncProc("glGetVertexAttribdvARB");
	glGetVertexAttribfv			= (PFNGLGETVERTEXATTRIBFVPROC) _GLSLGetFuncProc("glGetVertexAttribfvARB");
	glGetVertexAttribiv			= (PFNGLGETVERTEXATTRIBIVPROC) _GLSLGetFuncProc("glGetVertexAttribivARB");
	glGetVertexAttribPointerv	= (PFNGLGETVERTEXATTRIBPOINTERVPROC) _GLSLGetFuncProc("glGetVertexAttribPointervARB");
	glIsProgram					= (PFNGLISPROGRAMPROC) _GLSLGetFuncProc("glIsProgramARB");
	glIsShader					= (PFNGLISSHADERPROC) _GLSLGetFuncProc("glIsShaderARB");
	glLinkProgram				= (PFNGLLINKPROGRAMPROC) _GLSLGetFuncProc("glLinkProgramARB");
	glShaderSource				= (PFNGLSHADERSOURCEPROC) _GLSLGetFuncProc("glShaderSourceARB");
	glUseProgram				= (PFNGLUSEPROGRAMPROC) _GLSLGetFuncProc("glUseProgramARB");
	glUniform1f					= (PFNGLUNIFORM1FPROC) _GLSLGetFuncProc("glUniform1fARB");
	glUniform2f					= (PFNGLUNIFORM2FPROC) _GLSLGetFuncProc("glUniform2fARB");
	glUniform3f					= (PFNGLUNIFORM3FPROC) _GLSLGetFuncProc("glUniform3fARB");
	glUniform4f					= (PFNGLUNIFORM4FPROC) _GLSLGetFuncProc("glUniform4fARB");
	glUniform1i					= (PFNGLUNIFORM1IPROC) _GLSLGetFuncProc("glUniform1iARB");
	glUniform2i					= (PFNGLUNIFORM2IPROC) _GLSLGetFuncProc("glUniform2iARB");
	glUniform3i					= (PFNGLUNIFORM3IPROC) _GLSLGetFuncProc("glUniform3iARB");
	glUniform4i					= (PFNGLUNIFORM4IPROC) _GLSLGetFuncProc("glUniform4iARB");
	glUniform1fv				= (PFNGLUNIFORM1FVPROC) _GLSLGetFuncProc("glUniform1fvARB");
	glUniform2fv				= (PFNGLUNIFORM2FVPROC) _GLSLGetFuncProc("glUniform2fvARB");
	glUniform3fv				= (PFNGLUNIFORM3FVPROC) _GLSLGetFuncProc("glUniform3fvARB");
	glUniform4fv				= (PFNGLUNIFORM4FVPROC) _GLSLGetFuncProc("glUniform4fvARB");
	glUniform1iv				= (PFNGLUNIFORM1IVPROC) _GLSLGetFuncProc("glUniform1ivARB");
	glUniform2iv				= (PFNGLUNIFORM2IVPROC) _GLSLGetFuncProc("glUniform2ivARB");
	glUniform3iv				= (PFNGLUNIFORM3IVPROC) _GLSLGetFuncProc("glUniform3ivARB");
	glUniform4iv				= (PFNGLUNIFORM4IVPROC) _GLSLGetFuncProc("glUniform4ivARB");
	glUniformMatrix2fv			= (PFNGLUNIFORMMATRIX2FVPROC) _GLSLGetFuncProc("glUniformMatrix2fvARB");
	glUniformMatrix3fv			= (PFNGLUNIFORMMATRIX3FVPROC) _GLSLGetFuncProc("glUniformMatrix3fvARB");
	glUniformMatrix4fv			= (PFNGLUNIFORMMATRIX4FVPROC) _GLSLGetFuncProc("glUniformMatrix4fvARB");
	glValidateProgram			= (PFNGLVALIDATEPROGRAMPROC) _GLSLGetFuncProc("glValidateProgramARB");
	glVertexAttrib1d			= (PFNGLVERTEXATTRIB1DPROC) _GLSLGetFuncProc("glVertexAttrib1dARB");
	glVertexAttrib1dv			= (PFNGLVERTEXATTRIB1DVPROC) _GLSLGetFuncProc("glVertexAttrib1dvARB");
	glVertexAttrib1f			= (PFNGLVERTEXATTRIB1FPROC) _GLSLGetFuncProc("glVertexAttrib1fARB");
	glVertexAttrib1fv			= (PFNGLVERTEXATTRIB1FVPROC) _GLSLGetFuncProc("glVertexAttrib1fvARB");
	glVertexAttrib1s			= (PFNGLVERTEXATTRIB1SPROC) _GLSLGetFuncProc("glVertexAttrib1sARB");
	glVertexAttrib1sv			= (PFNGLVERTEXATTRIB1SVPROC) _GLSLGetFuncProc("glVertexAttrib1svARB");
	glVertexAttrib2d			= (PFNGLVERTEXATTRIB2DPROC) _GLSLGetFuncProc("glVertexAttrib2dARB");
	glVertexAttrib2dv			= (PFNGLVERTEXATTRIB2DVPROC) _GLSLGetFuncProc("glVertexAttrib2dvARB");
	glVertexAttrib2f			= (PFNGLVERTEXATTRIB2FPROC) _GLSLGetFuncProc("glVertexAttrib2fARB");
	glVertexAttrib2fv			= (PFNGLVERTEXATTRIB2FVPROC) _GLSLGetFuncProc("glVertexAttrib2fvARB");
	glVertexAttrib2s			= (PFNGLVERTEXATTRIB2SPROC) _GLSLGetFuncProc("glVertexAttrib2sARB");
	glVertexAttrib2sv			= (PFNGLVERTEXATTRIB2SVPROC) _GLSLGetFuncProc("glVertexAttrib2svARB");
	glVertexAttrib3d			= (PFNGLVERTEXATTRIB3DPROC) _GLSLGetFuncProc("glVertexAttrib3dARB");
	glVertexAttrib3dv			= (PFNGLVERTEXATTRIB3DVPROC) _GLSLGetFuncProc("glVertexAttrib3dvARB");
	glVertexAttrib3f			= (PFNGLVERTEXATTRIB3FPROC) _GLSLGetFuncProc("glVertexAttrib3fARB");
	glVertexAttrib3fv			= (PFNGLVERTEXATTRIB3FVPROC) _GLSLGetFuncProc("glVertexAttrib3fvARB");
	glVertexAttrib3s			= (PFNGLVERTEXATTRIB3SPROC) _GLSLGetFuncProc("glVertexAttrib3sARB");
	glVertexAttrib3sv			= (PFNGLVERTEXATTRIB3SVPROC) _GLSLGetFuncProc("glVertexAttrib3svARB");
	glVertexAttrib4Nbv			= (PFNGLVERTEXATTRIB4NBVPROC) _GLSLGetFuncProc("glVertexAttrib4NbvARB");
	glVertexAttrib4Niv			= (PFNGLVERTEXATTRIB4NIVPROC) _GLSLGetFuncProc("glVertexAttrib4NivARB");
	glVertexAttrib4Nsv			= (PFNGLVERTEXATTRIB4NSVPROC) _GLSLGetFuncProc("glVertexAttrib4NsvARB");
	glVertexAttrib4Nub			= (PFNGLVERTEXATTRIB4NUBPROC) _GLSLGetFuncProc("glVertexAttrib4NubARB");
	glVertexAttrib4Nubv			= (PFNGLVERTEXATTRIB4NUBVPROC) _GLSLGetFuncProc("glVertexAttrib4NubvARB");
	glVertexAttrib4Nuiv			= (PFNGLVERTEXATTRIB4NUIVPROC) _GLSLGetFuncProc("glVertexAttrib4NuivARB");
	glVertexAttrib4Nusv			= (PFNGLVERTEXATTRIB4NUSVPROC) _GLSLGetFuncProc("glVertexAttrib4NusvARB");
	glVertexAttrib4bv			= (PFNGLVERTEXATTRIB4BVPROC) _GLSLGetFuncProc("glVertexAttrib4bvARB");
	glVertexAttrib4d			= (PFNGLVERTEXATTRIB4DPROC) _GLSLGetFuncProc("glVertexAttrib4dARB");
	glVertexAttrib4dv			= (PFNGLVERTEXATTRIB4DVPROC) _GLSLGetFuncProc("glVertexAttrib4dvARB");
	glVertexAttrib4f			= (PFNGLVERTEXATTRIB4FPROC) _GLSLGetFuncProc("glVertexAttrib4fARB");
	glVertexAttrib4fv			= (PFNGLVERTEXATTRIB4FVPROC) _GLSLGetFuncProc("glVertexAttrib4fvARB");
	glVertexAttrib4iv			= (PFNGLVERTEXATTRIB4IVPROC) _GLSLGetFuncProc("glVertexAttrib4ivARB");
	glVertexAttrib4s			= (PFNGLVERTEXATTRIB4SPROC) _GLSLGetFuncProc("glVertexAttrib4sARB");
	glVertexAttrib4sv			= (PFNGLVERTEXATTRIB4SVPROC) _GLSLGetFuncProc("glVertexAttrib4svARB");
	glVertexAttrib4ubv			= (PFNGLVERTEXATTRIB4UBVPROC) _GLSLGetFuncProc("glVertexAttrib4ubvARB");
	glVertexAttrib4uiv			= (PFNGLVERTEXATTRIB4UIVPROC) _GLSLGetFuncProc("glVertexAttrib4uivARB");
	glVertexAttrib4usv			= (PFNGLVERTEXATTRIB4USVPROC) _GLSLGetFuncProc("glVertexAttrib4usvARB");
	glVertexAttribPointer		= (PFNGLVERTEXATTRIBPOINTERPROC) _GLSLGetFuncProc("glVertexAttribPointerARB");
#endif
}



GLuint _GLSLCreateShader(GLenum type, const char* data, bool bFromFile) {
	// load shader
	const char* shaderSource;
	if(bFromFile) {
		FILE* hf = fopen(data, "rb");
		if(!hf) {
			Log(LOG_ERROR, "Shader-File '%s' not found!", data);
			return -1;
		}
		fseek(hf, 0, SEEK_END);
		long fileSize = ftell(hf);
		fseek(hf, 0, SEEK_SET);
		shaderSource = (char*)malloc(fileSize+1);
		memset((void*)shaderSource, 0, fileSize+1);
		fread((void*)shaderSource, fileSize, 1, hf);
		fclose(hf);
		if(fileSize <= 8) {
			Log(LOG_ERROR, "Shader source in '%s' appears to be invalid (too small)!", data);
			free((void*)shaderSource);
			return -1;
		}
	} else {
		shaderSource = data;
	}
	
	// create and compile
	GLuint sh = glCreateShader(type);
	glShaderSource(sh, 1, &shaderSource, 0);
	glCompileShader(sh);
	if(bFromFile) { free((void*)shaderSource); }
	
	// error checking
	GLint shaderStatus;
	glGetShaderiv(sh, GL_COMPILE_STATUS, &shaderStatus);
	if(shaderStatus == GL_FALSE) {
		char log[2048];
		int logLength;
		glGetShaderInfoLog(sh, 2048-1, &logLength, log);
		Log(LOG_ERROR, "Shader compilation of '%s' failed! Shader Compiler message: %s", data, trim(log));
		Log(LOG_INFO, "Shader-Source:\n%s\n", shaderSource);
	}
	
	return sh;
}


GLSLSHADER* GLSLShaderCreate(const char* szVS, const char* szFS, bool bFromFile) {
	// Create data structure
	GLSLSHADER* shader = (GLSLSHADER*)malloc(sizeof(GLSLSHADER));
	shader->dwRef = 1;

	// Create vertex shader
	shader->vs = _GLSLCreateShader(GL_VERTEX_SHADER, szVS, bFromFile);
	if(shader->vs == -1) {
		Log(LOG_ERROR, "Error while loading vertex shader '%s'.", szVS);
		// TODO: Error handling
	}

	// Create fragment shader
	shader->fs = _GLSLCreateShader(GL_FRAGMENT_SHADER, szFS, bFromFile);
	if(shader->fs == -1) {
		Log(LOG_ERROR, "Error while loading fragment shader '%s'.", szFS);
		// TODO: Error handling
	}

	// Create program
	shader->prg = glCreateProgram();
	glAttachShader(shader->prg, shader->vs);
	glAttachShader(shader->prg, shader->fs);
	glLinkProgram(shader->prg);

	// check status
	GLint linkStatus;
	glGetProgramiv(shader->prg, GL_LINK_STATUS, &linkStatus);
	if(linkStatus == 0) {
		char log[2048];
		int logLength;
		glGetProgramInfoLog(shader->prg, 2048-1, &logLength, log);
		Log(LOG_ERROR, "Shader linking failed! Message: %s", trim(log));
	}

	return shader;
}


void GLSLShaderDestroy(GLSLSHADER* shader) {
	GLsizei count;
	GLuint shaders[32];
	glGetAttachedShaders(shader->prg, 32, &count, shaders);
	for(GLsizei n=0; n<count; n++) {
		glDetachShader(shader->prg, shaders[n]);
		glDeleteShader(shaders[n]);
	}
	glDeleteProgram(shader->prg);
	free(shader);
}


void GLSLSetShader(GLSLSHADER* shader) {
	g_activeShader = shader;
	if(shader) {
		glUseProgram(shader->prg);
	} else {
		glUseProgram(0);
	}
}

GLSLSHADER* GLSLGetShader() {
	return g_activeShader;
}


int GLSLGetUniform(GLSLSHADER* shader, const char* szName) {
	return glGetUniformLocation(shader->prg, szName);
}

void GLSLSetUniform1i(int uni, int f) {
	glUniform1i(uni,f);
}

void GLSLSetUniform1f(int uni, float f) {
	glUniform1f(uni,f);
}


void GLSLSetUniform2f(int uni, float f1, float f2) {
	glUniform2f(uni,f1,f2);
}


void GLSLSetUniform3f(int uni, float f1, float f2, float f3) {
	glUniform3f(uni,f1,f2,f3);
}


void GLSLSetUniform4f(int uni, float f1, float f2, float f3, float f4) {
	glUniform4f(uni,f1,f2,f3,f4);
}


void GLSLSetUniformMatrix(int uni, float* mx) {
	glUniformMatrix4fv(uni, 1, 0, (float*)mx);
}

