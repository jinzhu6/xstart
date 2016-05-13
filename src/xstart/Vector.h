#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "ScriptObject.h"

class Vector : public ScriptObject {
public:

	Vector() : ScriptObject() {
		id = "Vector";
		help = "3D vector class.";
		ctor = "((optional) {float} x, (optional) {float} y, (optional) {float} z)  or  ([Vector] copy)";

		x = y = z = 0.0;

		BindMember("x", &x, TYPE_FLOAT);
		BindMember("y", &y, TYPE_FLOAT);
		BindMember("z", &z, TYPE_FLOAT);
		BindFunction("set", (SCRIPT_FUNCTION)&Vector::gm_set);
		BindFunction("length", (SCRIPT_FUNCTION)&Vector::gm_length);
		BindFunction("add", (SCRIPT_FUNCTION)&Vector::gm_add);
		BindFunction("subtract", (SCRIPT_FUNCTION)&Vector::gm_subtract);
		BindFunction("scale", (SCRIPT_FUNCTION)&Vector::gm_scale);
		BindFunction("divide", (SCRIPT_FUNCTION)&Vector::gm_divide);
		BindFunction("distance", (SCRIPT_FUNCTION)&Vector::gm_distance);
		BindFunction("normalize", (SCRIPT_FUNCTION)&Vector::gm_normalize);
		BindFunction("normal", (SCRIPT_FUNCTION)&Vector::gm_normal);
		BindFunction("radians2D", (SCRIPT_FUNCTION)&Vector::gm_radians2D);
		BindFunction("rotate2D", (SCRIPT_FUNCTION)&Vector::gm_rotate2D);
		BindFunction("cross2D",  (SCRIPT_FUNCTION)&Vector::gm_cross2D);
		BindFunction("dot2D",  (SCRIPT_FUNCTION)&Vector::gm_dot2D);
	}

	int Initialize(gmThread* a_thread) {
		return gm_set(a_thread);
	}

	void set(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}
	int gm_set(gmThread* a_thread) {
		if(a_thread->ParamType(0) == GM_TYPE_OBJECT) {
			GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
			x = vin->x;
			y = vin->y;
			z = vin->z;
			return GM_OK;
		}
		if(a_thread->GetNumParams() >= 1) {
			GM_CHECK_FLOAT_OR_INT_PARAM(_x, 0);
			x = _x;
		}
		if(a_thread->GetNumParams() >= 2) {
			GM_CHECK_FLOAT_OR_INT_PARAM(_y, 1);
			y = _y;
		}
		if(a_thread->GetNumParams() >= 3) {
			GM_CHECK_FLOAT_OR_INT_PARAM(_z, 2);
			z = _z;
		}
		return GM_OK;
	}

	int gm_length(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		float length = sqrt(x*x + y*y + z*z);
		a_thread->PushFloat(length);
		return GM_OK;
	}

	int gm_subtract(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
		x -= vin->x;
		y -= vin->y;
		z -= vin->z;
		return ReturnThis(a_thread);
	}

	int gm_add(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
		x += vin->x;
		y += vin->y;
		z += vin->z;
		return ReturnThis(a_thread);
	}

	int gm_scale(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
		x *= vin->x;
		y *= vin->y;
		z *= vin->z;
		return ReturnThis(a_thread);

	}

	int gm_divide(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
		x /= vin->x;
		y /= vin->y;
		z /= vin->z;
		return ReturnThis(a_thread);
	}

	int gm_normalize(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		float length = sqrt(x*x + y*y + z*z);
		x /= length;
		y /= length;
		z /= length;
		return ReturnThis(a_thread);
	}

	int gm_normal(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		float length = sqrt(x*x + y*y + z*z);
		Vector* v = new Vector();
		v->x = x/length;  v->y = y/length;  v->z = z/length;
		return v->ReturnThis(a_thread);
	}

	int gm_distance(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
		float _x = x - vin->x;
		float _y = y - vin->y;
		float _z = z - vin->z;
		float length = sqrt(_x*_x + _y*_y + _z*_z);
		a_thread->PushFloat(length);
		return GM_OK;
	}

	int gm_radians2D(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		float rad = atan2(x, -y);
		if(rad < 0.0) { rad = 3.14159265359 * 0.5 + (3.14159265359 - -rad) * 0.5; }
		else { rad *= 0.5; }
		a_thread->PushFloat(rad);
		return GM_OK;
	}

	int gm_rotate2D(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		GM_CHECK_FLOAT_OR_INT_PARAM(rad, 0);
		float xrot = cos(rad) * x - sin(rad) * y;
		float yrot = cos(rad) * y + sin(rad) * x;
		x = xrot;
		y = yrot;
		return ReturnThis(a_thread);
	}

	int gm_cross2D(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
		float cross = x * vin->y - y * vin->x;
		a_thread->PushFloat(cross);
		return GM_OK;
	}

	int gm_dot2D(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Vector*, GM_TYPE_OBJECT, vin, 0);
		float dot = x * vin->x + y * vin->y;
		a_thread->PushFloat(dot);
		return GM_OK;
	}

public:
	gmfloat x;
	gmfloat y;
	gmfloat z;
};

#endif
