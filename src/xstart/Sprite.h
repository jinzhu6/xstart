#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "Model.h"

class Box : public Mesh {
public:

	Box() : Mesh() {
		id = "Box";
		ctor = "([Texture] texture, {float} width, {float} height)";
		help = "";
	}

	virtual int Initialize(gmThread* a_thread) {
		if (a_thread->ParamType(0) == GM_TYPE_OBJECT) {
			SetMemberObject("texture0", (Texture*)a_thread->ParamUser_NoCheckTypeOrParam(0));
		}
		float w = dimension->x = a_thread->ParamFloatOrInt(1, 64.0);
		float h = dimension->y = a_thread->ParamFloatOrInt(2, 64.0);
		setPosition(0, -1.0*w / 2, -1.0*h / 2, 0.0);
		setPosition(1, 1.0*w / 2, -1.0*h / 2, 0.0);
		setPosition(2, 1.0*w / 2, 1.0*h / 2, 0.0);
		setPosition(3, -1.0*w / 2, -1.0*h / 2, 0.0);
		setPosition(4, 1.0*w / 2, 1.0*h / 2, 0.0);
		setPosition(5, -1.0*w / 2, 1.0*h/2, 0.0);
		setTexCoord(0, 0.0, 0.0, 0.0);
		setTexCoord(1, 1.0, 0.0, 0.0);
		setTexCoord(2, 1.0, 1.0, 0.0);
		setTexCoord(3, 0.0, 0.0, 0.0);
		setTexCoord(4, 1.0, 1.0, 0.0);
		setTexCoord(5, 0.0, 1.0, 0.0);
		setNormal(0, 0.0, 0.0, 1.0);
		setNormal(1, 0.0, 0.0, 1.0);
		setNormal(2, 0.0, 0.0, 1.0);
		setNormal(3, 0.0, 0.0, 1.0);
		setNormal(4, 0.0, 0.0, 1.0);
		setNormal(5, 0.0, 0.0, 1.0);
		return GM_OK;
	}

private:


};

#endif
