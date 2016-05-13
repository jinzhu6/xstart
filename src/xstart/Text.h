#ifndef _TEXT_H_
#define _TEXT_H_

#include "Texture.h"
#include "Font.h"
#include "Color.h"


class Text : public Texture {
public:

	Text() : Texture() {
		id= "Text";
		ctor = "({string} text, {string} font, {int} x, {int} y, {string} color)";
		help = "Text node class for font text rendering and caching.";

		textureFlags = TEX_NOMIPMAP;   //  TEX_NOFILTER | TEX_SHARP | TEX_NOMIPMAP | TEX_CLAMP

		text = " ";
		color = "#ffffffff";
		outline = 0;
		outlineColor = "#000000ff";
		font = 0;
		clearColor = "#ffffff00";
		password = 0;
		passwordLength = 0;

		BindMember("font", &font, TYPE_OBJECT);
		BindMember("text", &text, TYPE_STRING);
		BindMember("color", &color, TYPE_STRING);
		BindMember("outline", &outline, TYPE_INT);
		BindMember("password", &password, TYPE_INT);
		BindMember("passwordLength", &passwordLength, TYPE_INT);
		BindMember("outlineColor", &outlineColor, TYPE_STRING);
	}

	virtual int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) { // text
			if(a_thread->ParamType(0) == GM_STRING) {
				GM_CHECK_STRING_PARAM(_text, 0);
				text = _text;
			}
		}
		if(a_thread->GetNumParams() >= 2) { // font
			if(a_thread->ParamType(1) == GM_TYPE_OBJECT) {
				GM_CHECK_USER_PARAM(ScriptObject*, GM_TYPE_OBJECT, _font, 1);
				SetMemberObject("font", _font, &a_thread->Param(1));
				font = (Font*)_font;
			}
		}
		if(a_thread->GetNumParams() >= 3) { // x
			a_thread->ParamFloatOrInt(2, position->x, 0.0f);
		}
		if(a_thread->GetNumParams() >= 4) { // y
			a_thread->ParamFloatOrInt(3, position->y, 0.0f);
		}
		if(a_thread->GetNumParams() >= 5) { // color
			this->color = a_thread->ParamString(4, "#ffffffff");
		}
		if(a_thread->GetNumParams() >= 6) { // outline
			a_thread->ParamInt(5, this->outline, 0);
		}
		if(a_thread->GetNumParams() >= 7) { // outlineColor
			this->outlineColor = a_thread->ParamString(6, "#000000ff");
		}
		return GM_OK;
	}

	void update() {
		if(!valid) {
			if(!font) return;
			validate();

			// use the outline or text color to clear the texture
			std::string bgColor;
			if(outline) {
				bgColor = outlineColor;
			} else {
				bgColor = color;
			}

			// set alpha for the texture cleaning to zero
			Color col = Color();
			col.fromStringRGBA(bgColor.c_str());
			col.a = 0.0;
			clearColor = col.toStringRGBA();

			// for passwords change all characters to '•'
			std::string pw;
			if(password) {
				for(int n=0; n<text.length(); n++) pw += "*";
				for(int n=0; n<passwordLength-text.length(); n++) pw += "-";
				//pw += "⏐";
			} else {
				pw = text;
			}

			// measure text
			int width, height;
			font->measure(pw.c_str(), outline, &width, &height);
			width += 1;
			height += 2;

			// check texture
			if(texture) {
				if(texture->width != width || texture->height != height) {
					Texture::create(width, height);
				}
			} else {
				Texture::create(width, height);
			}
			Texture::clear();

			if(font->drawText(this, pw.c_str(), color.c_str(), 0, 0, outline, outlineColor.c_str())) {
				this->dimension->x = texture->width;
				this->dimension->y = texture->height;
			}
		}

		return;
	}

	void SetDot(const char* key, gmVariable &var) {
		if(strcmp(key, "text") == 0) {
			if(strcmp(text.c_str(), var.GetCStringSafe()) != 0) {
				invalidate();
			}
			/*if(text == "") {
				text = " ";
			}*/
		}
		if(strcmp(key, "font") == 0) {
			invalidate();
		}
		if(strcmp(key, "color") == 0) {
			invalidate();
		}
		if(strcmp(key, "outline") == 0) {
			invalidate();
		}
		if(strcmp(key, "outlineColor") == 0) {
			invalidate();
		}
		if(strcmp(key, "rightToLeft") == 0) {
			invalidate();
		}
	}

public:
	int outline;
	int password;
	int passwordLength;
	std::string text;
	std::string color;
	std::string outlineColor;
	Font* font;
};


#endif
