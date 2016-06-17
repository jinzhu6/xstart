#include "corela.h"
#include "glutils.h"
#ifndef _WIN32
#define GL_GLEXT_PROTOTYPES
#endif
#include "glext.h"

#define PI 3.14159265f

#define USE_MULTITEX 0

#ifdef _WIN32
PFNGLACTIVETEXTUREARBPROC glActiveTexture = 0;
#endif

void RenderVertices(coDword vertexCount, GLfloat* positions, GLfloat* normals, GLfloat* texCoords, TEXTURE* tx0, TEXTURE* tx1, TEXTURE* tx2, TEXTURE* tx3) {
	BEGIN_CHECK_GL_ERROR();

#ifdef _WIN32
	if(!glActiveTexture) { glActiveTexture = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress((LPCSTR)"glActiveTextureARB"); }
#endif

	// bind textures
	if(tx0) {
#if USE_MULTITEX == 1
		glActiveTexture(GL_TEXTURE0_ARB);
#endif
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tx0->handle);
	}

#if USE_MULTITEX == 1
    if(tx1) {
        glActiveTexture(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tx1->handle);
        glActiveTexture(GL_TEXTURE0_ARB);
    }
    if(tx2) {
        glActiveTexture(GL_TEXTURE2_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tx2->handle);
        glActiveTexture(GL_TEXTURE0_ARB);
    }
    if(tx3) {
        glActiveTexture(GL_TEXTURE3_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tx3->handle);
        glActiveTexture(GL_TEXTURE0_ARB);
    }
#endif

	// set arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, positions);

	if(normals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normals);
	}
	else {
		glNormal3f(0.0, 0.0, 1.0);
	}
	if(texCoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, 0, texCoords);
	}

	// draw vertex array
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	// disable texture units
#if USE_MULTITEX == 1
	glActiveTexture(GL_TEXTURE0_ARB);
#endif
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

#if USE_MULTITEX == 1
    glActiveTexture(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2_ARB);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3_ARB);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0_ARB);
#endif

	END_CHECK_GL_ERROR("RenderVertices");
}

void RenderTextureQuad(TEXTURE* tx, double px, double py, double cx, double cy, double w, double h, TEXTURE_RECT* rc, float rotate, float scale, float alpha, bool flipX, bool flipY) {
	if(!tx) { return; }
	if(!rc) { rc = &tx->drawRect; }
	if(w==0) { w = tx->vwidth; }
	if(h==0) { h = tx->vheight; }

	px = py = 0.0;
//	cx = cy = 0.0;

	// vertices
	GLfloat vpos[] = {
		-cx,          -cy,			0.0,
		w-cx,         -cy,			0.0,
		w-cx,         h-cy,			0.0,
		w-cx,         h-cy,			0.0,
		-cx,          h-cy,			0.0,
		-cx,          -cy,			0.0,
	};
	GLfloat vtex[] = {
		rc->left,  rc->top, 0.0,
		rc->right, rc->top, 0.0,
		rc->right, rc->bottom, 0.0,
		rc->right, rc->bottom, 0.0,
		rc->left,  rc->bottom, 0.0,
		rc->left,  rc->top, 0.0,
	};

	RenderVertices(6, vpos, 0, vtex, tx, 0, 0, 0);

	// set color and alpha
/*	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_EQUAL);
	glColor4f(1.0,1.0,1.0,alpha);

	// draw vertex array
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tx->handle);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, vpos);
	glTexCoordPointer(2, GL_DOUBLE, 0, vtex);
	glDrawArrays(GL_QUADS, 0, 4);

	// reset states and matrices
	glColor4f(1.0,1.0,1.0,1.0);*/
}

void RenderMultiTextureQuad(TEXTURE* tx0, TEXTURE* tx1, double px, double py, double cx, double cy, double w, double h, TEXTURE_RECT* rc, float rotate, float scale, float alpha, bool flipX, bool flipY) {
#ifdef _WIN32
	if(!glActiveTexture) { glActiveTexture = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress((LPCSTR)"glActiveTextureARB"); }
#endif

	if(!tx0 || !tx1) { return; }
	if(!rc) { rc = &tx0->drawRect; }
	if(w==0) { w = tx0->vwidth; }
	if(h==0) { h = tx0->vheight; }

	px = py = 0.0;
//	cx = cy = 0.0;

	GLdouble vpos[] = {
		-cx,          -cy,		0.0,
		w-cx,         -cy,		0.0,
		w-cx,         h-cy,		0.0,
		-cx,          h-cy,		0.0
	};
	GLdouble vtex[] = {
		rc->left,  rc->top,
		rc->right, rc->top,
		rc->right, rc->bottom,
		rc->left,  rc->bottom,
	};

	// set color and alpha
	glColor4f(1.0,1.0,1.0,alpha);

	// bind first texture
	glActiveTexture(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tx0->handle);

	// bind second texture
	if(tx1) {
		glActiveTexture(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tx1->handle);
	}

	// draw vertex array
	glActiveTexture(GL_TEXTURE0_ARB);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, vpos);
	glTexCoordPointer(2, GL_DOUBLE, 0, vtex);
	glDrawArrays(GL_QUADS, 0, 4);

	// reset states and matrices
	glColor4f(1.0,1.0,1.0,1.0);

	// disable second texture unit
	if(tx1) {
		glActiveTexture(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// set active texture channel to 0
	glActiveTexture(GL_TEXTURE0_ARB);
}
