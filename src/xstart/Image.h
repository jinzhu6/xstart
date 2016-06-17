#ifndef _IMAGE_H_
#define _IMAGE_H_

// OBSOLETE: Use Bitmaps and Textures.

#if 0

#include "Node.h"


class Image : public Node {
public:

	Image() : Node() {
		id = "Image";
		ctor = "((optional) {string} file, (optional) {float} pos_x, (optional) {float} pos_y, (optional) {float} width, (optional) {float} height)";
		help = "(OBSOLETE, USE [Texture] OR [Bitmap] instead)";

		texture = 0;
		image = 0;
		alpha = 1.0;
		useBlending = 0;

		BindMember("alpha", &alpha, TYPE_FLOAT);
		BindMember("useBlending", &useBlending, TYPE_INT);

		BindFunction("load", (SCRIPT_FUNCTION)&Image::gm_load);
		BindFunction("save", (SCRIPT_FUNCTION)&Image::gm_save);
		BindFunction("create", (SCRIPT_FUNCTION)&Image::gm_create);
		BindFunction("pick", (SCRIPT_FUNCTION)&Image::gm_pick);
//		BindFunction("_center", (SCRIPT_FUNCTION)&Image::gm_center);
//		BindFunction("_fill", (SCRIPT_FUNCTION)&Image::gm_fill);
	}

	virtual int Initialize(gmThread* a_thread) { // (file,x,y,w,h)
		if(a_thread->GetNumParams() >= 1) {
			gmVariable var;
			a_thread->Param(0, var);
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
		return GM_OK;
	}

	virtual void invalidate() {
		if(texture) {
			TextureDestroy(texture);
		}
		texture = 0;

		if(image) {
			ImageDestroy(image);
		}
		image = 0;

		valid = false;
	}

	virtual void validate() {
		if(!image) {
			image = ImageCreate((dimension->x >= 1.0) ? dimension->x : 1.0, (dimension->y >= 1.0) ? dimension->y : 1.0);
			if(!image) {
				Log(LOG_ERROR, "Image (id:%s) with dimension [%d,%d] could not be created!", id.c_str(), dimension->x, dimension->y);
				return;
			}
			ImageFill(image, 0xFFFFFF00);
		}

		if(!texture && image) {
			texture = TextureCreate(image->width, image->height, TEX_NOMIPMAP, 0);
			TextureSetImage(texture, image);
		}

		valid = true;
	}

	void render() {
		// TODO: Possible duplicate with validate() stuff?
		if(!texture && image) {
			texture = TextureCreate(image->width, image->height, TEX_NOMIPMAP, 0);
			TextureSetImage(texture, image);
		}

		if(texture) {
			if(useBlending) { glEnable(GL_BLEND); }
			RenderTextureQuad(texture, position->x, position->y, center->x, center->y, dimension->x, dimension->y, 0, rotation->z, 1.0, alpha);
		}

		RenderChilds();
	}

	bool load(const char* file) {
		invalidate();
		image  = ImageLoad(file);
		if(!image) {
			Log(LOG_ERROR, "Image '%s' could not be loaded!", file);
			return false;
		} else {
			Log(LOG_DEBUG, "Image '%s' loaded successfully.", file);
		}
		dimension->x = image->width;
		dimension->y = image->height;
		return true;
	}
	int gm_load(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt((int)load(file));
		return GM_OK;
	}

	bool save(const char* file) {
		validate();
		ImageSave(image, file);
		return true;
	}
	int gm_save(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt((int)save(file));
		return GM_OK;
	}

	bool create(int w, int h) {
		invalidate();
		image = ImageCreate(w, h);
		return image != 0;
	}
	int gm_create(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_INT_PARAM(w, 0);
		GM_CHECK_INT_PARAM(h, 1);
		a_thread->PushInt(create(w,h));
		return GM_OK;
	}

	/*void fill(int r, int g, int b, int a) {
		// ???
		validate();
		ImageFill(image, (a << 24) + (r << 16) + (g << 8) + b);
		valid = false;
	}
	int gm_fill(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(r, 0);
		GM_CHECK_INT_PARAM(g, 1);
		GM_CHECK_INT_PARAM(b, 2);
		GM_CHECK_INT_PARAM(a, 3);
		fill(r,g,b,a);
		return GM_OK;
	}*/
	/*
	void _center() {
		Validate();
		this->center->x = this->dimension->x * 0.5;
		this->center->y = this->dimension->y * 0.5;
		this->center->z = this->dimension->z * 0.5;
	}
	int gm_center(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		_center();
		return GM_OK;
	}
	*/

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
	IMAGE* image;
	TEXTURE* texture;
	float alpha;
	int useBlending;
};


#endif

#endif