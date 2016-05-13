#ifdef _WIN32

#ifndef _BASLER_CAMERA_H_
#define _BASLER_CAMERA_H_

#include <corela.h>
#include "ScriptObject.h"
#include "Shader.h"
#include "Framebuffer.h"


struct BASLERCAM;
int				BaslerInit						();
void			BaslerDestroy					();
BASLERCAM*		BaslerOpen						(int index);
void			BaslerClose						(BASLERCAM* cam);
void			BaslerStart						(BASLERCAM* cam, int continuous);
void			BaslerStop						(BASLERCAM* cam);
void			BaslerTrigger					(BASLERCAM* cam);
int				BaslerSetTrigger				(BASLERCAM* cam, const char* trigger, int enable, int software);
int				BaslerSetOutputLine				(BASLERCAM* cam, const char* source);
int				BaslerSetFeature				(BASLERCAM* cam, const char* key, const char* value);
int				BaslerSetFeatureInt				(BASLERCAM* cam, const char* key, int value);
int				BaslerSetFeatureFloat			(BASLERCAM* cam, const char* key, float value);
int				BaslerSetFeatureBool			(BASLERCAM* cam, const char* key, bool value);
int				BaslerGetFeatureInt				(BASLERCAM* cam, const char* key);
unsigned int	BaslerGetRequiredBufferSize		(BASLERCAM* cam);
int				BaslerGrabSingleFrame			(BASLERCAM* cam, void* buffer, unsigned int size, int msTimeout);
void			BaslerSetHeartbeatTimeout		(BASLERCAM* cam, unsigned int msTimeout);
int				BaslerCreateEventHandler		(BASLERCAM* cam, int numBuffers);
void			BaslerStartEventHandler			(BASLERCAM* cam);
int				BaslerPollEvent					(BASLERCAM* cam);


class BaslerCam : public Node {

public:

	BaslerCam() {
		id = "BaslerCam";
		ctor = "({int} index, {int} x, {int} y, {int} width, {int} height)";
		help = "Basler camera interface (Pylon SDK). The Pylon C and C++ SDK needs to be installed and xstart needs to be compiled with BaslerCam support! This node makes use of shaders and framebuffer objects, so it may not work properly with other shader nodes or FBOs.";

//		textureFlags = TEX_NOFILTER | TEX_NOMIPMAP/* | TEX_PBO*/ | TEX_CLAMP;
		size = new Vector();
		cam = 0;
		grab = 0;
		fbo = 0;
		mask = 0;
		hotpixelFilterStrength = 0.0;

		BaslerInit();

		BindMember("size", &size, TYPE_OBJECT, 0, "[Vector] size", "READ ONLY. Width and height of the recording.");
		BindFunction("open", (SCRIPT_FUNCTION)&BaslerCam::gm_open, "{int} open({int} index, {int} width, {int} height)", "Opens the camera.");
		BindFunction("close", (SCRIPT_FUNCTION)&BaslerCam::gm_close, "close()", "Closes and releases the camera properly.");
		BindFunction("start", (SCRIPT_FUNCTION)&BaslerCam::gm_start, "[this] start({int} continuous)", "Starts the camera stream, either in single frame mode (0) or in continuously mode (1).");
		BindFunction("stop", (SCRIPT_FUNCTION)&BaslerCam::gm_stop, "[this] stop()", "Stops the camera stream.");
		BindFunction("save", (SCRIPT_FUNCTION)&BaslerCam::gm_save, "[this] save({string} file)", "Saves the current content to a PNG image file.");
		BindFunction("setPixelFormat", (SCRIPT_FUNCTION)&BaslerCam::gm_setPixelFormat, "{int} setPixelFormat({string} format", "Sets the format of the camera, default is \"rg8\", accepted values are: \"rg8\" or \"gb8\".");
		BindFunction("setFeature", (SCRIPT_FUNCTION)&BaslerCam::gm_setFeature, "{int} setFeature({string} key, {string} value)", "Sets a string based feature on the camera, look in the Pylon software for available features and use the right setFeatureXXX() function.");
		BindFunction("setFeatureInt", (SCRIPT_FUNCTION)&BaslerCam::gm_setFeatureInt, "{int} setFeatureInt({string} key,  {int} value)", "Sets a int based feature on the camera.");
		BindFunction("setFeatureBool", (SCRIPT_FUNCTION)&BaslerCam::gm_setFeatureBool, "{int} setFeatureBool({string} key,  {bool} value)", "Sets a bool based feature on the camera.");
		BindFunction("setFeatureFloat", (SCRIPT_FUNCTION)&BaslerCam::gm_setFeatureFloat, "{int} setFeatureFloat({string} key,  {float} value)", "Sets a float based feature on the camera.");
		BindFunction("grabImage", (SCRIPT_FUNCTION)&BaslerCam::gm_grabImage, "{int} grabImage({float or int} timeoutSeconds", "Grabs an image from the camera, if the camera is in trigger mode, the grabImage functions waits for the given seconds and returns non-zero when a image could be grabbed.");
//		BindFunction("update", (SCRIPT_FUNCTION)&BaslerCam::gm_grabImage);
		BindFunction("setTriggerMode", (SCRIPT_FUNCTION)&BaslerCam::gm_setTriggerMode, "[this] setTriggerMode({int} mode)", "Sets the trigger mode. 0 == disabled, 1 == hardware, 2 == software");
		BindFunction("trigger", (SCRIPT_FUNCTION)&BaslerCam::gm_trigger, "[this] trigger()", "Does the software trigger.");
		BindFunction("snapshot", (SCRIPT_FUNCTION)&BaslerCam::gm_snapshot, "[Texture] snapshot()", "Get the current camera image as Texture object.");
		BindFunction("snapshotBitmap", (SCRIPT_FUNCTION)&BaslerCam::gm_snapshotBitmap, "[Bitmap] snapshotBitmap()", "Get the current camera image as Bitmap object.");
		BindMember("hotpixelFilterStrength", &this->hotpixelFilterStrength, TYPE_FLOAT, 0, "{float} hotpixelFilterStrength", "Set the strength of the hotpixel filter. Default is 0.0, use greater values to filter hotpixels.");
	}

	~BaslerCam() {
		close();
		//BaslerDestroy();
	}

	int Initialize(gmThread* a_thread) {
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
		if(a_thread->GetNumParams() >= 1) {
			GM_CHECK_INT_PARAM(index, 0);
			if(index >= 0) { open(index, dimension->x, dimension->y); }
		}
		return GM_OK;
	}

	void render() {
		if(fbo) {
			GLSLSHADER* prevShader = GLSLGetShader();

			if(hotpixelFilterStrength > 0.0) {
				// enable denoise shader
				shaderDenoise.setFloat("FILTER_STRENGTH", this->hotpixelFilterStrength);
				GLSLSetShader(shaderDenoise.shader);
			}

			// render fbo to screen or target
			RenderTextureQuad(fbo, position->x, position->y, pivot->x, pivot->y, dimension->x, dimension->y, 0, rotation->z, 1.0, 1.0);

			// disable denoise shader
			GLSLSetShader(prevShader);
		}

		RenderChilds();
	}

	bool open(int index, int width, int height) {
		// open the camera with the given index
		cam = BaslerOpen(index);
		if(!cam) { return false; }

		// set common camera features
		BaslerSetHeartbeatTimeout(cam, 60000);
		BaslerSetFeatureInt(cam, "Width", width);
		BaslerSetFeatureInt(cam, "Height", height);
		BaslerSetFeature(cam, "BalanceWhiteAuto", "Continuous");
		BaslerSetFeature(cam, "ExposureAuto", "Continuous");
		BaslerSetFeatureBool(cam, "CenterX", true);
		BaslerSetFeatureBool(cam, "CenterY", true);
		BaslerSetOutputLine(cam, "FlashWindow");

		// remember width and height in size member
		this->size->x = width;
		this->size->y = height;

		// set pixel format
		setPixelFormat("BayerRG8");

		// setup bayer shader
		shaderBayer.load("\
out vec2 TexCoord;\
\
void main(void) {\
	TexCoord = gl_MultiTexCoord0.st;\
	gl_Position = ftransform();\
}",
		                 "\
in vec2 TexCoord;\
uniform sampler2D image;\
uniform sampler2D mask;\
\
uniform vec2 FILTER_SCALE;\
\
void main(void) {\
	\
	vec3 sums = vec3(0.0, 0.0, 0.0);\
	vec3 cols = vec3(0.0, 0.0, 0.0);\
	\
	for(int y = -1; y <= 1; y++) {\
		for(int x = 0; x <= 1; x++) {\
			vec3 m  = texture2D(mask,  TexCoord + vec2(x + FILTER_SCALE.x*0.5, y + FILTER_SCALE.y*0.5) * FILTER_SCALE).rgb;\
			sums += m;\
			\
			float v = texture2D(image, TexCoord + vec2(x + FILTER_SCALE.x*0.5, y + FILTER_SCALE.y*0.5) * FILTER_SCALE).r;\
			cols += m * v;\
		}\
	}\
	\
	cols /= sums;\
	gl_FragColor = vec4(cols, 1.0);\
}", false);
		shaderBayer.setFloat2("FILTER_SCALE", 1.0 / (double)width, 1.0 / (double)height);
		shaderBayer.setInt("image", 0);
		shaderBayer.setInt("mask", 1);

		// setup denoise shader
		shaderDenoise.load("\
out vec2 TexCoord;\
\
void main(void) {\
	TexCoord = gl_MultiTexCoord0.st;\
	gl_Position = ftransform();\
}", "\
in vec2 TexCoord;\
uniform sampler2D tex0;\
uniform vec2 FILTER_SCALE;\
uniform int FILTER_SIZE;\
uniform int FILTER_SAME;\
uniform float FILTER_STRENGTH;\
\
void main(void) {\
	vec4 average = vec4(0.0,0.0,0.0,0.0);\
	float strength = 1.0 - FILTER_STRENGTH;\
	vec4 center = texture2D(tex0, TexCoord);\
	int total = 0;\
	int same = 0;\
	\
	for(int y = -FILTER_SIZE; y <= FILTER_SIZE; y++) {\
		for(int x = -FILTER_SIZE; x <= FILTER_SIZE; x++) {\
			if(x == 0 && y == 0) continue;\
			\
			vec4 c = texture2D(tex0, TexCoord + vec2(x + FILTER_SCALE.x*0.5, y + FILTER_SCALE.y*0.5) * FILTER_SCALE);\
			average += c;\
			total += 1;\
			\
			if( abs(center.r - c.r) > strength ) continue;\
			if( abs(center.g - c.g) > strength ) continue;\
			if( abs(center.b - c.b) > strength ) continue;\
			\
			same++;\
		}\
	}\
	\
	if(same < FILTER_SAME) {\
		center = average / total;\
	}\
	\
	gl_FragColor = center;\
}", false);
		shaderDenoise.setFloat2("FILTER_SCALE", 1.0 / (double)width, 1.0 / (double)height);
		shaderDenoise.setInt("FILTER_SIZE", 2); // 2
		shaderDenoise.setInt("FILTER_SAME", 9); // 9

		// create fbo
		if(fbo) { TextureDestroy(fbo); }
		fbo = TextureCreate(width, height, TEX_CLAMP | TEX_NOFILTER | TEX_NOMIPMAP | TEX_TARGET | TEX_CLEAR, "#ffffffff");

		return true;
	}
	int gm_open(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_INT_PARAM(index, 0);
		GM_CHECK_INT_PARAM(width, 1);
		GM_CHECK_INT_PARAM(height, 2);
		a_thread->PushInt(open(index, width, height));
		return GM_OK;
	}

	void close() {
		if(grab) { TextureDestroy(grab); grab = 0; }
		if(mask) { TextureDestroy(mask); mask = 0; }
		if(fbo) { TextureDestroy(fbo); fbo = 0; }
		if(cam) { BaslerClose(cam); cam = 0; }
	}
	int gm_close(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		close();
		return GM_OK;
	}

	bool save(const char* file, int _width, int _height) {
		if(fbo) {
			int w=fbo->width, h=fbo->height;
			if(_width)  { w = _width; }
			if(_height) { h = _height; }
			TextureSave(fbo, file, w, h);
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

	bool setPixelFormat(const char* _format) {
		char pf[256];
		strcpy(pf, _format);
		strlwr(pf);

		if(strcmp(pf, "bayerrg8")==0 || strcmp(pf, "bayer rg8")==0 || strcmp(pf, "rg8")==0) {
			if( BaslerSetFeature(cam, "PixelFormat", "BayerRG8") ) {

				// create mask image
				IMAGE* imageMask = ImageCreate(size->x, size->y);
				for(int y = 0; y < size->y; y++) {
					for(int x = 0; x < size->x; x++) {
						PIXEL px;
						px.red = 0;		px.green = 0;		px.blue = 0;		px.alpha = 255;

						if(y % 2) {
							if(x % 2) {
								px.red = 255;
							} else {
								px.green = 255;
							}
						} else {
							if(x % 2) {
								px.green = 255;
							} else {
								px.blue = 255;
							}
						}

						ImageSetPixel(imageMask, x, y, &px);
					}
				}

				// create RG8 texture for shader
				if(mask) { TextureDestroy(mask); }
				mask = TextureCreate(size->x, size->y, TEX_NOFILTER | TEX_NOMIPMAP, 0);
				TextureUpload(mask, (coByte*)imageMask->data, TEX_FORMAT_BGRA);
				ImageDestroy(imageMask);

				return true;
			}
		}

		if(strcmp(pf, "bayergb8")==0 || strcmp(pf, "bayer gb8")==0 || strcmp(pf, "gb8")==0) {
			if( BaslerSetFeature(cam, "PixelFormat", "BayerGB8") ) {

				// create mask image
				IMAGE* imageMask = ImageCreate(size->x, size->y);
				for(int y = 0; y < size->y; y++) {
					for(int x = 0; x < size->x; x++) {
						PIXEL px;
						px.red = 0;		px.green = 0;		px.blue = 0;		px.alpha = 255;

						if(y % 2) {
							if(x % 2) {
								px.green = 255;
							} else {
								px.red = 255;
							}
						} else {
							if(x % 2) {
								px.blue = 255;
							} else {
								px.green = 255;
							}
						}

						ImageSetPixel(imageMask, x, y, &px);
					}
				}

				// create RG8 texture for shader
				if(mask) { TextureDestroy(mask); }
				mask = TextureCreate(size->x, size->y, TEX_NOFILTER | TEX_NOMIPMAP, 0);
				TextureUpload(mask, (coByte*)imageMask->data, TEX_FORMAT_BGRA);
				ImageDestroy(imageMask);

				return true;
			}
		}

		Log(LOG_ERROR, "Failed to set pixel-format for Basler camera to '%s'!", _format);
		return false;
	}
	int gm_setPixelFormat(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(format, 0);
		a_thread->PushInt(setPixelFormat(format));
		return GM_OK;
	}

	bool setFeature(const char* key, const char* value) {
		if(cam) {
			return (bool)BaslerSetFeature(cam, key, value);
		}
		return false;
	}
	int gm_setFeature(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(key, 0);
		GM_CHECK_STRING_PARAM(value, 1);
		a_thread->PushInt(setFeature(key, value));
		return GM_OK;
	}

	bool setFeatureInt(const char* key, int value) {
		if(cam) {
			return (bool)BaslerSetFeatureInt(cam, key, value);
		}
		return false;
	}
	int gm_setFeatureInt(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(key, 0);
		GM_CHECK_INT_PARAM(value, 1);
		a_thread->PushInt(setFeatureInt(key, value));
		return GM_OK;
	}

	bool setFeatureFloat(const char* key, int value) {
		if(cam) {
			return (bool)BaslerSetFeatureFloat(cam, key, value);
		}
		return false;
	}
	int gm_setFeatureFloat(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(key, 0);
		GM_CHECK_FLOAT_PARAM(value, 1);
		a_thread->PushInt(setFeatureFloat(key, value));
		return GM_OK;
	}

	bool setFeatureBool(const char* key, int value) {
		if(cam) {
			return (bool)BaslerSetFeatureBool(cam, key, value);
		}
		return false;
	}
	int gm_setFeatureBool(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(key, 0);
		GM_CHECK_INT_PARAM(value, 1);
		a_thread->PushInt(setFeatureBool(key, value));
		return GM_OK;
	}

	bool grabImage(int timeout) {
		if(!cam) { return false; }

		// get capture dimensions
		coDword capWidth  = BaslerGetFeatureInt(cam, "Width");
		coDword capHeight = BaslerGetFeatureInt(cam, "Height");
		if(capWidth  <= 0 || capHeight <= 0) { Log(LOG_ERROR, "BaslerCam capture dimension is invalid (%d x %d)!", capWidth, capHeight); return false; }
		if(!grab) { grab = TextureCreate(capWidth, capHeight, TEX_NOFILTER | TEX_NOMIPMAP/* | TEX_PBO*/ | TEX_CLAMP, 0); }

		// check grab texture dimension to be the same size as the camera dimension
		if(grab->width != capWidth || grab->height != capHeight) {
			TextureDestroy(grab);
			grab = TextureCreate(capWidth, capHeight, TEX_NOFILTER | TEX_NOMIPMAP/* | TEX_PBO*/ | TEX_CLAMP, 0);

			// flip camera texture for rendering
			grab->drawRect.left = 1.0;
			grab->drawRect.right = 0.0;
			grab->drawRect.top = 1.0;
			grab->drawRect.bottom = 0.0;
		}

		// lock texture
		coByte* data = TextureLock(grab, false, TEX_FORMAT_RED8);

		// capture image
		int result = BaslerGrabSingleFrame(cam, data, grab->width * grab->height * 1, timeout);

		// unlock texture
		TextureUnlock(grab, result != 0, TEX_FORMAT_RED8);

		// enable fbo
		TEXTURE* prevTarget = glUtilsGetRenderTarget();
		glUtilsSetRenderTarget(fbo);

		// clear fbo
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// enable bayer shader
		GLSLSHADER* prevShader = GLSLGetShader();
		GLSLSetShader(shaderBayer.shader);

		// render to fbo
		RenderMultiTextureQuad(grab, mask, position->x, position->y, pivot->x, pivot->y, mask->width, mask->height, 0, rotation->z, 1.0, 1.0);

		// disable fbo
		glUtilsSetRenderTarget(prevTarget);

		// disable bayer shader
		GLSLSetShader(prevShader);

		return result;
	}
	int gm_grabImage(gmThread* a_thread) {
		float timeout;
		a_thread->ParamFloatOrInt(0, timeout, 1.0);
		a_thread->PushInt(grabImage( (int)(timeout * 1000.0)) );
		return GM_OK;
	}

	int gm_trigger(gmThread* a_thread) {
		BaslerTrigger(cam);
		return ReturnThis(a_thread);
	}

	void setTriggerMode(bool enabled, bool software) {
		if(!cam) {
			Log(LOG_ERROR, "setTriggerMode() failed - camera not opened!");
			return;
		}
		BaslerSetTrigger(cam, "FrameStart", enabled, software);
	}
	int gm_setTriggerMode(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(enabled, 0);
		setTriggerMode((bool)enabled, enabled == 2);
		return ReturnThis(a_thread);
	}

	void start(bool continuous) {
		BaslerStart(cam, continuous);
	}
	int gm_start(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(continuous, 0);
		start(continuous);
		return ReturnThis(a_thread);
	}

	void stop() {
		BaslerStop(cam);
	}
	int gm_stop(gmThread* a_thread) {
		stop();
		return ReturnThis(a_thread);
	}

	Texture* snapshot() {
		if(!fbo) { return 0; }
		Texture* snap = new Texture();
		if(snap->texture) { TextureDestroy(snap->texture); }
		snap->texture = TextureDuplicate(fbo);
		// flip texture vertically
		snap->texture->drawRect.left   = 1.0;
		snap->texture->drawRect.top    = 1.0;
		snap->texture->drawRect.right  = 0.0;
		snap->texture->drawRect.bottom = 0.0;
		return snap;
	}
	int gm_snapshot(gmThread* a_thread) {
		Texture* snap = snapshot();
		if(!snap) { return GM_OK; }
		return snap->ReturnThis(a_thread);
	}

	Bitmap* snapshotBitmap() {
		Bitmap* bitmap = new Bitmap();
		bitmap->create(fbo->width, fbo->height);
		TextureGetImage(fbo, bitmap->image);
		return bitmap;
	}
	int gm_snapshotBitmap(gmThread* a_thread) {
		Bitmap* bitmap = snapshotBitmap();
		if(!bitmap) { return GM_OK; }
		return bitmap->ReturnThis(a_thread);
	}

public:
	BASLERCAM* cam;
	Vector* size;
	TEXTURE* grab;
	TEXTURE* mask;
	TEXTURE* fbo;
	Shader shaderBayer;
	Shader shaderDenoise;
	float hotpixelFilterStrength;
};


#endif

#endif