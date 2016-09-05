#ifndef _BITMAP_IMAGE_H_
#define _BITMAP_IMAGE_H_

#include <corela.h>
#include "ScriptObject.h"


class Bitmap : public ScriptObject {
public:

	Bitmap() : ScriptObject() {
		id = "Bitmap";
		help = "The bitmap class can load, save, store and manipulate one bitmap image. This is NOT a scene node, consider using a [Texture] to render a bitmap. Bitmaps can transfer from and to [Texture].";
		ctor = "({string} imageFile)";
		image = 0;
		BindFunction("create", (SCRIPT_FUNCTION)&Bitmap::gm_create, "[this] create({int} width, {int} height)", "Creates a bitmap image or the given size.");
		BindFunction("load", (SCRIPT_FUNCTION)&Bitmap::gm_load, "{int} load({string} file)", "Loads the given image into the image bitmap. Width and height is taken from the image. If a different resolution is needed, the bitmap must be resized afterwards.");
		BindFunction("save", (SCRIPT_FUNCTION)&Bitmap::gm_save, "{int} save({string} file)", "Saves the bitmap as a PNG image.");
		BindFunction("clear", (SCRIPT_FUNCTION)&Bitmap::gm_clear, "[this] clear({string} color)", "Clears the image with the given color.");
		BindFunction("flipX", (SCRIPT_FUNCTION)&Bitmap::gm_flipX, "[this] flipX()", "Flips the bitmap image horizontally.");
		BindFunction("flipY", (SCRIPT_FUNCTION)&Bitmap::gm_flipY, "[this] flipY()", "Flips the bitmap image vertically.");
		BindFunction("swapRB", (SCRIPT_FUNCTION)&Bitmap::gm_swapRB, "[this] swapRB()", "Swaps the red and blue components of all pixels in the bitmap. This might be useful for images captured by a camera, since captured images may have a BGRA format.");
		BindFunction("resize", (SCRIPT_FUNCTION)&Bitmap::gm_resize, "[this] resize({int} width, {int} height, {int} bilinear");
		BindFunction("duplicate", (SCRIPT_FUNCTION)&Bitmap::gm_duplicate, "[Bitmap] duplicate()", "Returns a duplicate of the whole bitmap as a new object.");
		BindFunction("cut", (SCRIPT_FUNCTION)&Bitmap::gm_cut, "[Bitmap] cut({int} x, {int} y, {int} width, {int} height)", "Returns a copy of the given area.");
		BindFunction("paste", (SCRIPT_FUNCTION)&Bitmap::gm_paste, "paste([Bitmap], {int} x, {int} y", "Paste a given bitmap on the given coordinates.");
		BindFunction("softenEdge", (SCRIPT_FUNCTION)&Bitmap::gm_softenEdge, "softenEdge({int} width)", "Does a rectangular alpha-blending on the bitmap edges.");
		BindFunction("multiplyMask", (SCRIPT_FUNCTION)&Bitmap::gm_multiply, "multiply([Bitmap] mask, {float} offset, {float} scale)", "Multiplies the bitmap with the given bitmap mask.");
		BindFunction("saturateMask", (SCRIPT_FUNCTION)&Bitmap::gm_saturateMask, "saturateMask([Bitmap] mask, {float} offset, {float} scale)", "Changes saturation based on the given mask.");
		BindFunction("getWidth", (SCRIPT_FUNCTION)&Bitmap::gm_getWidth, "{int} getWidth()", "Gets the current width of the bitmap.");
		BindFunction("getHeight", (SCRIPT_FUNCTION)&Bitmap::gm_getHeight, "{int} getHeight()", "Gets the current height of the bitmap.");
		// TODO: getPixel, setPixel
	}

	~Bitmap() {
		if(image) {
			ImageDestroy(image);
			image = 0;
		}
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) {
			gm_load(a_thread);
			return GM_OK;
		}
		return GM_OK;
	}

	bool create(int width, int height) {
		if(image) {
			ImageDestroy(image);
			image = 0;
		}
		image = ImageCreate(width, height);
		if(!image) {
			Log(LOG_FATAL, "Failed to create a Bitmap image of dimensions %dx%d!");
			return false;
		}
		return true;
	}
	int gm_create(gmThread* a_thread) {
		GM_CHECK_FLOAT_OR_INT_PARAM(width, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(height, 1);
		if(!create((int)width, (int)height)) { return GM_EXCEPTION; }
		return ReturnThis(a_thread);
	}

	bool load(const char* file) {
		if(image) { ImageDestroy(image); }
		image = ImageLoad(_FILE(file));
		return image ? true : false;
	}
	int gm_load(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(file, 0);
		if(!load(file)) {
			a_thread->PushNull();
			return GM_EXCEPTION;
		}
		return ReturnThis(a_thread);
	}

	bool save(const char* file) {
		if(!image) {
			return false;
		}
		ImageSave(image, file);
		return true;
	}
	int gm_save(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt((int)save(file));
		return GM_OK;
	}

	bool clear(const char* color) {
		if(!image) {
			Log(LOG_ERROR, "Error while calling clear(). No image in Bitmap!");
			return false;
		}
		coDword dwColor = ColorParse(color);
		ImageFill(image, dwColor);
		return true;
	}
	int gm_clear(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(color, 0);
		if(!clear(color)) { return GM_EXCEPTION; }
		return ReturnThis(a_thread);
	}

	void flipY() {
		if(image) {
			ImageFlipY(image);
		}
	}
	int gm_flipY(gmThread* a_thread) {
		flipY();
		return ReturnThis(a_thread);
	}

	void flipX() {
		if(image) {
			ImageFlipX(image);
		}
	}
	int gm_flipX(gmThread* a_thread) {
		flipX();
		return ReturnThis(a_thread);
	}

	void swapRB() {
		if(image) {
			ImageSwapRB(image);
		}
	}
	int gm_swapRB(gmThread* a_thread) {
		swapRB();
		return ReturnThis(a_thread);
	}

	bool resize(int width, int height, bool bilinear) {
		if(image) {
			ImageResize(image, width, height, bilinear);
			return true;
		} else {
			return create(width, height);
		}
		return false;
	}
	int gm_resize(gmThread* a_thread) {
		if(!image) {
			return ReturnThis(a_thread);
		}

		int bilinear = 1;
		gmfloat w = (gmfloat)image->width, h = (gmfloat)image->height;

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
		resize((int)w,(int)h,bilinear);
		return ReturnThis(a_thread);
	}

	Bitmap* cut(int x, int y, int width, int height) {
		if(!image) {
			return 0;
		}
		IMAGE* cutImage = ImageGetCut(image, x, y, width, height);
		Bitmap* cutBitmap = new Bitmap();
		cutBitmap->image = cutImage;
		return cutBitmap;
	}
	int gm_cut(gmThread* a_thread) {
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(width, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(height, 3);

		Bitmap* cutBitmap = cut((int)x, (int)y, (int)width, (int)height);
		if(cutBitmap) {
			return cutBitmap->ReturnThis(a_thread);
		}
		return GM_OK;
	}

	Bitmap* duplicate() {
		return cut(0, 0,image->width, image->height);
	}
	int gm_duplicate(gmThread* a_thread) {
		Bitmap* cutBitmap = duplicate();
		if(cutBitmap) {
			return cutBitmap->ReturnThis(a_thread);
		}
		return GM_OK;
	}

	void paste(Bitmap* sourceBmp, int x, int y) {
		if(!sourceBmp) { return; }
		ImagePaste(image, sourceBmp->image, x, y, true);
	}
	int gm_paste(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Bitmap*, GM_TYPE_OBJECT, sourceBmp, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 2);
		paste(sourceBmp, (int)x, (int)y);
		return GM_OK;
	}

	void softenEdge(int edge) {
		if(!image) {
			return;
		}
		ImageSoftenEdge(image, edge);
	}
	int gm_softenEdge(gmThread* a_thread) {
		GM_CHECK_FLOAT_OR_INT_PARAM(edge, 0);
		softenEdge(edge);
		return GM_OK;
	}

	void multiply(Bitmap* mask, float offset, float scale) {
		if(mask->image->width != image->width || mask->image->height != image->height) {
			Bitmap* maskCopy = mask->duplicate();
			maskCopy->resize(image->width, image->height, true);
			ImageMultiply(image, maskCopy->image, offset, scale);
			delete(maskCopy);
		} else {
			ImageMultiply(image, mask->image, offset, scale);
		}
	}
	int gm_multiply(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Bitmap*, GM_TYPE_OBJECT, mask, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(offset, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(scale, 2);
		multiply(mask, offset, scale);
		return GM_OK;
	}

	void saturateMask(Bitmap* mask, float offset, float scale) {
		if(!image) {
			Log(LOG_ERROR, "Error while calling saturateMask(). No image in Bitmap!");
			return;
		}
		if(!mask)  {
			Log(LOG_ERROR, "Error while calling saturateMask(). No mask was given!");
			return;
		}

		if(mask->image->width != image->width || mask->image->height != image->height) {
			Log(LOG_WARNING, "Mask and image dimensions do not match. I must resize the mask now!");
			Bitmap* maskCopy = mask->duplicate();
			maskCopy->resize(image->width, image->height, true);
			ImageSaturateMask(image, maskCopy->image, offset, scale);
			delete(maskCopy);
		} else {
			ImageSaturateMask(image, mask->image, offset, scale);
		}
	}
	int gm_saturateMask(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Bitmap*, GM_TYPE_OBJECT, mask, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(offset, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(scale, 2);
		saturateMask(mask, offset, scale);
		return GM_OK;
	}

	int getWidth() {
		if(image) {
			return image->width;
		}
		return 0;
	}
	int gm_getWidth(gmThread* a_thread) {
		a_thread->PushInt(getWidth());
		return GM_OK;
	}

	int getHeight() {
		if(image) {
			return image->height;
		}
		return 0;
	}
	int gm_getHeight(gmThread* a_thread) {
		a_thread->PushInt(getHeight());
		return GM_OK;
	}

public:
	IMAGE* image;
};

#endif
