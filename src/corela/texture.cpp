#include "corela.h"
#include "glutils.h"
#include <stdio.h>

#define PI 3.14159265f


bool ColorParse(const char* color, float* r, float* g, float* b, float* a) {
	if(r) {
		*r = 0.0;
	}
	if(g) {
		*g = 0.0;
	}
	if(b) {
		*b = 0.0;
	}
	if(a) {
		*a = 1.0;
	}

	if(!color) {
		return false;
	}
	if(strlen(color) < 2) {
		return false;
	}

	char red[3], green[3], blue[3], alpha[3];
	if(color[0] == '#' && !isalnum(color[0])) {
		color++;
	}

	if(strlen(color) >= 8) {
		alpha[0] = color[6];
		alpha[1] = color[7];
		alpha[2] = 0;
	} else {
		alpha[0] = 'f';
		alpha[1] = 'f';
		alpha[2] = 0;
	}

	if(strlen(color) >= 2) {
		red[0] = color[0];
		red[1] = color[1];
		red[2] = 0;
	}
	if(strlen(color) >= 4) {
		color += 2;
	}

	if(strlen(color) >= 2) {
		green[0] = color[0];
		green[1] = color[1];
		green[2] = 0;
	}
	if(strlen(color) >= 4) {
		color += 2;
	}

	if(strlen(color) >= 2) {
		blue[0] = color[0];
		blue[1] = color[1];
		blue[2] = 0;
	}

	int _r = 0, _g = 0, _b = 0, _a = 255;

	if(r) {
		sscanf(red, "%x", &_r);
	}
	if(g) {
		sscanf(green, "%x", &_g);
	}
	if(b) {
		sscanf(blue, "%x", &_b);
	}
	if(a) {
		sscanf(alpha, "%x", &_a);
	}

	*r = (float)_r / (float)255;
	*g = (float)_g / (float)255;
	*b = (float)_b / (float)255;
	*a = (float)_a / (float)255;

	return true;
}

coDword ColorParse(const char* _in) {
	if(!_in) {
		return 0x000000ff;
	}
	if(_in[0] == '#' && !isalnum(_in[0])) {
		_in++;
	}
	if(strlen(_in) < 2) {
		return 0;
	}

	char col[9];
	col[8] = 0;
	col[0] = _in[0];
	col[1] = _in[1];

	if(strlen(_in) >= 4) {
		_in += 2;
	}
	col[2] = _in[0];
	col[3] = _in[1];

	if(strlen(_in) >= 4) {
		_in += 2;
	}
	col[4] = _in[0];
	col[5] = _in[1];

	if(strlen(_in) >= 4) {
		_in += 2;
		col[6] = _in[0];
		col[7] = _in[1];
	} else {
		col[6] = 'f';
		col[7] = 'f';
	}

	coDword c;
	sscanf(col, "%x", &c);
	return c;
}

TEXTURE* TextureCreate(coDword width, coDword height, coDword flags, const char* clearColor) {
	// fix invalid dimensions
	if(width <= 0 || width > 8096) { width = 1; }
	if(height <= 0 || width > 8096) { height = 1; }

	BEGIN_CHECK_GL_ERROR();

	// Create texture object
	TEXTURE* tx = (TEXTURE*)malloc(sizeof(TEXTURE));
	memset(tx, 0, sizeof(TEXTURE));

	// setup struct
	tx->target = GL_TEXTURE_2D;
	tx->flags = flags;
	tx->vwidth = width;
	tx->vheight = height;
	tx->width = width = glCapsCompatTextureSize(width);
	tx->height = height = glCapsCompatTextureSize(height);
	Log(LOG_DEBUG, "Creating texture with dimension %dx%d.", width, height);

	// setup source drawing rectangle
	tx->drawRect.left = 0.0;
	tx->drawRect.right = 1.0;
	tx->drawRect.top = 0.0;
	tx->drawRect.bottom = 1.0;

	// use texture compression, if requested and supported
	if(flags & TEX_COMPRESS && glUtilsHasExtension("ARB_texture_compression")) {
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);
		if(flags & TEX_NOALPHA) {
			tx->internalFormat = GL_COMPRESSED_RGB;
		} else {
			tx->internalFormat = GL_COMPRESSED_RGBA;
		}
	} else {
		if(flags & TEX_NOALPHA) {
			tx->internalFormat = GL_RGB;
		} else {
			tx->internalFormat = GL_RGBA;
		}
	}

	// generate texture and set parameters
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tx->handle);
	glBindTexture(tx->target, tx->handle);

	// set texture filter/mipmap filter
	if(flags & TEX_NOMIPMAP) {
		if(flags & TEX_NOFILTER) {
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(glCaps.maxAnisotropy) {
				glTexParameteri(tx->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, glCaps.maxAnisotropy);
			}
		}
	} else {
		if(flags & TEX_NOFILTER) {
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(glCaps.maxAnisotropy) {
				glTexParameteri(tx->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, glCaps.maxAnisotropy);
			}
		}
	}

	// set texture lod bias for "sharp" textures
	if(flags & TEX_SHARP && glUtilsHasExtension("EXT_texture_lod_bias")) {
		glTexParameterf(tx->target, GL_TEXTURE_LOD_BIAS, -0.5f);
	}

	// set texture clamping
	if(flags & TEX_CLAMP) {
		glTexParameteri(tx->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(tx->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// clear texture when requested
	if(flags & TEX_CLEAR) {
		TextureClear(tx, ColorParse(clearColor));
	}

	// create PBO on request
	if(flags & TEX_PBO) {
		glBindTexture(GL_TEXTURE_2D, tx->handle);
		glGenBuffers(1, &tx->pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, tx->pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, width*height*4, 0, GL_STREAM_DRAW_ARB);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	}

	END_CHECK_GL_ERROR("TextureCreate");

	// set inital texture content
/*	IMAGE* image = ImageCreate(width, height);
	if(clearColor) {
		ImageFill(image, ColorParse(clearColor));
	}
//	TextureSetImage(tx, image);
	ImageDestroy(image);*/

	return tx;
}

void TextureDestroy(TEXTURE* tex) {
	if(tex->pLockMem) {
		free(tex->pLockMem);
	}
	if(tex->pbo) {
		glDeleteBuffers(1, &tex->pbo);
	}
	if(glIsTexture(tex->handle)) {
		glDeleteTextures(1, &tex->handle);
	}
	free(tex);
}

void TextureClear(TEXTURE* tex, coDword color) {
	// TODO: Optimize and use PBO if possible.
	IMAGE* im = ImageCreate(tex->width, tex->height);
	TextureGetImage(tex, im);
	ImageFill(im, color);
	TextureSetImage(tex, im);
	ImageDestroy(im);
}

TEXTURE* TextureLoad(const char* file, coDword flags) {
	BEGIN_CHECK_GL_ERROR();

	IMAGE* image = ImageLoad(file);
	if(!image) {
		Log(LOG_ERROR, "Error while loading texture '%s'.", file);
		return 0;
	}
	Log(LOG_INFO, "Image '%s' loaded (%dx%d)", file, image->width, image->height);

	TEXTURE* tx = TextureCreate(image->width, image->height, flags, 0);
	if(!tx) {
		Log(LOG_ERROR, "Error while creating texture for '%s' (%dx%d).", file, image->width, image->height);
		ImageDestroy(image);
		return 0;
	}

	if(image->width != tx->width || image->height != tx->height) {
		Log(LOG_INFO, "Resizing image '%s' (%dx%d) to %dx%d", file, image->width, image->height, tx->width, tx->height);
		ImageResize(image, tx->width, tx->height, true);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, tx->internalFormat, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->width, image->height, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

	if( !(flags & TEX_NOMIPMAP)) {
		glUtilsGenerateMipmap(GL_TEXTURE_2D);
	}

	ImageDestroy(image);
	END_CHECK_GL_ERROR("TextureLoad");
	return tx;
}

void TextureSave(TEXTURE* tex, const char* file, int width, int height) {
	if(width <= 0) { width = tex->width; }
	if(height <= 0) { height = tex->height; }

	if (width == tex->width && height == tex->height) {
		IMAGE img;
		img.width = width;
		img.height = height;
		img.data = (PIXEL*)TextureLock(tex, true);
		ImageSavePNG(&img, file);
		TextureUnlock(tex, false);
		return;
	}

	float ya = tex->drawRect.bottom - tex->drawRect.top / (float)height;
	float xa = tex->drawRect.right - tex->drawRect.left / (float)width;
	float y = tex->drawRect.top;
	float x = tex->drawRect.left;
	int px, py;

	IMAGE* imgSrc = ImageCreate(tex->width, tex->height);
	TextureGetImage(tex, imgSrc);

	IMAGE* imgDst = ImageCreate(width, height);

	for(int ny = 0; ny < height; ny++) {
		for(int nx = 0; nx < width; nx++) {
			// TODO: Get interpolated pixel
			int px = (int)(x);
			int py = (int)(y);
			PIXEL* pix = ImageGetPixel(imgSrc, px, py);

			if(tex->pbo) {
				BYTE t = pix->red;
				pix->red = pix->blue;
				pix->blue = t;
			}

			ImageSetPixel(imgDst, nx, ny, pix);
			x += xa;
		}
		x = tex->drawRect.left;
		y += ya;
	}

	ImageSavePNG(imgDst, file);
	ImageDestroy(imgDst);
	ImageDestroy(imgSrc);
}

coByte* TextureLock(TEXTURE* tex, coBool read, int format) {

	// TODO: With PBO how aboud reading/non-reading/writing???
	if(tex->pbo) {
		BEGIN_CHECK_GL_ERROR();

		glBindTexture(GL_TEXTURE_2D, tex->handle);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, tex->pbo);
//		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->width, tex->height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(tex->target, 0, tex->internalFormat, tex->width, tex->height, 0, format, GL_UNSIGNED_BYTE, 0);
		glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, tex->width * tex->height * 4, 0, GL_STREAM_DRAW_ARB);
		GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
//		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		END_CHECK_GL_ERROR("TextureLock");

		return (coByte*)ptr;
	} else {
		BEGIN_CHECK_GL_ERROR();

		if(tex->pLockMem) {
			free(tex->pLockMem);
		}
		tex->pLockMem = (coByte*)malloc(tex->width * tex->height * 4);
//		memset(tex->pLockMem, 0x00, tex->width * tex->height * 4);

		if(read) {
			glBindTexture(tex->target, tex->handle);
			// TODO: glGetTexImage does not work on Udoo!?!
			glGetTexImage(tex->target, 0, format, GL_UNSIGNED_BYTE, tex->pLockMem);
		}

		END_CHECK_GL_ERROR("TextureLock");

		return tex->pLockMem;
	}
}

void TextureUnlock(TEXTURE* tex, coBool write, int format) {
	if(tex->pbo) {
		glBindTexture(GL_TEXTURE_2D, tex->handle);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, tex->pbo);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);
//		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->width, tex->height, tex->internalFormat, GL_UNSIGNED_BYTE, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	} else {
		if(write) {
			TextureUpload(tex, tex->pLockMem, format);
		}
		free(tex->pLockMem);
		tex->pLockMem = 0;
	}
}

void TextureUpload(TEXTURE* tex, coByte* data, int format) {
	// TODO: Optimize uploads via PBOs.
	BEGIN_CHECK_GL_ERROR();

	glEnable(tex->target);
	glBindTexture(tex->target, tex->handle);
	glTexImage2D(tex->target, 0, tex->internalFormat, tex->width, tex->height, 0, format, GL_UNSIGNED_BYTE, data);
	if(!(tex->flags & TEX_NOMIPMAP)) { glUtilsGenerateMipmap(GL_TEXTURE_2D); }

	END_CHECK_GL_ERROR("TextureUpload");
}

void TextureGetImage(TEXTURE* tex, IMAGE* im, int format) {
	// resize image if neccessary
	if(im->width != tex->width || im->height != tex->height) {
		ImageResize(im, tex->width, tex->height, false);
	}

	coByte* data = TextureLock(tex, true, format);
	memcpy(im->data, data, im->width * im->height * 4);
	TextureUnlock(tex, false, format);
}

void TextureSetImage(TEXTURE* tex, IMAGE* im, int format) {
	// get compatible texture size
	int w = glCapsCompatTextureSize(im->width);
	int h = glCapsCompatTextureSize(im->height);

	tex->width = w;
	tex->height = h;
	//tex->vwidth = im->width;
	//tex->vheight = im->height;

	// upload image to texture
	TextureUpload(tex, (coByte*)im->data, format);
}

TEXTURE* TextureDuplicate(TEXTURE* source) {
	TEXTURE* dup = TextureCreate(source->width, source->height, source->flags, 0);

	coByte* bufferSrc = TextureLock(source, true);
	TextureUpload(dup, bufferSrc);
	TextureUnlock(source, false);

	return dup;
}
