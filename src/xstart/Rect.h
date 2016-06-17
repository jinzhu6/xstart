#ifndef _RECT_H_
#define _RECT_H_

#include "ScriptObject.h"

#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a<b?b:a)


class Rect : public ScriptObject {
public:

	Rect() : ScriptObject() {
		id = "Rect";
		left = top = right = bottom = 0.0;

		BindFunction("enclose", (SCRIPT_FUNCTION)&Rect::_enclose);
		BindMember("left", &left, TYPE_FLOAT);
		BindMember("top", &top, TYPE_FLOAT);
		BindMember("right", &right, TYPE_FLOAT);
		BindMember("bottom", &bottom, TYPE_FLOAT);
		BindFunction("set", (SCRIPT_FUNCTION)&Rect::gm_set);
		BindFunction("hitTest", (SCRIPT_FUNCTION)&Rect::gm_hitTest);
		BindFunction("getWidth", (SCRIPT_FUNCTION)&Rect::gm_getWidth);
		BindFunction("getHeight", (SCRIPT_FUNCTION)&Rect::gm_getHeight);
	}

	void set(float _left, float _top, float _right, float _bottom) {
		left = _left;
		top = _top;
		right = _right;
		bottom = _bottom;
	}
	int gm_set(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_FLOAT_OR_INT_PARAM(left, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(top, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(right, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(bottom, 3);

		set(left, top, right,bottom);

		return GM_OK;
	}

	bool hitTest(double x, double y) {
		if(x < left) { return false; }
		if(x > right) { return false; }
		if(y < top) { return false; }
		if(y > bottom) { return false; }
		return true;
	}
	int gm_hitTest(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);

		a_thread->PushInt(hitTest(x, y));

		return GM_OK;
	}

	void enclose(const Rect &r) {
		left = MIN(left, r.left);
		top = MIN(top, r.top);
		right = MAX(right, r.top);
		bottom = MAX(bottom, r.bottom);
	}

	int _enclose(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Rect*, GM_TYPE_OBJECT, rc, 0);
		enclose(*rc);
		return GM_OK;
	}

	int gm_getWidth(gmThread* a_thread) {
		a_thread->PushFloat(right - left);
		return GM_OK;
	}

	int gm_getHeight(gmThread* a_thread) {
		a_thread->PushFloat(bottom - top);
		return GM_OK;
	}

public:
	float left;
	float top;
	float right;
	float bottom;
};


#endif
