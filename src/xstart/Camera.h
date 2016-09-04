#ifdef _MSC_BUILD

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <corela.h>
#include "ScriptObject.h"

class Camera : public Texture {
public:

	Camera() : Texture() {
		id = "Camera";
		ctor = "({sting} (or) {int} device, {int} pos_x, {int} pos_y, {int} width, {int} height, {int} bits";
		help = "Camera node that displays the image from the given camera and/or saves it.";
		textureFlags = TEX_NOMIPMAP | TEX_PBO | TEX_CLAMP;

		device = 0;

		BindFunction("open", (SCRIPT_FUNCTION)&Camera::gm_open, "{int} open({sting} (or) {int} device, {int} width, {int} height, {int} bits)", "Opens the camera with the given settings, the 'device' can either be an index, the device name or part of the beginning of the device name. Returns non-zero on success.");
		// TODO: Clean closing of camera
		//		BindFunction("close", (SCRIPT_FUNCTION)&Camera::gm_close, "{int} close()");
		BindFunction("update", (SCRIPT_FUNCTION)&Camera::gm_update, "[this] update()", "Does a manual update of the camera texture, may need to be called in sync with the cameras framerate.");
		BindFunction("config", (SCRIPT_FUNCTION)&Camera::gm_config, "[this] config()", "Opens a DirectShow configuration dialog for the device. Some changes on that dialog should then be stored for this application (not reset on reboot). But this needs to be tested, since it is driver-dependend.");
		BindFunction("setFocus",  (SCRIPT_FUNCTION)&Camera::gm_setFocus, "[this] setFocus({int} focus)", "Sets the focus for the device or enables the auto-focus when the value is zero or below.");
		BindFunction("setZoom", (SCRIPT_FUNCTION)&Camera::gm_setZoom, "[this] setZoom({int} zoom)", "Sets the zoom for the device. ");
		BindFunction("setExposure", (SCRIPT_FUNCTION)&Camera::gm_setExposure, "[this] setExposure({int} exposure)", "Sets the exposure for the device.");
		BindFunction("setWhiteBalance", (SCRIPT_FUNCTION)&Camera::gm_setWhiteBalance, "[this] setWhiteBalance({int} balance)", "Set white-balance for the device.");
		BindFunction("setBrightness", (SCRIPT_FUNCTION)&Camera::gm_setBrightness, "[this] setBrightness({int} brightness)", "Set brightness for the device.");
		BindFunction("setContrast", (SCRIPT_FUNCTION)&Camera::gm_setContrast, "[this] setContrast({int} contrast)", "Set contrast for the device.");
		BindFunction("setSaturation", (SCRIPT_FUNCTION)&Camera::gm_setSaturation, "[this] setSaturation({int} saturation)", "Set saturation for the device.");
		BindFunction("setPowerlineFrequency", (SCRIPT_FUNCTION)&Camera::gm_setPowerlineFrequency, "[this] setPowerlineFrequency({int} frequency)", "Set the frequency of the powerline to match the camera.");
		BindFunction("snapshot", (SCRIPT_FUNCTION)&Camera::gm_snapshot, "[Texture] snapshot()", "Makes a snapshot of the current camera image and returns it as a texture.");
		BindFunction("snapshotBitmap", (SCRIPT_FUNCTION)&Camera::gm_snapshotBitmap, "[Bitmap] snapshotBitmap((optional)[Bitmap] dst)", "Makes a snapshot of the current camera image and returns it as a bitmap.");
	}

	~Camera() {
		if(device) { CaptureRelease(device); }
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) {
			gmVariable var;
			a_thread->Param(0, var);
			if(!var.IsNull()) {
				id = var.GetCStringSafe();
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
		int bits = 0;
		if(a_thread->GetNumParams() >= 6) {
			a_thread->ParamInt(5, bits, 0);
		}
		if(a_thread->GetNumParams() >= 1) {
			if(a_thread->ParamType(0) == GM_STRING) {
				const char* devName;
				devName = a_thread->ParamString(0, "");
				open(0, devName, dimension->x, dimension->y, bits);
			} else if(a_thread->ParamType(0) == GM_INT) {
				int index = 0;
				index = a_thread->ParamInt(0);
				open(index, "", dimension->x, dimension->y, bits);
			}
		}

		return GM_OK;
	}

	bool open(int index, const char* devName, int width, int height, int bits = 0) {
		texture = TextureCreate(width, height, textureFlags, 0);

		// flip texture rendering vertically
		texture->drawRect.left = 1.0;
		texture->drawRect.right = 0.0;
		texture->drawRect.top = 1.0;
		texture->drawRect.bottom = 0.0;

		wchar_t  ws[128];
		swprintf(ws, 100, L"%hs", devName);
		device = CaptureDevice(index, ws, width, height, bits);
		return device != 0;
	}
	int gm_open(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(4);
		GM_CHECK_FLOAT_OR_INT_PARAM(width, 1);
		GM_CHECK_FLOAT_OR_INT_PARAM(height, 2);
		GM_CHECK_INT_PARAM(bits, 3);

		if(a_thread->ParamType(0) == GM_STRING) {
			const char* devName;
			devName = a_thread->ParamString(0, "");
			a_thread->PushInt(open(0, devName, width, height, bits));
		} else if(a_thread->ParamType(0) == GM_INT) {
			int index = 0;
			index = a_thread->ParamInt(0);
			a_thread->PushInt(open(index, "", width, height, bits));
		}

		return GM_OK;
	}

	/*void close() {
		CaptureRelease();
	}
	int gm_close(gmThread* a_thread) {
		close();
		return GM_OK;
	}*/

	bool _update() {
		// get capture dimensions
		coDword capWidth, capHeight;
		if(!CaptureGetImage(device, 0, &capWidth, &capHeight)) { Log(LOG_ERROR, "Error while capturing image from camera."); return false; }

		// check texture dimension to be the same size as the camera dimension
		if(texture->width != capWidth || texture->height != capHeight) {
			TextureDestroy(texture);
			texture = TextureCreate(capWidth, capHeight, textureFlags, 0);
			texture->drawRect.left = 1.0;
			texture->drawRect.right = 0.0;
			texture->drawRect.top = 1.0;
			texture->drawRect.bottom = 0.0;
		}

		// make fake image from locked texture
		IMAGE imageTmp;
		imageTmp.width = texture->width;
		imageTmp.height = texture->height;
		imageTmp.data = (PIXEL*)TextureLock(texture, false, TEX_FORMAT_BGRA);

		// capture image and unlock texture
		if(!CaptureGetImage(device, &imageTmp, &texture->width, &texture->height)) { Log(LOG_ERROR, "Error while capturing image from camera."); return false; }
		TextureUnlock(texture, true, TEX_FORMAT_BGRA);

		return true;
	}
	int gm_update(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		_update();
		return ReturnThis(a_thread);
	}

	Bitmap* snapshotBitmap(Bitmap* bitmap) {
		if(!bitmap) { bitmap = new Bitmap(); }

		// get capture dimensions
		coDword capWidth, capHeight;
		if(!CaptureGetImage(device, 0, &capWidth, &capHeight)) { Log(LOG_ERROR, "Error while capturing image from camera."); return bitmap; }

		// if no existing bitmap was given, create one
		if(bitmap->getWidth() != capWidth || bitmap->getHeight() != capHeight) {
			bitmap->resize(capWidth, capHeight, false);
		}
		
		// capture image and unlock texture
		if(!CaptureGetImage(device, bitmap->image, &capWidth, &capHeight)) { Log(LOG_ERROR, "Error while capturing image from camera."); return bitmap; }

		return bitmap;
	}
	int gm_snapshotBitmap(gmThread* a_thread) {
		Bitmap* bitmap;
		if(a_thread->GetNumParams() >= 1) {
			GM_CHECK_USER_PARAM(Bitmap*, GM_TYPE_OBJECT, _bitmap, 0);
			bitmap = snapshotBitmap(_bitmap);
		} else {
			bitmap = snapshotBitmap(0);
		}
		return bitmap->ReturnThis(a_thread);
	}

	Texture* snapshot() {
		// get capture dimensions
		coDword capWidth, capHeight;
		CaptureGetImage(device, 0, &capWidth, &capHeight);

		// create texture
		Texture* texture = new Texture();
		texture->create(capWidth, capHeight);

		// make fake image from locked texture
		IMAGE imageTmp;
		imageTmp.width = texture->texture->width;
		imageTmp.height = texture->texture->height;
		imageTmp.data = (PIXEL*)TextureLock(texture->texture, false, TEX_FORMAT_BGRA);

		// capture image and unlock texture
		CaptureGetImage(device, &imageTmp, &texture->texture->width, &texture->texture->height);
		TextureUnlock(texture->texture, true, TEX_FORMAT_BGRA);

		return texture;
	}
	int gm_snapshot(gmThread* a_thread) {
		Texture* texture = snapshot();
		return texture->ReturnThis(a_thread);
	}

	void config() {
		CaptureConfig(device);
	}
	int gm_config(gmThread* a_thread) {
		config();
		return ReturnThis(a_thread);
	}

	void setFocus(int focus) {
		CaptureSetFocus(device, focus);
	}
	int gm_setFocus(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(focus, 0);
		setFocus(focus);
		return ReturnThis(a_thread);
	}

	void setZoom(int zoom) {
		CaptureSetZoom(device, zoom);
	}
	int gm_setZoom(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(zoom, 0);
		setZoom(zoom);
		return ReturnThis(a_thread);
	}

	void setExposure(int exp) {
		CaptureSetExposure(device, exp);
	}
	int gm_setExposure(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(exp, 0);
		setExposure(exp);
		return ReturnThis(a_thread);
	}

	void setWhiteBalance(int bal) {
		CaptureSetWhiteBalance(device, bal);
	}
	int gm_setWhiteBalance(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(bal, 0);
		setWhiteBalance(bal);
		return ReturnThis(a_thread);
	}

	void setBrightness(int brightness) {
		CaptureSetBrightness(device, brightness);
	}
	int gm_setBrightness(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(brightness, 0);
		setBrightness(brightness);
		return ReturnThis(a_thread);
	}

	void setContrast(int contrast) {
		CaptureSetContrast(device, contrast);
	}
	int gm_setContrast(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(contrast, 0);
		setContrast(contrast);
		return ReturnThis(a_thread);
	}

	void setSaturation(int saturation) {
		CaptureSetSaturation(device, saturation);
	}
	int gm_setSaturation(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(saturation, 0);
		setSaturation(saturation);
		return ReturnThis(a_thread);
	}

	void setPowerlineFrequency(int freq) {
		CaptureSetPowerlineFrequency(device, freq);
	}
	int gm_setPowerlineFrequency(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(freq, 0);
		setPowerlineFrequency(freq);
		return ReturnThis(a_thread);
	}

	/*void render() {
		if(texture) {
			RenderTextureQuad(texture, position->x, position->y, center->x, center->y, dimension->x, dimension->y, 0, rotation->z, 1.0, 1.0);
		}
		RenderChilds();
	}*/

public:
	void* device;
};


#endif


#endif
