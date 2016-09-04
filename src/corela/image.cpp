#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "corela.h"

#define USE_OPENMP

#ifndef INCLUDE_HEADER_ONLY
#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.c"

#define CLAMP(x,min,max) {if(x<min)x=min;if(x>max)x=max;}


inline PIXEL* getPixel(IMAGE* image, int x, int y) {
	while(x < 0) {
		x += image->width;
	}
	while(y < 0) {
		y += image->height;
	}
	if(x >= image->width) {
		x = x % image->width;
	}
	if(y >= image->height) {
		y = y % image->height;
	}
	return &image->data[x+y*image->width];
}

inline PIXEL* getPixelUnsafe(IMAGE* image, int x, int y) {
	return &image->data[x+y*image->width];
}

inline void setPixel(IMAGE* image, int x, int y, PIXEL* pixel) {
	while(x < 0) {
		x += image->width;
	}
	while(y < 0) {
		y += image->height;
	}
	if(x >= image->width) {
		x = x % image->width;
	}
	if(y >= image->height) {
		y = y % image->height;
	}
	image->data[x+y*image->width] = *pixel;
}

inline void setPixel(IMAGE* image, int x, int y, BYTE r, BYTE g, BYTE b, BYTE a) {
	while(x < 0) {
		x += image->width;
	}
	while(y < 0) {
		y += image->height;
	}
	if(x >= image->width) {
		x = x % image->width;
	}
	if(y >= image->height) {
		y = y % image->height;
	}
	image->data[x+y*image->width].red = r;
	image->data[x+y*image->width].green = g;
	image->data[x+y*image->width].blue = b;
	image->data[x+y*image->width].alpha = a;
}

inline void setPixelUnsafe(IMAGE* image, int x, int y, BYTE r, BYTE g, BYTE b, BYTE a) {
	image->data[x+y*image->width].red = r;
	image->data[x+y*image->width].green = g;
	image->data[x+y*image->width].blue = b;
	image->data[x+y*image->width].alpha = a;
}

IMAGE* ImageCreate(int width, int height) {
	Log(LOG_DEBUG, "Creating image (%dx%d) ...", width, height);
	if(width <= 0 || height <= 0) {
		Log(LOG_ERROR, "Invalid dimension for image (%dx%d)!", width, height);
		return 0;
	}
	if(width > 8096 || height > 8096) {
		Log(LOG_WARNING, "Huge image dimension for new image (%dx%d).", width, height);
	}
	IMAGE* image = (IMAGE*)malloc(sizeof(IMAGE));
	image->width = width;
	image->height = height;
	image->data = (PIXEL*)malloc(sizeof(PIXEL) * width * height);
	if(image->data == 0) {
		Log(LOG_FATAL, "IMAGE CREATION (%dx%d) FAILED!", width, height);
	}
//	memset(image->data, 64, sizeof(PIXEL) * width * height);
	return image;
}

PIXEL* ImageGetPixel(IMAGE* image, int x, int y) {
	return getPixel(image, x, y);
}

void ImageSetPixel(IMAGE* image, int x, int y, PIXEL* pixel) {
	setPixel(image, x, y, pixel);
}

// TODO: Optimize pixel alpha mixing without breaking correctness (esp. with destination alpha!)
void ImageMixPixel2(IMAGE* image, int x, int y, PIXEL* src) {
	//ImageSetPixel(image, x, y, src);
	//return;

	PIXEL res;
	PIXEL* dst = getPixel(image, x, y);

	float f_src = (float)src->alpha / 255.0f;
	float f_dst = (float)dst->alpha / 255.0f;
	float f_total = f_src + f_dst;
	if(f_total == 0.0) {
		return;
	}
	if(f_dst == 0.0) {
		setPixel(image, x, y, src);
		return;
	}

	f_src /= f_total;
	f_dst /= f_total;

	CLAMP(f_src, 0.0f, 1.0f);
	CLAMP(f_dst, 0.0f, 1.0f);

	res.red   = (BYTE)(  (float)src->red*f_src   + (float)dst->red*f_dst  );
	res.green = (BYTE)(  (float)src->green*f_src + (float)dst->green*f_dst  );
	res.blue  = (BYTE)(  (float)src->blue*f_src  + (float)dst->blue*f_dst  );

	res.alpha = (dst->alpha + src->alpha) > 255 ? 255 : (dst->alpha + src->alpha);

	setPixel(image, x, y, &res);
}

// TODO: Optimize pixel alpha mixing without breaking correctness (esp. with destination alpha!)
void ImageMixPixel(IMAGE* image, int x, int y, PIXEL* src) {
	//ImageSetPixel(image, x, y, src);
	//return;

	PIXEL res;
	PIXEL* dst = getPixel(image, x, y);

	float f_src = (float)src->alpha / 255.0f;
	float f_dst = 1.0f - f_src;
	float f_total = f_src + f_dst;
	if(f_total == 0.0f) {
		return;
	}
	if(f_dst == 0.0f) {
		setPixel(image, x, y, src);
		return;
	}

	f_src /= f_total;
	f_dst /= f_total;

	CLAMP(f_src, 0.0, 1.0);
	CLAMP(f_dst, 0.0, 1.0);

	res.red   = (BYTE)(  (float)src->red*f_src   + (float)dst->red*f_dst  );
	res.green = (BYTE)(  (float)src->green*f_src + (float)dst->green*f_dst  );
	res.blue  = (BYTE)(  (float)src->blue*f_src  + (float)dst->blue*f_dst  );

	res.alpha = (dst->alpha + src->alpha) > 255 ? 255 : (dst->alpha + src->alpha);

	setPixel(image, x, y, &res);
}

void ImageDestroy(IMAGE* image) {
	free(image->data);
	free(image);
}

IMAGE* ImageGetCut(IMAGE* src, int px, int py, int width, int height) {
	IMAGE* dst = ImageCreate(width, height);

	int x,y;
	PIXEL* pix;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix)
#endif
	for(y=0; y<height; y++) {
		for(x=0; x<width; x++) {
			pix = getPixel(src, (int)(px+x), (int)(py+y));
			setPixel(dst, x, y, pix);
		}
	}

	return dst;
}

void ImagePaste(IMAGE* dst, IMAGE* src, int px, int py, bool alpha) {
	int x,y;
	PIXEL* pix;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix)
#endif
	for(y=0; y<src->height; y++) {
		for(x=0; x<src->width; x++) {
			pix = getPixel(src, x, y);
			if(alpha) {
				ImageMixPixel(dst, x+px, y+py, pix);
			} else {
				setPixel(dst, x+px, y+py, pix);
			}
		}
	}
}

// TODO: Optimize with OpenMP too.
/*void ImageMultiply(IMAGE* src, IMAGE* mul, float offset, float scale) {
	int x,y;
	PIXEL* pix;

	for(y=0; y<src->height; y++) {
		for(x=0; x<src->width; x++) {
			pix = getPixel(src, x, y);

			if(alpha) {
				ImageMixPixel(dst, x+px, y+py, pix);
			} else {
				setPixel(dst, x+px, y+py, pix);
			}
		}
	}
}*/

void ImageSoftenEdge(IMAGE* image, int edge) {
	BYTE alpha;
	PIXEL* pix;
	int val, lx, ly;

	for(int y=0; y<image->height; y++) {
		for(int x=0; x<image->width; x++) {
			alpha = 0xff;
			lx = image->width - x;
			ly = image->height - y;

			val = edge;
			if(x < val) {
				val = x;
			}
			if(y < val) {
				val = y;
			}
			if(lx < val) {
				val = lx;
			}
			if(ly < val) {
				val = ly;
			}

			if(val < edge) {
				alpha = (BYTE)(((float)val / (float)edge) * 255.0);
			}

			pix = getPixelUnsafe(image, x, y);
			pix->alpha = alpha;
//			ImageMixPixel(image, x, y, pix);
			setPixel(image, x, y, pix);
		}
	}
}

void ImageSwapRB(IMAGE* image) {
	PIXEL* t;
	BYTE p;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,t,p)
#endif
	for(int y=0; y<image->height; y++) {
		for(int x=0; x<image->width; x++) {
			t = getPixelUnsafe(image, x, y);
			p = t->red;
			t->red = t->blue;
			t->blue = p;
		}
	}
}

void ImageFlipX(IMAGE* src) {
	int x,y;
	PIXEL* pix;
	PIXEL* tmp;
	PIXEL alt;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix,tmp,alt)
#endif
	for(y=0; y<src->height; y++) {
		for(x=0; x<src->width/2; x++) {
			tmp = getPixelUnsafe(src, src->width-x-1, y);
			alt = *tmp;
			pix = getPixelUnsafe(src, x, y);
			setPixel(src, src->width-x-1, y, pix);
			setPixel(src, x, y, &alt);
		}
	}
}

void ImageFlipY(IMAGE* src) {
	int x,y;
	PIXEL* pix;
	PIXEL* tmp;
	PIXEL alt;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix,tmp,alt)
#endif
	for(y=0; y<src->height/2; y++) {
		for(x=0; x<src->width; x++) {
			tmp = getPixelUnsafe(src, x, src->height-y-1);
			alt = *tmp;
			pix = getPixelUnsafe(src, x, y);
			setPixel(src, x, src->height-y-1, pix);
			setPixel(src, x, y, &alt);
		}
	}
}

void ImageShadow(IMAGE* img, int offset, int blur, PIXEL* color) {

}

void ImageCopy(IMAGE* src, IMAGE* dst) {
	int x,y;
	PIXEL* pix;
	double scale_x = (double)src->width  / (double)dst->width;
	double scale_y = (double)src->height / (double)dst->height;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix)
#endif
	for(y=0; y<dst->height; y++) {
		for(x=0; x<dst->width; x++) {
			pix = getPixel(src, (int)(x * scale_x), (int)(y * scale_y));
			setPixel(dst, x, y, pix);
		}
	}
}

void ImageCopyBilinear(IMAGE* src, IMAGE* dst) {
	PIXEL res;
	double scale_x = (double)src->width  / (double)dst->width;
	double scale_y = (double)src->height / (double)dst->height;

	PIXEL* pix[4];
	float fac[4];
	int x=0, y=0;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix,fac,res)
#endif
	for(y=0; y<dst->height; y++) {
		//printf("|");
		for(x=0; x<dst->width; x++) {
			float fx = (float)(x * scale_x);
			float fy = (float)(y * scale_y);

			int pos_x = int(fx);
			int pos_y = int(fy);

			float frac_x = fx - (float)pos_x;
			float frac_y = fy - (float)pos_y;
			float minus_x = 1.0f - frac_x;
			float minus_y = 1.0f - frac_y;

			pix[0] = getPixel(src, pos_x, pos_y);
			pix[1] = getPixel(src, pos_x+1, pos_y);
			pix[2] = getPixel(src, pos_x, pos_y+1);
			pix[3] = getPixel(src, pos_x+1, pos_y+1);

			fac[0] = minus_x * minus_y;
			fac[1] = frac_x * minus_y;
			fac[2] = minus_x * frac_y;
			fac[3] = frac_x * frac_y;

			res.red   = (BYTE)(  (float)pix[0]->red*fac[0]   + (float)pix[1]->red*fac[1]   + (float)pix[2]->red*fac[2]   + (float)pix[3]->red*fac[3]  );
			res.green = (BYTE)(  (float)pix[0]->green*fac[0] + (float)pix[1]->green*fac[1] + (float)pix[2]->green*fac[2] + (float)pix[3]->green*fac[3]  );
			res.blue  = (BYTE)(  (float)pix[0]->blue*fac[0]  + (float)pix[1]->blue*fac[1]  + (float)pix[2]->blue*fac[2]  + (float)pix[3]->blue*fac[3]  );
			res.alpha = (BYTE)(  (float)pix[0]->alpha*fac[0] + (float)pix[1]->alpha*fac[1] + (float)pix[2]->alpha*fac[2] + (float)pix[3]->alpha*fac[3]  );

			setPixel(dst, x, y, &res);
		}
	}
}

void ImageResize(IMAGE* src, int w, int h, bool bilinear) {
	if(w <= 0 || h <= 0) {
		Log(LOG_DEBUG, "Invalid image size in ImageResize(), width: %d, height: %d.", w, h);
		return;
	}
	if(src->width == w && src->height == h) {
		return;
	}
	IMAGE* tmp = ImageCreate(w, h);
	if(bilinear) {
		ImageCopyBilinear(src, tmp);
	} else {
		ImageCopy(src, tmp);
	}
	free(src->data);
//	*src = *tmp;
	memcpy(src, tmp, sizeof(IMAGE));
	free(tmp);
}

void ImageMix(IMAGE* dst, IMAGE* src, float f) {
	PIXEL* p1, *p2;
	IMAGE* mix = src;
	if(src->width != dst->width || src->height != dst->height) {
		mix = ImageCreate(dst->width, dst->height);
		ImageCopyBilinear(src, mix);
	}

	float f1 = 1.0f - f;
	float f2 = f;
	int x=0,y=0;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,p1,p2)
#endif
	for(y=0; y<dst->height; y++) {
		for(x=0; x<dst->width; x++) {
			p1 = getPixel(dst, x, y);
			p2 = getPixel(mix, x, y);

			if(f == 0.5f) {
				p1->red = (p1->red + p2->red) / 2;
				p1->green = (p1->green + p2->green) / 2;
				p1->blue = (p1->blue + p2->blue) / 2;
				p1->alpha = (p1->alpha + p2->alpha) / 2;
			} else {
				p1->red   = (BYTE)(  (float)p1->red*f1   + (float)p2->red*f2  );
				p1->green = (BYTE)(  (float)p1->green*f1 + (float)p2->green*f2  );
				p1->blue  = (BYTE)(  (float)p1->blue*f1  + (float)p2->blue*f2  );
				p1->alpha = (BYTE)(  (float)p1->alpha*f1 + (float)p2->alpha*f2  );
			}
		}
	}
}

void ImageMultiply(IMAGE* dst, IMAGE* mask, float offset, float scale) {
	PIXEL* p1, *p2;
	int x=0,y=0;
	float r, g, b, rs, gs, bs;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,p1,p2,r,g,b,rs,gs,bs)
#endif
	for(y=0; y<dst->height; y++) {
		for(x=0; x<dst->width; x++) {
			p1 = getPixel(dst, x, y);
			p2 = getPixel(mask, x, y);

			r = (float)p1->red / 255.0f;
			g = (float)p1->green / 255.0f;
			b = (float)p1->blue / 255.0f;

			rs = (((float)p2->red   / 255.0f - offset) * scale) + offset * 2.0f;
			gs = (((float)p2->green / 255.0f - offset) * scale) + offset * 2.0f;
			bs = (((float)p2->blue  / 255.0f - offset) * scale) + offset * 2.0f;

			r = r*rs;
			g = g*gs;
			b = b*bs;

			CLAMP(r, 0.0, 1.0);
			CLAMP(g, 0.0, 1.0);
			CLAMP(b, 0.0, 1.0);

			p1->red   = (coByte) (r * 255.0f);
			p1->green = (coByte) (g * 255.0f);
			p1->blue  = (coByte) (b * 255.0f);

//			p1->red = (coByte)  ((  (float)(p1->red) / 255.0   *   ((float)(p2->red) / 255.0 - offset) * scale  ) * 255.0);
//			p1->green = (coByte)  ((  (float)(p1->green) / 255.0   *   ((float)(p2->green) / 255.0 - offset) * scale  ) * 255.0);
//			p1->blue = (coByte)  ((  (float)(p1->blue) / 255.0   *   ((float)(p2->blue) / 255.0 - offset) * scale  ) * 255.0);
		}
	}
}

void ImageSaturateMask(IMAGE* dst, IMAGE* mask, float offset, float scale) {
	PIXEL* p1, *p2;
	int x=0,y=0;
	float r, g, b, change;
	float P;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,p1,p2,r,g,b,change,P)
#endif
	for(y=0; y<dst->height; y++) {
		for(x=0; x<dst->width; x++) {
			p1 = getPixel(dst, x, y);
			p2 = getPixel(mask, x, y);

			r = (float)p1->red / 255.0f;
			g = (float)p1->green / 255.0f;
			b = (float)p1->blue / 255.0f;

			P = sqrt(r * r * 0.299f  +  g * g * 0.587f  + b * b * 0.114f);

			change = 1.0;
			change += (float)(p2->red + p2->green + p2->blue) / (255.0f*3.0f) * scale - offset * scale;

			r  = P + (r - P) * change;
			g  = P + (g - P) * change;
			b  = P + (b - P) * change;

			CLAMP(r, 0.0, 1.0f);
			CLAMP(g, 0.0, 1.0f);
			CLAMP(b, 0.0, 1.0f);

			p1->red   = (coByte) (r * 255.0f);
			p1->green = (coByte) (g * 255.0f);
			p1->blue  = (coByte) (b * 255.0f);
		}
	}
}

void ImageGreyscale(IMAGE* srcImage) {
	int x,y,yw;
	PIXEL* pix;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,yw,pix)
#endif
	for(y=0; y<srcImage->height; y++) {
		x = 0;
		yw = y*srcImage->width;
		pix = &srcImage->data[yw+x];

		for(; x<srcImage->width; x++) {
			//pix = &srcImage->data[x+yw];
			pix->red = pix->green = pix->blue = (pix->red + pix->green + pix->blue) / 3;
			pix++;
		}
	}
}

void ImageSetAlpha(IMAGE* srcImage, BYTE a) {
	int x,y,yw;
	PIXEL* pix;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,yw,pix)
#endif
	for(y=0; y<srcImage->height; y++) {
		x = 0;
		yw = y*srcImage->width;
		pix = &srcImage->data[yw+x];

		for(; x<srcImage->width; x++) {
			//pix = &srcImage->data[x+yw];
			pix->alpha = a;
			pix++;
		}
	}
}

void ImageFill(IMAGE* srcImage, coDword color) {
	coByte alpha = coByte((color & 0x000000FF));
	coByte red   = coByte((color & 0xFF000000) >> 24);
	coByte green = coByte((color & 0x00FF0000) >> 16);
	coByte blue  = coByte((color & 0x0000FF00) >> 8);

	int x,y,yw;
	PIXEL* pix;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,yw,pix)
#endif
	for(y=0; y<srcImage->height; y++) {
		x = 0;
		yw = y*srcImage->width;
		pix = &srcImage->data[yw+x];

		for(; x<srcImage->width; x++) {
			//pix = &srcImage->data[x+y*srcImage->width];
			pix->red = red;
			pix->green = green;
			pix->blue = blue;
			pix->alpha = alpha;
			pix++;
		}
	}
}

void ImageClampGray(IMAGE* srcImage, BYTE min, BYTE max) {
	int x,y;
	PIXEL* pix;
	BYTE v;

#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix,v)
#endif
	for(y=0; y<srcImage->height; y++) {
		for(x=0; x<srcImage->width; x++) {
			pix = &srcImage->data[x+y*srcImage->width];
			v = (pix->red + pix->green + pix->blue)/3;
			if(v < min) {
				v = min;
			}
			if(v > max) {
				v = max;
			}
			pix->red = pix->green = pix->blue = v;
		}
	}
}

void ImageSharpen(IMAGE* srcImage, IMAGE* dstImage, float sharpen) {
	PIXEL* pix, res;
	int r,g,b;
	int sh = 100 - (int)(sharpen * 100);
	int sp = 100 + (int)(sharpen * 100) * 8;

	int x,y;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix,res,r,g,b)
#endif
	for(y=0; y<srcImage->height; y++) {
		for(x=0; x<srcImage->width; x++) {
			pix = getPixel(srcImage, x-1, y-1);
			r = pix->red   * sh;
			g = pix->green * sh;
			b = pix->blue  * sh;

			pix = getPixel(srcImage, x, y-1);
			r += pix->red   * sh;
			g += pix->green * sh;
			b += pix->blue  * sh;

			pix = getPixel(srcImage, x+1, y-1);
			r += pix->red   * sh;
			g += pix->green * sh;
			b += pix->blue  * sh;

			pix = getPixel(srcImage, x-1, y);
			r += pix->red   * sh;
			g += pix->green * sh;
			b += pix->blue  * sh;

			pix = getPixel(srcImage, x+1, y);
			r += pix->red   * sh;
			g += pix->green * sh;
			b += pix->blue  * sh;

			pix = getPixel(srcImage, x-1, y+1);
			r += pix->red   * sh;
			g += pix->green * sh;
			b += pix->blue  * sh;

			pix = getPixel(srcImage, x, y+1);
			r += pix->red   * sh;
			g += pix->green * sh;
			b += pix->blue  * sh;

			pix = getPixel(srcImage, x+1, y+1);
			r += pix->red   * sh;
			g += pix->green * sh;
			b += pix->blue  * sh;

			pix = getPixel(srcImage, x, y);
			r += pix->red   * sp;
			g += pix->green * sp;
			b += pix->blue  * sp;

			r = r / 900;
			g = g / 900;
			b = b / 900;

			if(r < 0) {
				r = 0;
			}
			if(r > 255) {
				r = 255;
			}
			if(g < 0) {
				g = 0;
			}
			if(g > 255) {
				g = 255;
			}
			if(b < 0) {
				b = 0;
			}
			if(b > 255) {
				b = 255;
			}
			res.red = r;
			res.green = g;
			res.blue = b;
			res.alpha = pix->alpha;

			setPixel(dstImage, x, y, &res);
		}
	}
}

void ImageAlphaNoiseRemoval(IMAGE* srcImage, IMAGE* dstImage) {
	PIXEL* pix;
	int x,y,a;
#ifdef USE_OPENMP
	#pragma omp parallel for private(x,y,a,pix)
#endif
	for(y=0; y<srcImage->height; y++) {
		for(x=0; x<srcImage->width; x++) {
			pix = getPixel(srcImage, x, y-1);
			a = pix->alpha;

			pix = getPixel(srcImage, x+1, y-1);
			a += pix->alpha;

			pix = getPixel(srcImage, x+1, y);
			a += pix->alpha;

			pix = getPixel(srcImage, x+1, y+1);
			a += pix->alpha;

			pix = getPixel(srcImage, x, y+1);
			a += pix->alpha;

			pix = getPixel(srcImage, x-1, y+1);
			a += pix->alpha;

			pix = getPixel(srcImage, x-1, y);
			a += pix->alpha;

			pix = getPixel(srcImage, x-1, y-1);
			a += pix->alpha;

			pix = getPixel(srcImage, x, y);
			//a += pix->alpha;
			//if(pix->alpha == 0) c++;

			//setPixel(dstImage, x, y, pix->red, pix->green, pix->blue, a > 255*6 ? 255 : 0);
			setPixel(dstImage, x, y, pix->red, pix->green, pix->blue, a / 8);
		}
	}
}

void ImageMedian(IMAGE* srcImage, IMAGE* dstImage, int filterSize) {
	if(filterSize > 64) {
		filterSize=64;
	}
	PIXEL* colors[64*64]; // Max. filter size is 64!!!
	int values[64*64];

	int x=0,y=0;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,colors,values)
#endif
	for(y=0; y<dstImage->height; y++) {
		for(x=0; x<dstImage->width; x++) {
			int nv=0;
			for(int fy=-filterSize; fy<=filterSize; fy++) {
				for(int fx=-filterSize; fx<=filterSize; fx++) {
					colors[nv] = getPixel(srcImage, x+fx, y+fy);
					values[nv] = colors[nv]->red + colors[nv]->green + colors[nv]->blue;
					nv++;
				}
			}

			int found = 0;
			for(int i=0; i<nv/2; i++) {
				int lowest = 255*3;
				for(int n=0; n<nv; n++) {
					if(values[n] < lowest) {
						found = n;
						lowest = values[n];
					}
				}
				values[found] = 255*3;
			}
			//if(found)
			setPixel(dstImage, x, y, colors[found]);
		}
	}
}

inline BYTE _PackFloatInByte(float in) {
	return (BYTE) ((in+1.0f) / 2.0f * 255.0f);
}
void ImageNormalMap(IMAGE* srcImage, IMAGE* dstImage, float scale, int specularAlpha) {
	float dX, dY, nX, nY, nZ, oolen;
	PIXEL* pix;

	int x=0,y=0;
	PIXEL res;
#ifdef USE_OPENMP
	#pragma omp parallel for private(y,x,pix,res,dX,dY,nX,nY,nZ,oolen)
#endif
	for(y=0; y<dstImage->height; y++) {
		//printf(".");
		for(x=0; x<dstImage->width; x++) {
			//pix = getPixel(srcImage, x, y);
			//setPixel(dstImage, x, y, pix);
			//continue;

			// Do Y Sobel filter
			pix = getPixel(srcImage, x-1, y+1);
			dY  = ((float) pix->red) / 255.0f * -1.0f;

			pix = getPixel(srcImage, x, y+1);
			dY += ((float) pix->red) / 255.0f * -2.0f;

			pix = getPixel(srcImage, x+1, y+1);
			dY += ((float) pix->red) / 255.0f * -1.0f;

			pix = getPixel(srcImage, x-1, y-1);
			dY += ((float) pix->red) / 255.0f * 1.0f;

			pix = getPixel(srcImage, x, y-1);
			dY += ((float) pix->red) / 255.0f * 2.0f;

			pix = getPixel(srcImage, x+1, y-1);
			dY += ((float) pix->red) / 255.0f * 1.0f;

			// Do X Sobel filter
			pix = getPixel(srcImage, x-1, y-1);
			dX  = ((float) pix->red) / 255.0f * -1.0f;

			pix = getPixel(srcImage, x-1, y);
			dX += ((float) pix->red) / 255.0f * -2.0f;

			pix = getPixel(srcImage, x-1, y+1);
			dX += ((float) pix->red) / 255.0f * -1.0f;

			pix = getPixel(srcImage, x+1, y-1);
			dX += ((float) pix->red) / 255.0f *  1.0f;

			pix = getPixel(srcImage, x+1, y);
			dX += ((float) pix->red) / 255.0f *  2.0f;

			pix = getPixel(srcImage, x+1, y+1);
			dX += ((float) pix->red) / 255.0f *  1.0f;

			// Cross Product of components of gradient reduces to
			//nX = dX*scale;
			//nY = -dY*scale;
			nX = dX*scale;
			nY = -dY*scale;
			nZ = 1;

			// Normalize
			oolen = 1.0f/((float) sqrtf (nX*nX + nY*nY + nZ*nZ));
			nX *= oolen;
			nY *= oolen;
			nZ *= oolen;

			res.red   = (BYTE)_PackFloatInByte(nX);
			res.green = (BYTE)_PackFloatInByte(nY);
			res.blue  = (BYTE)_PackFloatInByte(nZ);
			res.alpha = 0xff;

			if(specularAlpha) {
				float a = 1.0f - (float)(res.red + res.blue) / 512.0f;
				//float a = 1.0f - (fabsf(nX) + fabsf(nY)) / 2.0f;
				a = fabsf(a-0.5f);
				a = a * 4.0f;
				if(a > 1.0) {
					a = 1.0f;
				}
				//a = 1.0f - a;
				a = (a - 0.5f) * 2.0f;
				if(a > 1.0) {
					a = 1.0f;
				}
				res.alpha = (BYTE)_PackFloatInByte(a);
			}

			setPixel(dstImage, x, y, &res);
		}
	}
}

// use libpng here for maximum performance and compatibility
#include <zlib/png.h>
void ImageSavePNG(IMAGE* image, const char* filename) {
	// Open file for saving
	FILE* hf = fopen(filename, "wb");
	if (!hf) { return; }

	// Create required PNG structs
	png_structp pPNG = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop pInfo = png_create_info_struct(pPNG);

	// Set error handling jump for libPNG
	if(setjmp(png_jmpbuf(pPNG))) {
		png_destroy_write_struct(&pPNG, &pInfo);
		fclose(hf);
		return;
	}

	// Setup PNG for writing
	png_set_compression_level(pPNG, 0);
	png_set_filter(pPNG, 0, PNG_FILTER_NONE);
	png_init_io(pPNG, hf);
	png_set_IHDR(pPNG, pInfo, image->width, image->height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
//	png_set_invert_alpha(pPNG);
//	png_set_bgr(pPNG);
	png_write_info(pPNG, pInfo);
//	png_set_flush(pPNG, 25);

	// Write all rows
	for(int n = 0; n < image->height; n++) {
		png_write_row(pPNG, (png_bytep)&image->data[n*image->width]);
	}
	png_write_flush(pPNG);

	// Finish writing PNG
	png_write_end(pPNG, pInfo);
	png_destroy_write_struct(&pPNG, &pInfo);

	// Close file
	fclose(hf);

	return;
}
bool ImageLoadPNG(IMAGE* image, const char* filename) {
	// open file for reading
	//FILE* hf = fopen(filename, "rb");
	FILE* hf;
	
#ifdef _WIN32
	int err = fopen_s(&hf, filename, "rb");
	if (!hf) { 
		Log(LOG_ERROR, "Error while opening the file'%s': %s", filename, strerror(err));
		return false; }
#else
	hf = fopen(filename, "rb");
	if(!hf) {
		Log(LOG_ERROR, "Error while opening the file'%s'!", filename);
		return false; }
#endif
	
	// check header
	char header[8];
	fread(header, 1, 8, hf);
	if (png_sig_cmp((png_const_bytep)header, 0, 8)) { fclose(hf); return false; }

	// create png structs
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop info_ptr = png_create_info_struct(png_ptr);

	// set error handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(hf);
		return false;
	}

	// init
	png_init_io(png_ptr, hf);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	// read info
	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);
	png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	int number_of_passes = png_set_interlace_handling(png_ptr);

	// expand palettes
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
	}

	// ensure color type RGBA
	if (color_type == PNG_COLOR_TYPE_RGB) {
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
	}

	// ensure bit-depth 8
	if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
	}

	// create image memory
	image->width = width;
	image->height = height;
	image->data = (PIXEL*)malloc(sizeof(PIXEL) * width * height);

	// read image
	png_bytep* row_pointers;
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y<height; y++)
		row_pointers[y] = (png_byte*)&image->data[y*image->width];
	png_read_image(png_ptr, row_pointers);
	
	// cleanup
	if (fclose(hf) != 0) {
		Log(LOG_ERROR, "Error while closing file '%s'!", filename);
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	return true;
}

IMAGE* ImageLoad(const char* filename) {
	int comp;
	IMAGE* image = (IMAGE*)malloc(sizeof(IMAGE));
	Log(LOG_INFO, "Loading image '%s'.", filename);
	if (!ImageLoadPNG(image, filename)) {
		Log(LOG_INFO, "  Using STBI '%s'.", filename);
		image->data = (PIXEL*)stbi_load(filename, &image->width, &image->height, &comp, 4);
		if (!image->data) {
			Log(LOG_ERROR, "FAILED TO LOAD IMAGE '%s'!", filename);
			free(image);
			return 0;
		}
	}
	//Log(LOG_INFO, "ok.");
	return image;
}

void ImageSave(IMAGE* image, const char* filename) {
	stbi_write_png(filename, image->width, image->height, 4, image->data, 0);
}

#endif
