#ifndef _NODE_EX_H_
#define _NODE_EX_H_

#include "Shader.h"
#include "Color.h"

extern FRAME* g_FRAME;


class NodeEx : public Node {
public:

	NodeEx() : Node() {
		id = "NodeEx";
		help = "Generic extended scene node.";
		ctor = "((optional) {string} id, (optional) {float} pos_x, (optional) {float} pos_y, (optional) {float} width, (optional) {float} height)";

		useShader = 1;
		shader = new Shader();
		color = new Color();
		color->fromStringRGBA("#FFFFFFFF");

		BindMember("shader", &shader, TYPE_OBJECT, 0, "[Shader] shader", "<b>Experimental.</b> A shader used for rendering this node and its childs.");
		BindMember("useShader", &useShader, TYPE_INT, 0, "{int} useShader", "<b>Experimental.</b> Enable/Disable shader on node level.");
		BindMember("color", &color, TYPE_OBJECT, 0, "[Color] color", "<b>Experimental.</b> The color used while rendering.");
	}

	~NodeEx() {
	}

	virtual bool RenderChilds() {
		GLSLSHADER* prevShader;
		int n=0;

		// iterate through childs
		while(Node* c = GetChild(n++)) {

			// preserve current shader
			if(useShader && shader->active && shader->shader) {
				prevShader = GLSLGetShader();
				GLSLSetShader(shader->shader);
			}

			// render
			if(c->isRenderable) {
				c->ValidateAndRender();
			}

			// restore previous shader
			if(useShader && shader->active && shader->shader) {
				GLSLSetShader(prevShader);
			}
		}
		return true;
	}

	virtual void ValidateAndRender() {
		// checks
		if(!visible) { return; }
		if(!valid) { update(); }

		// apply transformations
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
//		glTranslatef(position->x - pivot->x, position->y - pivot->y, position->z - pivot->z);
		glTranslatef(position->x, position->y, position->z);
		glRotatef(rotation->x*(180.0f/3.14159265f), 1.0, 0.0, 0.0);
		glRotatef(rotation->y*(180.0f/3.14159265f), 0.0, 1.0, 0.0);
		glRotatef(rotation->z*(180.0f/3.14159265f), 0.0, 0.0, 1.0);
		glScalef(scaling->x * scaling->z, scaling->y * scaling->z, 1.0);
		// TODO: Scaling goes here!
		
		// enable scissor clipping
		if(clipLeft >= 0.0) {
			glEnable(GL_SCISSOR_TEST);
			FrameSetScissor((FRAME*)g_FRAME, clipLeft, clipTop, clipRight, clipBottom);
		}

		// enable shader, if set
		GLSLSHADER* prevShader;
		if(useShader && shader->active && shader->shader) {
			prevShader = GLSLGetShader();
			GLSLSetShader(shader->shader);
		}

		// use color
		if(this->color)	{
			glColor4f(this->color->r, this->color->g, this->color->b, this->color->a);
		}

		// render node (and childs)
		render();

		// reset color
		glColor4f(1.0, 1.0, 1.0, 1.0);

		// disable scissor clipping
		if(clipLeft >= 0.0) {
			glDisable(GL_SCISSOR_TEST);
		}

		// restore matrix
		glPopMatrix();

		// restore shader
		if(useShader && shader->active && shader->shader) {
			GLSLSetShader(prevShader);
		}
	}

public:
	int useShader;
	Shader* shader;
	Color* color;
};


#endif
