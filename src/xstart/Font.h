#ifndef _FONT_H_
#define _FONT_H_

#include <corela.h>
#include "ScriptObject.h"
#include "Texture.h"


class Font : public ScriptObject {
public:

	Font() : ScriptObject() {
		id = "Font";
		ctor = "({string} fontFile, (optional) {int} fontSize), (optional) {int} righToLeft";
		help = "Creates a font object for rendering text.";

		font = 0;
		rightToLeft = 0;

		BindMember("rightToLeft", &rightToLeft, TYPE_INT);
		BindFunction("load", (SCRIPT_FUNCTION)&Font::gm_load, "[this] load({string} fontFile, {int} fontSize)", "Loads the font file with the given font size.");
		BindFunction("measure", (SCRIPT_FUNCTION)&Font::gm_measure, "[Vector] measure({string} text, (optional) {int} outlineWidth)", "Measures the texture size of the text if rendered with this font. Only x and y of the [Vector] object will be set.");
		BindFunction("drawText", (SCRIPT_FUNCTION)&Font::gm_drawText, "{int} drawText([Texture] texture, {string} text, {string} color, {int} x, {int} y), (optional) {int} outlineWidth, {string} outlineColor", "<b>Subject to change.</b> Draws a text directly in a given texture.");
	}

	~Font() {
		if(font) {
			FontDestroy(font);
		}
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() == 1) {
			GM_CHECK_STRING_PARAM(font, 0);
			__load(font, 18);
			return GM_OK;
		}
		if(a_thread->GetNumParams() == 2) {
			GM_CHECK_STRING_PARAM(font, 0);
			GM_CHECK_FLOAT_OR_INT_PARAM(size, 1);
			__load(font, size);
			return GM_OK;
		}
		if(a_thread->GetNumParams() == 3) {
			GM_CHECK_STRING_PARAM(font, 0);
			GM_CHECK_FLOAT_OR_INT_PARAM(size, 1);
			GM_CHECK_INT_PARAM(rToL, 2);
			rightToLeft = rToL;
			__load(font, size);
			return GM_OK;
		}
		return GM_OK;
	}

	bool __load(const char* fontFile, int fontSize) {
		font = FontLoad(_FILE(fontFile), fontSize);
		if(!font) { Log(LOG_ERROR, "Error while loading font '%s'!", fontFile); }
		return font != 0;
	}
	int gm_load(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		GM_CHECK_INT_PARAM(size, 1);
		a_thread->PushInt(__load(file,size));
		return ReturnThis(a_thread);
	}

	int measure(const char* text, int outline, int* widthOut, int* heightOut) {
		return FontRender(font, 0, 0, 0, widthOut, heightOut, text, 0, outline, 0);
	}
	int gm_measure(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(text, 0);

		int outline = 0;
		if(a_thread->GetNumParams() > 1) {
			a_thread->ParamInt(1, outline, 0);
		}

		int w, h;
		measure(text, outline, &w, &h);

		Vector* vDim = new Vector();
		vDim->x = (float)w;
		vDim->y = (float)h;
		vDim->z = 0.0;
		a_thread->PushNewUser(vDim, GM_TYPE_OBJECT);

		return GM_OK;
	}

	void drawToImage(IMAGE* image, const char* text, const char* color, int x, int y, int outline = 0, const char* colorOutline = "#000000") {
		FontRender(font, image, x, y, 0, 0, text, color, outline, colorOutline, rightToLeft);
	}

	bool drawText(Texture* texture, const char* text, const char* color, int x, int y, int outline = 0, const char* colorOutline = "#000000") {
		if(!font || !texture) { return false; }

		if(!texture->texture) {
			// find desireable texture dimensions
			int w,h;
			FontRender(font, 0, x, y, &w, &h, text, color, outline, colorOutline, rightToLeft);
			if(w <= 0 || h <= 0) { return false; }
			texture->create(w, h);
		}

		IMAGE* image = ImageCreate(texture->texture->width, texture->texture->height);
		TextureGetImage(texture->texture, image);
		FontRender(font, image, x, y, 0, 0, text, color, outline, colorOutline, rightToLeft);
		TextureSetImage(texture->texture, image);
		ImageDestroy(image);

		return true;
	}
	int gm_drawText(gmThread* a_thread) {
		//GM_CHECK_NUM_PARAMS(5);
		GM_CHECK_USER_PARAM(Texture*, GM_TYPE_OBJECT, texture, 0);
		GM_CHECK_STRING_PARAM(text, 1);
		GM_CHECK_STRING_PARAM(color, 2);
		GM_CHECK_INT_PARAM(x, 3);
		GM_CHECK_INT_PARAM(y, 4);
		if(a_thread->GetNumParams() >= 7) {
			GM_CHECK_INT_PARAM(outline, 5);
			GM_CHECK_STRING_PARAM(outlineColor, 6);
			a_thread->PushInt(drawText(texture, text, color, x, y, outline, outlineColor));
		} else {
			a_thread->PushInt(drawText(texture, text, color, x, y));
		}
		return GM_OK;
	}

public:

	FONT* font;
	int rightToLeft;

};

#endif
