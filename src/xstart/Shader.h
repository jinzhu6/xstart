#ifndef _SHADER_H_
#define _SHADER_H_

#include "Node.h"


class Shader : public Node {
public:

	Shader() : Node() {
		id = "Shader";
		ctor = "({string} vertexShaderFile, {string} fragmentShaderFile)";
		help = "GLSL shader node class. All childs of this node are rendered with this shader. If @active is set to zero all shaders are disabled at this point.";

		active = false;
		shader = 0;

		BindMember("active", &active, TYPE_INT, 0, "{int} active", "Sets if the childs are rendered with or without this shader.");
		BindFunction("load", (SCRIPT_FUNCTION)&Shader::gm_load, "{int} load({string} vertexShaderFile, {string} fragmentShaderFile), (optional) {int} fromFiles", "Loads the vertex and fragment shaders from the given files or directly from the strings (if the third parameter is set to zero). Returns non-zero on success.");
		BindFunction("setFloat", (SCRIPT_FUNCTION)&Shader::gm_setFloat, "[this] setFloat({float} x)", "Sets float uniform in shader.");
		BindFunction("setFloat2", (SCRIPT_FUNCTION)&Shader::gm_setFloat2, "[this] setFloat2({float} x, {float} y)", "Sets float vec2 uniform in shader.");
		BindFunction("setFloat3", (SCRIPT_FUNCTION)&Shader::gm_setFloat3, "[this] setFloat3({float} x, {float} y, float z)", "Sets float vec3 uniform in shader.");
		BindFunction("setFloat4", (SCRIPT_FUNCTION)&Shader::gm_setFloat4, "[this] setFloat4({float} x, {float} y, float z, float w)", "Sets float vec4 uniform in shader.");
		BindFunction("setInt", (SCRIPT_FUNCTION)&Shader::gm_setInt, "[this] setInt({int} n)", "Sets integer uniform in shader.");
	}

	~Shader() {
		if(shader) {
			GLSLShaderDestroy(shader);
		}
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() == 2) {
			GM_CHECK_STRING_PARAM(vs, 0);
			GM_CHECK_STRING_PARAM(fs, 1);
			load(vs,fs,true);
		}
		return GM_OK;
	}

	bool load(const char* vs, const char* fs, int fromFiles) {
		if(shader) { GLSLShaderDestroy(shader); }
		if(fromFiles) { shader = GLSLShaderCreate(_FILE(vs), _FILE(fs), fromFiles); }
		else { shader = GLSLShaderCreate(vs, fs, fromFiles); }
		if(shader) { active = true; return true; }
		else { active = false; return false; }
	}
	int gm_load(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(vs, 0);
		GM_CHECK_STRING_PARAM(fs, 1);

		int fromFiles = 1;
		int numParams = a_thread->GetNumParams();
		if(numParams >= 3) {
			GM_CHECK_INT_PARAM(fromFiles, 2);
		}

		a_thread->PushInt(load(vs,fs,fromFiles));
		return GM_OK;
	}



	void render() {
		GLSLSHADER* prevShader = GLSLGetShader();
		if(shader && active) { GLSLSetShader(shader); }
		else { GLSLSetShader(0); }
		RenderChilds();
		GLSLSetShader(prevShader);
	}

	void setFloat(const char* uniform, float value) {
		GLSLSHADER* prevShader = GLSLGetShader();
		if(shader) { GLSLSetShader(shader); }

		int uid = GLSLGetUniform(shader, uniform);
		if(uid == -1) {
			Log(LOG_ERROR, "GLSL Uniform '%s' not found in shader!", uniform);
			return;
		}
		GLSLSetUniform1f(uid, value);

		GLSLSetShader(prevShader);
	}
	int gm_setFloat(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(uniform, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(value, 1);
		setFloat(uniform, value);
		return ReturnThis(a_thread);
	}


	void setFloat2(const char* uniform, float value1, float value2) {
		GLSLSHADER* prevShader = GLSLGetShader();
		if(shader) { GLSLSetShader(shader); }

		int uid = GLSLGetUniform(shader, uniform);
		if(uid == -1) {
			Log(LOG_ERROR, "GLSL Uniform '%s' not found in shader!", uniform);
			return;
		}
		GLSLSetUniform2f(uid, value1, value2);

		GLSLSetShader(prevShader);
	}
	int gm_setFloat2(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_STRING_PARAM(uniform, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(value1, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(value2, 2);
		setFloat2(uniform, value1, value2);
		return ReturnThis(a_thread);
	}


	void setFloat3(const char* uniform, float value1, float value2, float value3) {
		GLSLSHADER* prevShader = GLSLGetShader();
		if(shader) { GLSLSetShader(shader); }

		int uid = GLSLGetUniform(shader, uniform);
		if(uid == -1) {
			Log(LOG_ERROR, "GLSL Uniform '%s' not found in shader!", uniform);
			return;
		}
		GLSLSetUniform3f(uid, value1, value2, value3);

		GLSLSetShader(prevShader);
	}
	int gm_setFloat3(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_STRING_PARAM(uniform, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(value1, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(value2, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(value3, 3);
		setFloat3(uniform, value1, value2, value3);
		return ReturnThis(a_thread);
	}


	void setFloat4(const char* uniform, float value1, float value2, float value3, float value4) {
		GLSLSHADER* prevShader = GLSLGetShader();
		if(shader) { GLSLSetShader(shader); }

		int uid = GLSLGetUniform(shader, uniform);
		if(uid == -1) {
			Log(LOG_ERROR, "GLSL Uniform '%s' not found in shader!", uniform);
			return;
		}
		GLSLSetUniform4f(uid, value1, value2, value3, value4);

		GLSLSetShader(prevShader);
	}
	int gm_setFloat4(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(5);
		GM_CHECK_STRING_PARAM(uniform, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(value1, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(value2, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(value3, 3);
		GM_CHECK_FLOAT_OR_INT_PARAM(value4, 4);
		setFloat4(uniform, value1, value2, value3, value4);
		return ReturnThis(a_thread);
	}


	void setInt(const char* uniform, int value) {
		GLSLSHADER* prevShader = GLSLGetShader();
		if(shader) { GLSLSetShader(shader); }

		int uid = GLSLGetUniform(shader, uniform);
		if(uid == -1) {
			Log(LOG_ERROR, "GLSL Uniform '%s' not found in shader!", uniform);
			return;
		}
		GLSLSetUniform1i(uid, value);

		GLSLSetShader(prevShader);
	}
	int gm_setInt(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(uniform, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(value, 1);
		setInt(uniform, (int)value);
		return ReturnThis(a_thread);
	}


public:
	int active;
	GLSLSHADER* shader;
};


#endif
