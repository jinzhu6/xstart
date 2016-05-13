#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "NodeEx.h"


class Texture : public NodeEx {
public:

	Texture() : NodeEx() {
		id = "Texture";
		ctor = "((optional) {string} file, (optional) {float} pos_x, (optional) {float} pos_y, (optional) {float} width, (optional) {float} height), (optional) {string} clearColor";
		help = "Texture node class.";

		texture = 0;
		textureFormat = TEX_FORMAT_RGBA;
		textureFlags = TEX_NOMIPMAP; //TEX_NOMIPMAP | TEX_NOFILTER; //TEX_CLAMP;
		clearColor = "#ff00ffff";
		opacity = 1.0;

		BindMember("clearColor", &clearColor, TYPE_STRING, 0, "{string} clearColor", "Color that is used to clear or create the texture.");
		BindMember("opacity", &opacity, TYPE_FLOAT, 0, "{float} opacity", "Opacity of the texture while rendering.");
//		BindMember("flipX", &flipX, TYPE_INT, 0, "{int} flipX", "Flips the texture horizontally while rendering.");
//		BindMember("flipY", &flipY, TYPE_INT, 0, "{int} flipY", "Flips the texture vertically while rendering.");

		BindFunction("setBitmap", (SCRIPT_FUNCTION)&Texture::gm_setBitmap, "{int} setBitmap({Bitmap} bitmap)");
		BindFunction("getBitmap", (SCRIPT_FUNCTION)&Texture::gm_getBitmap, "{Bitmap} getBitmap()");
		BindFunction("load", (SCRIPT_FUNCTION)&Texture::gm_load, "[this] load({string} file)", "Loads an image file and uses it. Returns null on failure.");
		BindFunction("save", (SCRIPT_FUNCTION)&Texture::gm_save, "[this] save({string} file", "Saves the current content to a PNG image file.");
		BindFunction("create", (SCRIPT_FUNCTION)&Texture::gm_create, "{int} create({int} width, {int} height)", "(Re-)creates the texture with the given dimensions. Returns non-zero on success.");
		BindFunction("duplicate", (SCRIPT_FUNCTION)&Texture::gm_duplicate, "[Texture] duplicate()", "Creates and returns a duplicate of the texture. The returned texture object has its own texture memory.");
		BindFunction("clear", (SCRIPT_FUNCTION)&Texture::gm_clear, "[this] clear((optional) {string} color)", "Fills the texture with the given or set clear color.");
		BindFunction("pick", (SCRIPT_FUNCTION)&Texture::gm_pick, "{string} pick({int} x, {int} y)", "Returns the color on the given texel position. The string has the format RRGGBBAA with hexadigital numbers. The [Color] can be used afterwards to translate or modify the color.");
		BindFunction("resize", (SCRIPT_FUNCTION)&Texture::gm_resize, "[this] resize({int} width, {int} height, (optional) {int} bilinear)", "Resizes the texture to the given size. Optionally a bilinear filter applies. <b>On some hardware & platform combinates (especially embedded hardware) the actual dimension may differ due to power-of-two and size restrictions.</b>");
		BindFunction("_center", (SCRIPT_FUNCTION)&Texture::gm_center, "_center()", "Moves the center-pivot to the middle of the bounding rect. This will change the rendering and rotation position!");
	}

	~Texture() {
		if(texture) {
			TextureDestroy(texture);
		}
	}

	virtual int Initialize(gmThread* a_thread) {
		gmVariable varNull;
		varNull.Nullify();
		if(a_thread->GetNumParams() >= 1) {
			gmVariable var = a_thread->Param(0, varNull);
			if(!var.IsNull()) {
				load(var.GetCStringSafe());
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
		const char* color = 0;
		if(a_thread->GetNumParams() >= 6) {
			a_thread->ParamString(5, color, "#ffffff");
			clearColor = color;
		}
		/*if(!texture) {
			texture = TextureCreate(dimension->x, dimension->y, textureFlags, color);
		}*/
		return GM_OK;
	}

	void render() {
		if(texture) {
			glColor4f(1.0,1.0,1.0,opacity);
			RenderTextureQuad(texture, position->x, position->y, pivot->x * scaling->x * scaling->z, pivot->y * scaling->y * scaling->z, dimension->x * scaling->x * scaling->z, dimension->y * scaling->y * scaling->z, 0, rotation->z, 1.0, opacity);
		}
		RenderChilds();
	}

	void clear() {
		if(texture) {
			TextureClear(texture, ColorParse(clearColor.c_str()));
		}
	}
	int gm_clear(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) {
			GM_CHECK_STRING_PARAM(_color, 0);
			clearColor = _color;
		}
		clear();
		return ReturnThis(a_thread);
	}

	bool resize(int width, int height, bool bilinear) {
		if(texture) {
			if(texture->width == width && texture->height == height) {
				return true;
			}
			IMAGE* image = ImageCreate(texture->width, texture->height);
			if(!image) {
				return false;
			}
			TextureGetImage(texture, image);
			ImageResize(image, width, height, bilinear);
			TextureSetImage(texture, image);
			ImageDestroy(image);
			// TODO: Get actual texture dimension back and set that!
			dimension->x = width;
			dimension->y = height;
		} else {
			create(width, height);
		}
		return true;
	}
	int gm_resize(gmThread* a_thread) {
		if(!texture) {
			return ReturnThis(a_thread);
		}
		int bilinear = 1;
		gmfloat w = texture->width, h = texture->height;
		int np = a_thread->GetNumParams();
		if(np >= 1) {
			a_thread->ParamFloatOrInt(0, w, 0.0);
		}
		if(np >= 2) {
			a_thread->ParamFloatOrInt(1, h, 0.0);
		}
		if(np >= 3) {
			a_thread->ParamInt(2, bilinear, 1);
		}
		resize(w,h,bilinear);
		return ReturnThis(a_thread);
	}

	bool create(int width, int height) {
		if(texture) {
			TextureDestroy(texture);
		}
		texture = TextureCreate(width, height, textureFlags, clearColor.c_str());
		return texture ? true : false;
	}
	int gm_create(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(w, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(h, 1);
		a_thread->PushInt(create((int)w, (int)h));
		return GM_OK;
	}

	Texture* duplicate() {
		Texture* dup = new Texture();
		dup->texture = TextureDuplicate(this->texture);
		dup->clearColor = this->clearColor;
		dup->opacity = this->opacity;
		return dup;
	}
	int gm_duplicate(gmThread* a_thread) {
		return duplicate()->ReturnThis(a_thread);
	}

	bool setImage(IMAGE* image) {
		if(!image) {
			return false;
		}
		if(!texture) {
			texture = TextureCreate(image->width, image->height, 0, clearColor.c_str());
		}
		if(texture->width != image->width || texture->height != image->height) {
			TextureDestroy(texture);
			texture = 0;
		}
		if(!texture) {
			texture = TextureCreate(image->width, image->height, 0, clearColor.c_str());
		}
		TextureUpload(texture, (coByte*)image->data);
		return true;
	}
	int gm_setBitmap(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Bitmap*, GM_TYPE_OBJECT, bitmap, 0);
		a_thread->PushInt(setImage(bitmap->image));
		return GM_OK;
	}

	Bitmap* getBitmap() {
		if(!texture) {
			return 0;
		}
		Bitmap* bitmap = new Bitmap();
		bitmap->create(texture->width, texture->height);
		TextureGetImage(texture, bitmap->image);
		return bitmap;
	}
	int gm_getBitmap(gmThread* a_thread) {
		Bitmap* bitmap = getBitmap();
		if(bitmap) {
			return bitmap->ReturnThis(a_thread);
		}
		return GM_OK;
	}

	virtual bool load(const char* file) {
		if(texture) { TextureDestroy(texture); }
		texture = TextureLoad(_FILE(file), textureFlags);
		if(!texture) {
			Log(LOG_ERROR, "Image '%s' not found!", file);
			return false;
		} else {
			Log(LOG_DEBUG, "Image '%s' loaded successfully.", file);
		}
		dimension->x = texture->vwidth;
		dimension->y = texture->vheight;
		return true;
	}
	int gm_load(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(file, 0);
		if(load(file)) {
			ReturnThis(a_thread);
			return GM_OK;
		} else {
			ReturnNull(a_thread);
			return GM_OK;
		}
	}

	bool save(const char* file, int _width, int _height) {
		if(texture) {
			int w=texture->width, h=texture->height;
			if(_width)  {
				w = _width;
			}
			if(_height) {
				h = _height;
			}
			TextureSave(texture, file, w, h);
		}
		return true;
	}
	int gm_save(gmThread* a_thread) {
		int w=0, h=0;
		GM_CHECK_STRING_PARAM(file, 0);
		if(a_thread->GetNumParams() >= 2) {
			a_thread->ParamInt(1, w, 0);
		}
		if(a_thread->GetNumParams() >= 3) {
			a_thread->ParamInt(2, h, 0);
		}
		a_thread->PushInt((int)save(file,w,h));
		return GM_OK;
	}

	PIXEL pick(int x, int y) {
		IMAGE* image = ImageCreate(1,1);
		TextureGetImage(texture, image);
		PIXEL pixel = *(ImageGetPixel(image, x, y));
		ImageDestroy(image);
		return pixel;
	}
	int gm_pick(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);
		PIXEL pixel = pick( (int)x, (int)y);
		char color[16];
		sprintf(color, "%02x%02x%02x%02x", pixel.red, pixel.green, pixel.blue, pixel.alpha);
		a_thread->PushNewString(color);
		return GM_OK;
	}

public:
	TEXTURE* texture;
	int textureFormat;
	coDword textureFlags;
	std::string clearColor;
	float opacity;
//	int flipX;
//	int flipY;
};


#endif
