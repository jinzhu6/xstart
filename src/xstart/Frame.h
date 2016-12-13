#ifndef _FRAME_H_
#define _FRAME_H_

#include <corela.h>
#include <glutils.h>
#include "ScriptObject.h"
#include "Node.h"
#include "Event.h"

extern FRAME* g_FRAME;
extern int MachineDestroy();

class Frame : public Handler {
public:

	Frame() : Handler() {
		id = "Frame";
		help = "Window frame for rendering output. The frame uses a scene-graph to draw its content, see @root member.";
		ctor = "({string} id, {int} pos_x, {int} pos_y, {int} width, {int} height, (optional) {string} color)";
		deferedGC = true;

		internalHandle = 0;
		width = 0.0;
		height = 0.0;
		mirrorX = false;
		mirrorY = false;
		clearRGBA[0] = 1.0;
		clearRGBA[1] = 0.0;
		clearRGBA[2] = 1.0;
		clearRGBA[3] = 0.0;
		root = new Node();

		BindMember("root", &root, TYPE_OBJECT, 0, "[Node] root", "Scene graph node, initially set to an empty Node object, see [Node].");
		BindMember("width", &width, TYPE_FLOAT, 0, "{float} width", "(Experimental) Virtual frame width.");
		BindMember("height", &height, TYPE_FLOAT, 0, "{float} height", "(Experimental) Virtual frame height.");
		BindMember("mirrorX", &mirrorX, TYPE_INT, 0, "{int} mirrorX", "(Experimental) Set to non-zero Mirror screen horizontal.");
		BindMember("mirrorY", &mirrorY, TYPE_INT, 0, "{int} mirrorY", "(Experimental) Set to non-zero Mirror screen vertically.");

		BindFunction("_open", (SCRIPT_FUNCTION)&Frame::gm_open, "[this] _open({int} pos_x, {int} pos_y, {int} width, {int} height, (optional) {string} color)", "Opens the frame on the given coordinates.");
		BindFunction("close", (SCRIPT_FUNCTION)&Frame::gm_close, "[this] close()", "Closes the window. Please note that any textures, shaders and other assets may be internally destroyed if no other window has a valid context on these.");
		BindFunction("render", (SCRIPT_FUNCTION)&Frame::gm_render, "{bool} render()", "Immediately renders the frames scene graph.");
		BindFunction("toggle", (SCRIPT_FUNCTION)&Frame::gm_toggle, "[this] toggle()", "Toggles the frame from/to frameless mode.");
		BindFunction("cursor", (SCRIPT_FUNCTION)&Frame::gm_showCursor, "[this] cursor({int} show)", "Shows or hides the cursor when its over the frame window.");
		BindFunction("setDimensions", (SCRIPT_FUNCTION)&Frame::gm_setDimensions, "[this] setDimensions({int} width, {int} height)", "(Experimental) Sets the virtual dimensions of the frame. If not set, the frames dimensions is used instead.");
		BindFunction("flip", (SCRIPT_FUNCTION)&Frame::gm_flip);
		BindFunction("select", (SCRIPT_FUNCTION)&Frame::gm_select);
		BindFunction("clear", (SCRIPT_FUNCTION)&Frame::gm_clear);
		BindFunction("maximize", (SCRIPT_FUNCTION)&Frame::gm_maximize);
		BindFunction("minimize", (SCRIPT_FUNCTION)&Frame::gm_minimize);
		BindFunction("show", (SCRIPT_FUNCTION)&Frame::gm_show);
		//BindFunction("mirror", (SCRIPT_FUNCTION)&Frame::gm_mirror); // use "mirrorX" and "mirrorY"
#ifdef _WIN32
		BindFunction("activate", (SCRIPT_FUNCTION)&Frame::gm_activate);
		BindFunction("getMultitouchCount", (SCRIPT_FUNCTION)&Frame::gm_getMultitouchCount);
		BindFunction("getMultitouchPoint", (SCRIPT_FUNCTION)&Frame::gm_getMultitouchPoint);
#endif

		BindFunction("onClose", (SCRIPT_FUNCTION)&Frame::gm_exit);
	}

	~Frame() {
		close();
	}

	int Initialize(gmThread* a_thread) {
		gmVariable varNull;
		varNull.Nullify();

		if(a_thread->GetNumParams() >= 1) {
			gmVariable var = a_thread->Param(0, varNull);
			if(!var.IsNull()) {
				id = var.GetCStringSafe();
			}
		}

		gmfloat x = 0;
		if(a_thread->GetNumParams() >= 2) {
			GM_CHECK_FLOAT_OR_INT_PARAM(px, 1);
			x = px;
		}

		gmfloat y = 0;
		if(a_thread->GetNumParams() >= 3) {
			GM_CHECK_FLOAT_OR_INT_PARAM(py, 2);
			y = py;
		}

		gmfloat dim_x = 0;
		if(a_thread->GetNumParams() >= 4) {
			a_thread->ParamFloatOrInt(3, dim_x, 0.0);
		}

		gmfloat dim_y = 0;
		if(a_thread->GetNumParams() >= 5) {
			a_thread->ParamFloatOrInt(4, dim_y, 0.0);
		}

		const char* color = 0;
		if(a_thread->GetNumParams() >= 6) {
			a_thread->ParamString(5, color, "#ffffff");
		}

		if(a_thread->GetNumParams() >= 4) {
			open((int)x, (int)y, (int)dim_x, (int)dim_y, color);
		}

		return GM_OK;
	}

	virtual bool HandleEvent(Event* e) {
		if(mirrorX) { e->x = -(e->x - width); }
		if(mirrorY) { e->y = -(e->y - height); }
		if(!::HandleEvent(e, this)) {
			if(root) {
				return root->HandleEvent(e);
			}
			return false;
		}
		return true;
	}

	void* open(int x, int y, int w, int h, const char* color = "#ff00ff00") {
		if(internalHandle) {
			close();
		}
		internalHandle = FrameCreate(this->id.c_str(), OnEventGlobal, this, x==-1?0x80000000:x, y==-1?0x80000000:y, w==-1?0x80000000:w, h==-1?0x80000000:h);
		if(color) {
			ColorParse(color, &clearRGBA[0], &clearRGBA[1], &clearRGBA[2], &clearRGBA[3]);
		}
		this->width = (float)w;
		this->height = (float)h;
		return internalHandle;
	}
	int gm_open(gmThread* a_thread) {
		const char* color = 0;
		int nump = a_thread->GetNumParams();
		int x=-1, y=-1, w=-1, h=-1;
		if(nump >= 1) {
			a_thread->ParamInt(0, x, 0);
		}
		if(nump >= 2) {
			a_thread->ParamInt(1, y, 0);
		}
		if(nump >= 3) {
			a_thread->ParamInt(2, w, 0);
		}
		if(nump >= 4) {
			a_thread->ParamInt(3, h, 0);
		}
		if(nump >= 5) {
			color = a_thread->ParamString(4, "");
		}
		open(x, y, w, h, color);
		toggle();
		return ReturnThis(a_thread);
	}

	void close() {
		if(internalHandle) {
			FrameClose((FRAME*)internalHandle);
			internalHandle = 0;
		}
	}
	int gm_close(gmThread* a_thread) {
		close();
		return ReturnThis(a_thread);
	}

	void select() {
		FrameSelect((FRAME*)internalHandle);
		g_FRAME = (FRAME*)this->internalHandle;
//		((FRAME*)internalHandle)->vwidth = this->width;
//		((FRAME*)internalHandle)->vheight = this->height;
//		glUtilsTo2D(0.0, 0.0, ((FRAME*)internalHandle)->vwidth, ((FRAME*)internalHandle)->vheight, 0.0, 1.0);

		double left = 0.0;
		double top = 0.0;
		double right = ((FRAME*)internalHandle)->vwidth;
		double bottom = ((FRAME*)internalHandle)->vheight;
		double tmp;
		if(mirrorX) { tmp = left; left = right;  right = tmp; }
		if(mirrorY) { tmp = top;  top = bottom;  bottom = tmp; }
		glUtilsTo2D(left, top, right, bottom, 0.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	int gm_select(gmThread* a_thread) {
		select();
		return GM_OK;
	}

	void toggle() {
		FrameToggleFull((FRAME*)internalHandle);
	}
	int gm_toggle(gmThread* a_thread) {
		toggle();
		return ReturnThis(a_thread);
	}

	void flip() {
		FrameFlip((FRAME*)internalHandle);
	}
	int gm_flip(gmThread* a_thread) {
		flip();
		return GM_OK;
	}

	void clear() {
		glClearColor(clearRGBA[0], clearRGBA[1], clearRGBA[2], clearRGBA[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	int gm_clear(gmThread* a_thread) {
		clear();
		return GM_OK;
	}

	bool render(bool doClear) {
		if(!internalHandle) { /*Log(LOG_ERROR, "Attempted to render to non-opened frame."); */return false; }
		glUtilsSetRenderTarget(0); select();
		if(doClear) { clear(); }
		if(root) { root->ValidateAndRender(); }
		if(doClear) { flip(); show(true); }
		return true;
	}
	int gm_render(gmThread* a_thread) {
		int doClear = 1;
		a_thread->ParamInt(0, doClear, 1);
		a_thread->PushInt(render((bool)doClear));
		//return ReturnThis(a_thread);
		//return GM_SYS_YIELD;
		return GM_OK;
	}

	void show(bool _show) {
		FrameShow(((FRAME*)internalHandle), _show);
	}
	int gm_show(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(_show, 0);
		show(_show);
		return ReturnThis(a_thread);
	}

	bool hitTest(float x, float y) {
		return false;
	}
	int gm_hitTest(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(_x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(_y, 1);
		bool hit = hitTest(_x, _y);
		a_thread->PushInt((int)hit);
	}

	void setDimensions(float width, float height) {
		((FRAME*)internalHandle)->vwidth = this->width = width;
		((FRAME*)internalHandle)->vheight = this->height = height;
	}
	int gm_setDimensions(gmThread* a_thread) {
		GM_CHECK_FLOAT_OR_INT_PARAM(w, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(h, 1);
		setDimensions(w, h);
		return ReturnThis(a_thread);
	}

	void mirror(bool mirrorX, bool mirrorY) {
		this->mirrorX = mirrorX;
		this->mirrorY = mirrorY;
	}
	int gm_mirror(gmThread* a_thread) {
		int mirrorX, mirrorY;
		a_thread->ParamInt(0, mirrorX, 0);
		a_thread->ParamInt(1, mirrorY, 0);
		mirror(mirrorX, mirrorY);
		return ReturnThis(a_thread);
	}

	int gm_showCursor(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(show, 0);
		FrameShowCursor( (FRAME*)internalHandle, (bool)show );
		return ReturnThis(a_thread);
	}

	int gm_exit(gmThread* a_thread) {
		machine->ResetAndFreeMemory();
		MachineDestroy();
		exit(0);
		return GM_OK;
	}

	int gm_maximize(gmThread* a_thread) {
		FrameMaximize((FRAME*)internalHandle);
		return ReturnThis(a_thread);
	}

	int gm_minimize(gmThread* a_thread) {
		FrameMinimize((FRAME*)internalHandle);
		return ReturnThis(a_thread);
	}

#ifdef _WIN32
	int gm_activate(gmThread* a_thread) {
		FrameActivate((FRAME*)internalHandle);
		return ReturnThis(a_thread);
	}

	int gm_getMultitouchCount(gmThread* a_thread) {
		a_thread->PushInt(FrameGetMultitouchCount((FRAME*)internalHandle));
		return GM_OK;
	}

	int gm_getMultitouchPoint(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(index, 0);
		Vector* pos = new Vector();
		if(FrameGetMultitouch((FRAME*)internalHandle, index, &pos->x, &pos->y)) { return pos->ReturnThis(a_thread); }
		delete(pos);
		GM_EXCEPTION_MSG("index %d out of range", index);
		return GM_EXCEPTION;
	}
#endif

public:
	void* internalHandle;
	float width, height;
	Node* root;
	float clearRGBA[4];
	int mirrorX;
	int mirrorY;
};


#endif
