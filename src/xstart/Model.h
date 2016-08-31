#ifndef _MODEL_H_
#define _MODEL_H_

#include "ScriptObject.h"
#include "NodeEx.h"
#include "Texture.h"
#include <vector>

class Mesh;

void Load3D_OBJ(Mesh* mesh, const char* file);

class Mesh : public NodeEx {
public:
	Mesh() : NodeEx() {
		id = "Mesh";
		help = "3D mesh.";

		texture0 = 0;
		texture1 = 0;
		texture2 = 0;
		texture3 = 0;
		size = 0;
		capacity = 6;
		positions = (GLfloat*)malloc( sizeof(GLfloat) * 3 * capacity );
		texCoords = (GLfloat*)malloc( sizeof(GLfloat) * 3 * capacity );
		normals   = (GLfloat*)malloc( sizeof(GLfloat) * 3 * capacity );

		BindMember("texture",  &texture0, TYPE_OBJECT);
		BindMember("texture0", &texture0, TYPE_OBJECT);
		BindMember("texture1", &texture0, TYPE_OBJECT);
		BindMember("texture2", &texture0, TYPE_OBJECT);
		BindMember("texture3", &texture0, TYPE_OBJECT);
		BindFunction("LoadObj", (SCRIPT_FUNCTION)&Mesh::gm_loadObj, "[this] loadObj({string} file)");
		BindFunction("setPosition", (SCRIPT_FUNCTION)&Mesh::gm_setPosition, "[this] setPosition({int} index, {float} x, {float} y, {float} z)");
		BindFunction("setTexCoord", (SCRIPT_FUNCTION)&Mesh::gm_setTexCoord, "[this] setTexCoord({int} index, {float} u, {float} v, {float} w)");
		BindFunction("setNormal", (SCRIPT_FUNCTION)&Mesh::gm_setNormal, "[this] setNormal({int} index)");
	}
	~Mesh() {
		free(positions);
		free(texCoords);
		free(normals);
	}

	coDword _checkIndex(coDword index) {
		if(index >= size) { size = index + 1; }
		if(index >= capacity) {
			capacity = index + 1 + index / 3;
			positions = (GLfloat*)realloc(positions, sizeof(GLfloat) * 3 * capacity);
			texCoords = (GLfloat*)realloc(texCoords, sizeof(GLfloat) * 3 * capacity);
			normals   = (GLfloat*)realloc(normals,   sizeof(GLfloat) * 3 * capacity);
		}
		return capacity;
	}

	void render() {
		RenderVertices(size, positions, normals, texCoords,
		               texture0 ? texture0->texture : 0,
		               texture1 ? texture1->texture : 0,
		               texture2 ? texture2->texture : 0,
		               texture3 ? texture3->texture : 0);
		// temporarily disable associated textures for scene rendering
		if(texture0) { texture0->isRenderable = false; }
		if(texture1) { texture1->isRenderable = false; }
		if(texture2) { texture2->isRenderable = false; }
		if(texture3) { texture3->isRenderable = false; }
		RenderChilds();
		if(texture0) { texture0->isRenderable = true; }
		if(texture1) { texture1->isRenderable = true; }
		if(texture2) { texture2->isRenderable = true; }
		if(texture3) { texture3->isRenderable = true; }
	}

	bool loadObj(const char* file) {
		Load3D_OBJ(this, file);
	}
	int gm_loadObj(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		loadObj(file);
		return ReturnThis(a_thread);
	}

	void setPosition(int index, float x, float y, float z) {
		_checkIndex(index);
		positions[index * 3] = x;
		positions[index * 3 + 1] = y;
		positions[index * 3 + 2] = z;
	}
	int gm_setPosition(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(index, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(z, 3);
		setPosition(index, x, y, z);
		return this->ReturnThis(a_thread);
	}

	void setTexCoord(int index, float u, float v, float w) {
		_checkIndex(index);
		texCoords[index * 3] = u;
		texCoords[index * 3 + 1] = v;
		texCoords[index * 3 + 2] = w;
	}
	int gm_setTexCoord(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(index, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(u, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(v, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(w, 3);
		setTexCoord(index, u, v, w);
		return this->ReturnThis(a_thread);
	}

	void setNormal(int index, float x, float y, float z) {
		_checkIndex(index);
		normals[index * 3] = x;
		normals[index * 3 + 1] = y;
		normals[index * 3 + 2] = z;
	}
	int gm_setNormal(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(index, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(z, 3);
		setNormal(index, x, y, z);
		return this->ReturnThis(a_thread);
	}

public:
	GLfloat* positions;
	GLfloat* texCoords;
	GLfloat* normals;
	coDword size;
	coDword capacity;
	Texture* texture0;
	Texture* texture1;
	Texture* texture2;
	Texture* texture3;
};

#endif
