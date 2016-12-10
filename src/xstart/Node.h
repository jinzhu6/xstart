#ifndef _SCENE_NODE_H_
#define _SCENE_NODE_H_

#include <corela.h>
#include "Event.h"
#include "Vector.h"
#include "Array.h"

extern FRAME* g_FRAME;


class Node : public Handler {
public:

	Node() : Handler() {
		id = "Node";
		help = "Empty scene node.";
		ctor = "((optional) {string} id, (optional) {float} pos_x, (optional) {float} pos_y, (optional) {float} width, (optional) {float} height)";
		isRenderable = true;

		position = new Vector();
		pivot = new Vector();
		dimension = new Vector();
		rotation = new Vector();
		scaling = new Vector();
		scaling->set(1.0, 1.0, 1.0);
		valid = false;
		visible = true;
		clipLeft = clipTop = clipRight = clipBottom = -1.0f;
		hitPrev = false;

		BindMember("visible", &visible, TYPE_INT, 0, "{int} visible", "Visibility of the node and its child.");
		BindMember("position", &position, TYPE_OBJECT, 0, "[Vector] position", "Position of the node on the frame.");
		BindMember("pivot", &pivot, TYPE_OBJECT, 0, "[Vector] pivot", "Relative pivot/anchor of the node for positioning, rotation, etc.");
		BindMember("dimension", &dimension, TYPE_OBJECT, 0, "[Vector] dimension", "Rendering dimensions.");
		BindMember("rotation", &rotation, TYPE_OBJECT, 0, "[Vector] rotation", "Rotation around the @center .");
		BindMember("scaling", &scaling, TYPE_OBJECT, 0, "[Vector] scaling", "Scaling of the node.");
		BindMember("scale", &scaling->z, TYPE_FLOAT, 0, "{float} scale", "Alias to @scaling .z");
		BindMember("x", &position->x, TYPE_FLOAT, 0, "{float} x", "Alias to @position .x");
		BindMember("y", &position->y, TYPE_FLOAT, 0, "{float} y", "Alias to @position .y");
		BindMember("z", &position->z, TYPE_FLOAT, 0, "{float} z", "Alias to @position .z");
		BindMember("width", &dimension->x,  TYPE_FLOAT, 0, "{float} width", "Alias to @dimension .x");
		BindMember("height", &dimension->y, TYPE_FLOAT, 0, "{float} height", "Alias to @dimension .y");
		BindMember("depth", &dimension->z,  TYPE_FLOAT, 0, "{float} depth", "Alias to @dimension .z");
		BindMember("rotate", &rotation->z, TYPE_FLOAT, 0, "{float} rotate", "Alias to @rotation .z");

		BindFunction("update", (SCRIPT_FUNCTION)&Node::gm_update, "[this] update()", "Force this node to update its contents and states. May be needed if the node is changed by the script after initialization.");
		BindFunction("render", (SCRIPT_FUNCTION)&Node::gm_render, "[this] render()", "Renders the node and all its childs inside the selected frame.");
		BindFunction("hit", (SCRIPT_FUNCTION)&Node::gm_hit, "{int} hit({float} x, {float} y)", "Does a hit-test on the node, returns 1 if the coordinates hit the node on the screen.");
		BindFunction("setClipRegion", (SCRIPT_FUNCTION)&Node::gm_setClipRegion, "[this] setClipRegion({float} left, {float} top, {float} right, {float} bottom)", "Sets the scissor clip region, everything inside will clip to that region.");
		BindFunction("getChild", (SCRIPT_FUNCTION)&Node::gm_getChild, "[Node] getChild({int} index)");
		//BindFunction("trigger", (SCRIPT_FUNCTION)&Node::gm_trigger, "");

		BindFunction("left", (SCRIPT_FUNCTION)&Node::gm_left, "[this] left()", "Aligns the left of the node to the pivot.");
		BindFunction("center", (SCRIPT_FUNCTION)&Node::gm_center, "[this] center()", "Aligns the horizontal center of the node to the pivot.");
		BindFunction("right", (SCRIPT_FUNCTION)&Node::gm_right, "[this] right()", "Aligns the right of the node to the pivot.");
		BindFunction("top", (SCRIPT_FUNCTION)&Node::gm_top, "[this] top()", "Aligns the top of the node to the pivot.");
		BindFunction("middle", (SCRIPT_FUNCTION)&Node::gm_middle, "[this] middle()", "Aligns the vertical center of the node to the pivot.");
		BindFunction("bottom", (SCRIPT_FUNCTION)&Node::gm_bottom, "[this] bottom()", "Aligns the bottom of the node to the pivot.");
	}

	virtual int Initialize(gmThread* a_thread) {
		gmVariable varNull;
		varNull.Nullify();

		if(a_thread->GetNumParams() >= 1) {
			gmVariable var = a_thread->Param(0, varNull);
			if(!var.IsNull()) {
				id = var.GetCStringSafe();
			}
		}
		if(a_thread->GetNumParams() >= 2) {
			GM_CHECK_FLOAT_OR_INT_PARAM(px, 1);
			position->x = px;
		}
		if(a_thread->GetNumParams() >= 3) {
			GM_CHECK_FLOAT_OR_INT_PARAM(py, 2);
			position->y = py;
		}
		if(a_thread->GetNumParams() >= 4) {
			GM_CHECK_FLOAT_OR_INT_PARAM(dx, 3);
			dimension->x = dx;
		}
		if(a_thread->GetNumParams() >= 5) {
			GM_CHECK_FLOAT_OR_INT_PARAM(dy, 4);
			dimension->y = dy;
		}
		return GM_OK;
	}

	void _EraseExisting(std::string key) {
		// erase existing child with given key
		std::vector<std::string>::iterator i;
		for(i = childOrder.begin(); i != childOrder.end(); i++) {
			if(*i == key) {
				childOrder.erase(i);
				return;
			}
		}
	}

	virtual void SetDot(const char* key, gmVariable &var) {
		_EraseExisting(key);
		if(!var.IsNull()) {
			if(var.m_type == GM_TYPE_OBJECT) {
				// TODO: Check if this is actually a Node!
				childOrder.push_back(key);
			}
		}
	}

	virtual Node* GetChild(int n) {
		while(n < 0) { n += childOrder.size(); }
		if(n >= childOrder.size()) {
			return 0;
		}
		gmVariable var = table->Get(machine, childOrder[n].c_str());
		Node* child = (Node*)var.GetUserSafe(GM_TYPE_OBJECT);
		return child;
	}

	virtual bool HandleEventChilds(Event* e) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(position->x - pivot->x, position->y - pivot->y, position->z - pivot->z);

		int n = childOrder.size() - 1;
		while(n >= 0) {
			Node* c = GetChild(n--);
			if(c->isEventHandler && c->visible) {
				if(c->HandleEvent(e)) {
					glPopMatrix();
					return true;
				}
			}
		}

		glPopMatrix();
		return false;
	}

	virtual bool HandleEvent(Event* e) {
		GLfloat mx[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mx);

		if(!this->visible) {
			return false;
		}

		bool hit = this->hit(e->x - mx[12], e->y - mx[13]);
		//bool hitPrev = this->hit(e->prevX - mx[12], e->prevY - mx[13]);

		if (hit && !hitPrev) e->id = "MouseEnter";
		if (!hit && hitPrev) e->id = "MouseLeave";

		if(e->id == "MouseMove" || e->id == "MouseDown" || e->id == "MouseUp") {
			if(hit) {
				if(::HandleEvent(e, this)) { return true; }
			}
		} else {
			if(::HandleEvent(e, this)) { return true; }
		}

		hitPrev = hit;
		
		return HandleEventChilds(e);
	}

	virtual bool RenderChilds() {
		int n=0;
		while(Node* c = GetChild(n++)) {
			if(c->isRenderable) {
				c->ValidateAndRender();
			}
		}
		return true;
	}

	virtual void update() {
		//RaiseEvent(EVENT_UPDATE, this);
		validate();
	}
	int gm_update(gmThread* a_thread) {
		update();
		return ReturnThis(a_thread);
	}

	virtual void invalidate() {
		valid = false;
	}

	virtual void validate() {
		valid = true;
	}

	virtual void render() {
		//RaiseEvent(EVENT_RENDER, this);
		RenderChilds();
	}
	int gm_render(gmThread* a_thread) {
		ValidateAndRender();
		return ReturnThis(a_thread);
	}

	virtual void ValidateAndRender() {
		// checks
		if(!visible) {
			return;
		}
		if(!valid) {
			update();
		}


#if 0  // OLD CODE
		// apply transformations
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(position->x + pivot->x * 2.0, position->y + pivot->y, position->z + pivot->z * 2.0);
		glScalef(scaling->x * scaling->z, scaling->y * scaling->z, 1.0); // TODO: Bad Idea for 3D
//		glTranslatef(position->x + center->x, position->y + center->y, position->z + center->z);
//		glRotatef(rotation->x*(180.0f/3.14159265f), 1.0, 0.0, 0.0);
//		glRotatef(rotation->y*(180.0f/3.14159265f), 0.0, 1.0, 0.0);
//		glRotatef(rotation->z*(180.0f/3.14159265f), 0.0, 0.0, 1.0);
#else  // CODE FROM NodeEx
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(position->x, position->y, position->z);
		glScalef(scaling->x * scaling->z, scaling->y * scaling->z, 1.0);
		glRotatef(rotation->x*(180.0f/3.14159265f), 1.0, 0.0, 0.0);
		glRotatef(rotation->y*(180.0f/3.14159265f), 0.0, 1.0, 0.0);
		glRotatef(rotation->z*(180.0f/3.14159265f), 0.0, 0.0, 1.0);
		glTranslatef(-pivot->x, -pivot->y, -pivot->z);
#endif
		// enable scissor clipping
		if(clipLeft >= 0.0) {
			glEnable(GL_SCISSOR_TEST);
			FrameSetScissor((FRAME*)g_FRAME, (int)clipLeft, (int)clipTop, (int)clipRight, (int)clipBottom);
			//glScissor(clipLeft, 1080 - clipBottom, clipRight - clipLeft, clipBottom - clipTop);
		}

		// render node (and childs)
		render();

		// disable scissor clipping
		if(clipLeft >= 0.0) {
			glDisable(GL_SCISSOR_TEST);
		}

		// restore matrix
		glPopMatrix();
	}

	virtual bool hit(float x, float y) {
		// TODO: Need proper 3D picking!
		if(!visible) { return false; }
		float minX, maxX, minY, maxY;
		minX = position->x - pivot->x;
		maxX = position->x - pivot->x + dimension->x;
		minY = position->y - pivot->y;
		maxY = position->y - pivot->y + dimension->y;
		if(x >= minX && x <= maxX && y >= minY && y <= maxY) {
			return true;
		}
		return false;
	}
	int gm_hit(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(testX, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(testY, 1);
		a_thread->PushInt( hit(testX, testY) );
		return GM_OK;
	}

	void setClipRegion(float left, float top, float right, float bottom) {
		clipLeft = left;
		clipTop = top;
		clipRight = right;
		clipBottom = bottom;
	}
	int gm_setClipRegion(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_FLOAT_OR_INT_PARAM(left, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(top, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(right, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(bottom, 3);
		setClipRegion(left, top, right, bottom);
		return ReturnThis(a_thread);
	}

	int gm_getChild(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(index, 0);
		Node* node = GetChild(index);
		if(node) {
			return node->ReturnThis(a_thread);
		}
		return ReturnNull(a_thread);
	}

	void centerAll() {
		this->pivot->x = (gmfloat)(this->dimension->x * 0.5);
		this->pivot->y = (gmfloat)(this->dimension->y * 0.5);
		this->pivot->z = (gmfloat)(this->dimension->z * 0.5);
	}

	void left(float off) {
		this->pivot->x = (gmfloat)(0.0 - off);
	}
	int gm_left(gmThread* a_thread) {
		gmfloat off=0.0;
		a_thread->ParamFloatOrInt(0, off, 0.0);
		left(off);
		return ReturnThis(a_thread);
	}

	void center(float off) {
		this->pivot->x = (gmfloat)(this->dimension->x * 0.5 - off);
	}
	int gm_center(gmThread* a_thread) {
		gmfloat off=0.0;
		a_thread->ParamFloatOrInt(0, off, 0.0);
//		update();
		center(off);
		return ReturnThis(a_thread);
	}

	void right(float off) {
		this->pivot->x = this->dimension->x + off;
	}
	int gm_right(gmThread* a_thread) {
		gmfloat off=0.0;
		a_thread->ParamFloatOrInt(0, off, 0.0);
//		update();
		right(off);
		return ReturnThis(a_thread);
	}

	void top(float off) {
		this->pivot->y = (gmfloat)(0.0 - off);
	}
	int gm_top(gmThread* a_thread) {
		gmfloat off=0.0;
		a_thread->ParamFloatOrInt(0, off, 0.0);
		top(off);
		return ReturnThis(a_thread);
	}

	void middle(float off) {
		this->pivot->y = (gmfloat)(this->dimension->y * 0.5 - off);
	}
	int gm_middle(gmThread* a_thread) {
		gmfloat off=0.0;
		a_thread->ParamFloatOrInt(0, off, 0.0);
//		update();
		middle(off);
		return ReturnThis(a_thread);
	}

	void bottom(float off) {
		this->pivot->y = this->dimension->y + off;
	}
	int gm_bottom(gmThread* a_thread) {
		gmfloat off=0.0;
		a_thread->ParamFloatOrInt(0, off, 0.0);
//		update();
		bottom(off);
		return ReturnThis(a_thread);
	}

public:
	int visible;
	bool valid;
	bool hitPrev;
	Vector* position;
	Vector* pivot;
	Vector* dimension;
	Vector* rotation;
	Vector* scaling;
	float clipLeft, clipTop, clipRight, clipBottom;
	std::vector<std::string> childOrder;
};


#endif
