#ifndef _COLOR_H_
#define _COLOR_H_

#include <corela.h>
#include "ScriptObject.h"

#define CLAMP(x,min,max) {if(x<min)x=min;if(x>max)x=max;}

class Color : public ScriptObject {
public:
	Color() {
		id = "Color";
		help = "Color class to help computatation and conversions. Individual values range from 0.0 to 1.0. YCbCr is full range.";

		r = g = b = a = 0.0;
		y = cb = cr = 0.0;

		BindMember("r", &r, TYPE_FLOAT, 0, "{float} r", "Red.");
		BindMember("g", &g, TYPE_FLOAT, 0, "{float} g", "Green.");
		BindMember("b", &b, TYPE_FLOAT, 0, "{float} b", "Blue.");
		BindMember("a", &a, TYPE_FLOAT, 0, "{float} a", "Alpha.");
		BindMember("y", &y, TYPE_FLOAT, 0, "{float} y", "Luma.");
		BindMember("cb", &cb, TYPE_FLOAT, 0, "{float} cb", "Blue difference chroma.");
		BindMember("cr", &cr, TYPE_FLOAT, 0, "{float} cr", "Red difference chroma.");
		BindMember("h", &h, TYPE_FLOAT, 0, "{float} h", "Hue.");
		BindMember("s", &s, TYPE_FLOAT, 0, "{float} s", "Saturation.");
		BindMember("v", &v, TYPE_FLOAT, 0, "{float} v", "Value.");

		BindFunction("fromStringRGBA", (SCRIPT_FUNCTION)&Color::gm_fromStringRGBA, "[this] fromStringRGBA({string} rgba)", "Parse a color string and save the values in the members r,g,b,a and y,cb,cr as floats.");
		BindFunction("toStringRGBA", (SCRIPT_FUNCTION)&Color::gm_toStringRGBA, "{string} toStringRGBA()", "Gets a string representation (#RRGGBBAA) of the stored r,g,b,a and a values.");
		BindFunction("updateYCbCrFromRGB", (SCRIPT_FUNCTION)&Color::gm_updateYCbCr, "[this] updateYCbCr()", "Updates the y,cb,cr values from the r,g,b values.");
		BindFunction("updateRGBFromYCbCr", (SCRIPT_FUNCTION)&Color::gm_updateRGB, "[this] updateRGB()", "Updates the r,g,b,a values from the y,cb,cr values.");
		BindFunction("updateHSVFromRGB", (SCRIPT_FUNCTION)&Color::gm_updateHSVFromRGB);
		BindFunction("updateRGBFromHSV", (SCRIPT_FUNCTION)&Color::gm_updateRGBFromHSV);
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() == 1) {
			GM_CHECK_STRING_PARAM(rgba, 0);
			fromStringRGBA(rgba);
		}
		return GM_OK;
	}

	void fromStringRGBA(const char* rgba) {
		ColorParse(rgba, &r, &g, &b, &a);
		updateYCbCr();
	}
	int gm_fromStringRGBA(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(rgba, 0);
		fromStringRGBA(rgba);
		return ReturnThis(a_thread);
	}
	std::string toStringRGBA() {
		char color[10];
		float _r = r, _g = g, _b = b, _a = a;
		CLAMP(_r, 0.0, 1.0);	CLAMP(_g, 0.0, 1.0);	CLAMP(_b, 0.0, 1.0);	CLAMP(_a, 0.0, 1.0);
		sprintf(color, "#%02X%02X%02X%02X", (int)(_r * 255.0), (int)(_g * 255.0), (int)(_b * 255.0), (int)(_a * 255.0));
		return std::string(color);
	}
	int gm_toStringRGBA(gmThread* a_thread) {
		a_thread->PushNewString( toStringRGBA().c_str(), 9 );
		return GM_OK;
	}
	void updateYCbCr() {
		y  =  0.299f * r + 0.587f * g + 0.114f * b;
		cr =  0.500f * r - 0.419f * g - 0.081f * b  + 0.5f;
		cb = -0.169f * r - 0.331f * g + 0.500f * b  + 0.5f;
	}
	int gm_updateYCbCr(gmThread* a_thread) {
		updateYCbCr();
		return ReturnThis(a_thread);
	}

	void updateRGB() {
		r = y + (cr - 0.5) * 1.4;
		g = y + (cb - 0.5) * -0.343 + (cr - 0.5) * -0.711;
		b = y + (cb - 0.5) * 1.765;
		CLAMP(r, 0.0, 1.0);   CLAMP(g, 0.0, 1.0);   CLAMP(b, 0.0, 1.0);
	}
	int gm_updateRGB(gmThread* a_thread) {
		updateRGB();
		return ReturnThis(a_thread);
	}

	int gm_updateRGBFromHSV(gmThread* a_thread) {
		double      hh, p, q, t, ff;
		long        i;

		if(s <= 0.001) {r = g = b = v;	return GM_OK; }

		hh = h * 360;
		if(hh >= 360.0) { hh = 0.0; }
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = v * (1.0 - s);
		q = v * (1.0 - (s * ff));
		t = v * (1.0 - (s * (1.0 - ff)));

		switch(i) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		case 5:
		default:
			r = v;
			g = p;
			b = q;
			break;
		}
		return GM_OK;
	}

	int gm_updateHSVFromRGB(gmThread* a_thread) {
		double      min, max, delta;

		min = r < g ? r : g;
		min = min  < b ? min  : b;
		max = r > g ? r : g;
		max = max  > b ? max : b;

		v = max; // v is max
		delta = max - min;
		if (delta < 0.0001) {
			s = 0;
			h = 0;
			return GM_OK;
		}
		if(max <= 0.0001) { s = 0.0; h = 0.0; return GM_OK; }

		s = (delta / max);

		if(r >= max) {
			h = (g - b) / delta;    // between yellow & magenta
		} else if(g >= max) {
			h = 2.0 + (b - r) / delta;    // between cyan & yellow
		} else {
			h = 4.0 + (r - g ) / delta;    // between magenta & cyan
		}

		h *= 60.0;                              // degrees
		while(h < 0.0) { h += 360.0; }
		h /= 360;

		return GM_OK;
	}

public:
	float r, g, b, a;
	float y, cb, cr;
	float h,s,v;
};

#endif
