#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <corela.h>
#include <cairo/cairo.h>
#include "Texture.h"
#include "Font.h"


class Canvas : public Texture {
public:

	Canvas() : Texture() {
		id = "Canvas";
		help = "A texture-based node object that can be drawn onto.";

		//textureFlags = TEX_NOMIPMAP | TEX_NOFILTER | TEX_CLAMP;
		texture = 0;
		image = 0;
		surface = 0;
		pattern = 0;
		context = 0;

		BindFunction("setSolid", (SCRIPT_FUNCTION)&Canvas::gm_setSolid, "[this] setSolid({string} color)", "Sets the fill and stroke colors to a solid color.");
		BindFunction("setLinear2", (SCRIPT_FUNCTION)&Canvas::gm_setLinear2, "[this] setLinear2({string} color1, {string} color2, (optional) {float} horizonalScale, (optional) {float} verticalScale)", "Sets the fill and stroke colors to a linear color pattern, the direction is taken from the scale parameter.");
		BindFunction("newPath", (SCRIPT_FUNCTION)&Canvas::gm_newPath, "[this] newPath()");
		BindFunction("closePath", (SCRIPT_FUNCTION)&Canvas::gm_closePath, "[this] closePath()");
		BindFunction("fill", (SCRIPT_FUNCTION)&Canvas::gm_fill, "[this] fill()", "Fills the last path with the set color or pattern.");
		BindFunction("stroke", (SCRIPT_FUNCTION)&Canvas::gm_stroke, "[this] stroke({float} lineWidth)", "Outlines the last path with the set color or pattern.");
		BindFunction("moveTo", (SCRIPT_FUNCTION)&Canvas::gm_moveTo, "[this] moveTo({float} x, {float} y)", "Moves the path cursor to the given position.");
		BindFunction("lineTo", (SCRIPT_FUNCTION)&Canvas::gm_lineTo, "[this] lineTo({float} x, {float} y)", "Draws a line from the current position to the given position.");
		BindFunction("curveTo", (SCRIPT_FUNCTION)&Canvas::gm_curveTo, "[this] curveTo({float} x1, {float} y1, {float} x2, {float} y2, {float} x3, {float} y3)", "Draws a curve from the current position to the given position.");
		BindFunction("rectangle", (SCRIPT_FUNCTION)&Canvas::gm_rectangle, "[this] rectangle({float} x, {float} y, {float} width, {float} height)", "Draws a rectangle.");
		BindFunction("roundedRectangle", (SCRIPT_FUNCTION)&Canvas::gm_roundedRectangle, "[this] roundedRectangle({float} x, {float} y, {float} width, {float} height, {float} cornerRadius)");
		BindFunction("arc", (SCRIPT_FUNCTION)&Canvas::gm_arc, "[this] arc({float} centerX, {float} centerY, {float} radius, {float} angle1, {float} angle2)", "Draws an arc or a circle.");
		BindFunction("clear", (SCRIPT_FUNCTION)&Canvas::gm_clear, "[this] clear()", "Clears the canvas with the set clear color.");
//		BindFunction("flush", (SCRIPT_FUNCTION)&Canvas::gm_flush, "[this] flush()");
//		BindFunction("drawLine", (SCRIPT_FUNCTION)&Canvas::gm_drawLine);
		BindFunction("drawText", (SCRIPT_FUNCTION)&Canvas::gm_drawText, "[this] drawText([Font] font, {string} text, {int} x, {int} y, {string} solidColor, (optional) {int} outlineWidth, (optional) {string} outlineColor)", "Draws text on the canvas.");
	}

	~Canvas() {
		if(pattern) { cairo_pattern_destroy(pattern); }
		if(context) { cairo_destroy(context); }
		if(surface) { cairo_surface_destroy(surface); }
		if(image)   { ImageDestroy(image); }
	}

	void _check() {
		if(!texture) {
			// create texture
			texture = TextureCreate(dimension->x, dimension->y, textureFlags, clearColor.c_str());
		}
		if(!image) {
			// create image buffer
			image = ImageCreate(texture->width, texture->height);
			ImageFill(image, ColorParse(clearColor.c_str()));
		}
		if(!surface) {
			// create cairo surface and context
			if (context) { cairo_destroy(context); }
			surface = cairo_image_surface_create_for_data((coByte*)image->data, CAIRO_FORMAT_ARGB32, image->width, image->height, image->width * 4);
			context = cairo_create(surface);
			cairo_set_line_cap(context, CAIRO_LINE_CAP_ROUND);
			cairo_set_line_join(context, CAIRO_LINE_JOIN_ROUND);
			cairo_set_antialias(context, CAIRO_ANTIALIAS_GOOD);
		}
	}

	void _upload() {
		// cairo surfaces are BGRA (pre-multiplied since cairo 1.0)
		// TODO: account for pre-multiplied alpha
		_check();
		TextureUpload(texture, cairo_image_surface_get_data(surface), TEX_FORMAT_BGRA);
	}

	virtual bool load(const char* file) {
		clear();
		image = ImageLoad(file);
		if(!image) {
			Log(LOG_ERROR, "Image '%s' not found!", file);
			return false;
		} else {
			Log(LOG_DEBUG, "Image '%s' loaded successfully.", file);
		}
		ImageSwapRB(image);
		dimension->x = image->width;
		dimension->y = image->height;
		_upload();
		return true;
	}

	void clear() {
		if (surface) {
			cairo_surface_destroy(surface); surface = 0;
			cairo_destroy(context); context = 0;
			_check();
		}
		/*if (context) {
			cairo_paint(context);
		}*/
	}
	int gm_clear(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) {
			GM_CHECK_STRING_PARAM(_color, 0);
			clearColor = _color;
		}
		clear();
		return ReturnThis(a_thread);
	}

	void renew() {
		if (context) { cairo_destroy(context); context = 0; }
		if (surface) { cairo_surface_destroy(surface); surface = 0; }
		if (image) { ImageDestroy(image); image = 0; }
		if (texture) { TextureDestroy(texture); texture = 0; }
		//		_check();
	}
	int gm_renew(gmThread* a_thread) {
		renew();
		return ReturnThis(a_thread);
	}

	void setSolid(const char* color) {
		_check();
		float r,g,b,a;
		if(ColorParse(color, &r, &g, &b, &a)) {
			if(pattern) { cairo_pattern_destroy(pattern); }
			cairo_set_source_rgba(context, r,g,b,a);
		}
	}
	int gm_setSolid(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(color, 0);
		setSolid(color);
		return ReturnThis(a_thread);
	}

	void setLinear2(const char* color1, const char* color2, float horizontal, float vertical) {
		_check();
		if(pattern) { cairo_pattern_destroy(pattern); }
		cairo_pattern_t* pattern = cairo_pattern_create_linear(0.0, 0.0, dimension->x * horizontal, dimension->y * vertical);
		float r,g,b,a;
		if(ColorParse(color1, &r, &g, &b, &a)) {
			cairo_pattern_add_color_stop_rgba(pattern, 0, r, g, b, a);
		}
		if(ColorParse(color2, &r, &g, &b, &a)) {
			cairo_pattern_add_color_stop_rgba(pattern, 1, r, g, b, a);
		}
		cairo_set_source(context, pattern);
	}
	int gm_setLinear2(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);

		GM_CHECK_STRING_PARAM(color1, 0);
		GM_CHECK_STRING_PARAM(color2, 1);

		gmfloat horizontal = 0.0, vertical = 1.0;
		a_thread->ParamFloatOrInt(2, horizontal, 0.0);
		a_thread->ParamFloatOrInt(3, vertical, 1.0);

		setLinear2(color1, color2, horizontal, vertical);
		return ReturnThis(a_thread);
	}

	void newPath() {
		_check();
		cairo_new_path(context);
	}
	int gm_newPath(gmThread* a_thread) {
		newPath();
		return ReturnThis(a_thread);
	}

	void closePath() {
		_check();
		cairo_close_path(context);
	}
	int gm_closePath(gmThread* a_thread) {
		closePath();
		return ReturnThis(a_thread);
	}

	void fill() {
		_check();
		cairo_fill_preserve(context);
		_upload();
	}
	int gm_fill(gmThread* a_thread) {
		fill();
		return ReturnThis(a_thread);
	}

	void stroke(float lineWidth) {
		_check();
		cairo_set_line_width(context, lineWidth);
		cairo_stroke_preserve(context);
		_upload();
	}
	int gm_stroke(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_FLOAT_OR_INT_PARAM(lineWidth, 0);
		stroke(lineWidth);
		return ReturnThis(a_thread);
	}

	void moveTo(float x, float y) {
		_check();
		cairo_move_to(context, x, y);
	}
	int gm_moveTo(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);
		moveTo(x, y);
		return ReturnThis(a_thread);
	}

	void lineTo(float x, float y) {
		_check();
		cairo_line_to(context, x, y);
	}
	int gm_lineTo(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);
		lineTo(x, y);
		return ReturnThis(a_thread);
	}

	void arc(float xc, float yc, float radius, float angle1, float angle2) {
		_check();
		cairo_arc(context, xc, yc, radius, angle1, angle2);
	}
	int gm_arc(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(5);
		GM_CHECK_FLOAT_OR_INT_PARAM(xc, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(yc, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(radius, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(angle1, 3);
		GM_CHECK_FLOAT_OR_INT_PARAM(angle2, 4);
		arc(xc, yc, radius, angle1, angle2);
		return ReturnThis(a_thread);
	}

	void curveTo(float x1, float y1, float x2, float y2, float x3, float y3) {
		_check();
		cairo_curve_to(context, x1, y1, x2, y2, x3, y3);
	}
	int gm_curveTo(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(6);
		GM_CHECK_FLOAT_OR_INT_PARAM(x1, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y1, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(x2, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(y2, 3);
		GM_CHECK_FLOAT_OR_INT_PARAM(x3, 4);
		GM_CHECK_FLOAT_OR_INT_PARAM(y3, 5);
		curveTo(x1, y1, x2, y2, x3, y3);
		return ReturnThis(a_thread);
	}

	void rectangle(float x, float y, float width, float height) {
		_check();
		cairo_rectangle(context, x, y, width, height);
	}
	int gm_rectangle(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(width, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(height, 3);
		rectangle(x, y, width, height);
		return ReturnThis(a_thread);
	}

	void roundedRectangle(float x, float y, float width, float height, float corner) {
		float aspect = height / width;
		// TODO: compute better radius, this one does not work right
		float radius = corner;
		float degrees = 3.14159265358979323846f / 180.0f;

		//cairo_new_sub_path(context);
		cairo_arc(context, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
		cairo_arc(context, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
		cairo_arc(context, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
		cairo_arc(context, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
		//cairo_close_path(context);
	}
	int gm_roundedRectangle(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(5);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(width, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(height, 3);
		GM_CHECK_FLOAT_OR_INT_PARAM(cornerRadius, 4);
		roundedRectangle(x, y, width, height, cornerRadius);
		return ReturnThis(a_thread);
	}

	void drawImage(IMAGE* im) {

	}

	/*	void drawLine(float x1, float y1, float x2, float y2) {
			_check();

			cairo_set_source_rgba(context, 0.0, 0.0, 0.0, 0.5);
			cairo_set_line_cap(context, CAIRO_LINE_CAP_ROUND);
			cairo_set_line_join(context, CAIRO_LINE_JOIN_ROUND);

	//		cairo_new_path(context);
			cairo_set_line_width(context, 8.0);
			cairo_move_to(context, x1, y1);
			cairo_line_to(context, x2, y2);
	//		cairo_close_path(context);

			cairo_stroke(context);
			//cairo_fill(context);

			_upload();
		}
		int gm_drawLine(gmThread* a_thread) {
			GM_CHECK_FLOAT_OR_INT_PARAM(x1, 0);
			GM_CHECK_FLOAT_OR_INT_PARAM(y1, 1);
			GM_CHECK_FLOAT_OR_INT_PARAM(x2, 2);
			GM_CHECK_FLOAT_OR_INT_PARAM(y2, 3);
			drawLine(x1,y1, x2,y2);
			return GM_OK;
		}*/

	bool drawText(Font* font, const char* text, int x, int y, const char* solidColor = "#ffffffff", int outline = 0, const char* outlineColor = "#ffffff00") {
		_check();
		ImageSwapRB(image);
		font->drawToImage(image, text, solidColor, x, y, 0, outlineColor);
//		ImageSavePNG(image, "test.png");
		ImageSwapRB(image);
		_upload();
		return true;
	}
	int gm_drawText(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Font*, GM_TYPE_OBJECT, font, 0);
		GM_CHECK_STRING_PARAM(text, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 2);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 3);
		GM_CHECK_STRING_PARAM(color, 4);
		if(a_thread->GetNumParams() >= 6) {
			GM_CHECK_INT_PARAM(outline, 5);
			GM_CHECK_STRING_PARAM(outlineColor, 6);
			drawText(font, text, x, y, color, outline, outlineColor);
		} else {
			drawText(font, text, x, y, color);
		}
		return ReturnThis(a_thread);
	}

public:
	IMAGE* image;
	cairo_t* context;
	cairo_surface_t* surface;
	cairo_pattern_t* pattern;
};


#endif
