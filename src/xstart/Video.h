#ifdef _WIN32

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <corela.h>
#include "Texture.h"

class Video : public Texture {
public:

	Video() : Texture() {
		id = "Video";
		ctor = "({string} file, {int} pos_x, {int} pos_y, {int} width, {int} height)";
		help = "Video playback node.";

		textureFlags = TEX_NOMIPMAP | TEX_PBO | TEX_CLAMP | TEX_NOALPHA;
		video = 0;

		BindFunction("open", (SCRIPT_FUNCTION)&Video::gm_open, "{int} open({string} file)", "Opens a video for playback. The video is <b>not</b> started here.");
		BindFunction("start", (SCRIPT_FUNCTION)&Video::gm_start, "[this] start()", "Starts the video. <b>Some video codecs may need manual timing and will not render beyond the first frame by itself.</b>");
		BindFunction("stop", (SCRIPT_FUNCTION)&Video::gm_stop, "[this] stop()", "Stops the video.");
		BindFunction("pause", (SCRIPT_FUNCTION)&Video::gm_pause, "[this] pause()", "Pauses the video.");
		BindFunction("setPosition", (SCRIPT_FUNCTION)&Video::gm_setPosition, "[this] setPosition({float} position)", "Sets the frame position (in seconds) of the video.");
		BindFunction("getPosition", (SCRIPT_FUNCTION)&Video::gm_getPosition, "{float} getPosition()", "Gets the frame position (in seconds) of the video.");
		BindFunction("getDuration", (SCRIPT_FUNCTION)&Video::gm_getDuration, "{float} getDuration()", "Gets the total duration (in seconds) of the video.");
		BindFunction("setSpeed", (SCRIPT_FUNCTION)&Video::gm_setSpeed, "[this] setSpeed({float} speed)", "Sets the playback rate of the video. With 1.0 meaning normal speed and 0.5 meaning half-speed and so on.");
		BindFunction("update", (SCRIPT_FUNCTION)&Video::gm_update, "[this] update()", "Captures the video frame into texture.");
		//BindFunction("snapshot", (SCRIPT_FUNCTION)&Camera::gm_snapshot, "[Texture] snapshot()", "Makes a snapshot of the current image and returns it as a texture.");
		//BindFunction("snapshotBitmap", (SCRIPT_FUNCTION)&Camera::gm_snapshotBitmap, "[Bitmap] snapshotBitmap()", "Makes a snapshot of the current image and returns it as a bitmap.");
	}

	~Video() {
		if(video) {
			VideoClose(video);
		}
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
		if(a_thread->GetNumParams() >= 1) {
			if(a_thread->ParamType(0) == GM_STRING) {
				const char* file = a_thread->ParamString(0, "");
				open(file);
			}
		}
		return GM_OK;
	}


	bool open(const char* file) {
		create(dimension->x, dimension->y);
		texture->drawRect.left = 0.0;
		texture->drawRect.right = 1.0;
		texture->drawRect.top = 1.0;
		texture->drawRect.bottom = 0.0;

		if(video) { VideoClose(video); }
		video = VideoOpen(_FILE(file));
		return video ? true : false;
	}
	int gm_open(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt(open(file));
		return GM_OK;
	}

	bool start() {
		if(video) { return VideoStart(video); }
		return false;
	}
	int gm_start(gmThread* a_thread) {
		start();
		return ReturnThis(a_thread);
	}

	bool stop() {
		if(video) { return VideoStop(video); }
		return false;
	}
	int gm_stop(gmThread* a_thread) {
		stop();
		return ReturnThis(a_thread);
	}

	bool pause() {
		if(video) { return VideoPause(video); }
		return false;
	}
	int gm_pause(gmThread* a_thread) {
		pause();
		return ReturnThis(a_thread);
	}

	bool setPosition(double position) {
		if(video) { return VideoSetPosition(video, position); }
		return false;
	}
	int gm_setPosition(gmThread* a_thread) {
		GM_CHECK_FLOAT_OR_INT_PARAM(position, 0);
		setPosition(position);
		return ReturnThis(a_thread);
	}

	double getPosition() {
		if(video) { return VideoGetPosition(video); }
		return 0.0;
	}
	int gm_getPosition(gmThread* a_thread) {
		a_thread->PushFloat(getPosition());
		return GM_OK;
	}

	double getDuration() {
		if(video) { return VideoGetDuration(video); }
		return 0.0;
	}
	int gm_getDuration(gmThread* a_thread) {
		a_thread->PushFloat(getDuration());
		return GM_OK;
	}

	int setSpeed(double speed) {
		//if(video) { return VideoSetSpeed(video, speed); }
		return false;
	}
	int gm_setSpeed(gmThread* a_thread) {
		GM_CHECK_FLOAT_OR_INT_PARAM(speed, 0);
		setSpeed(speed);
		return ReturnThis(a_thread);
	}

	void _update() {
		if(!video) { return; }

		// get capture dimensions
		coDword capWidth, capHeight;
		VideoGetImage(video, 0, &capWidth, &capHeight);

		// check texture dimension
		if(texture->width != capWidth && texture->height != capHeight) {
			create(capWidth, capHeight);
			texture->drawRect.left = 0.0;
			texture->drawRect.right = 1.0;
			texture->drawRect.top = 1.0;
			texture->drawRect.bottom = 0.0;
		}

		// make fake image from locked texture
		IMAGE imageTmp;
		imageTmp.width = texture->width;
		imageTmp.height = texture->height;
		imageTmp.data = (PIXEL*)TextureLock(texture, false , TEX_FORMAT_BGR);

		// capture image and unlock texture
		VideoGetImage(video, &imageTmp, &texture->width, &texture->height);
		TextureUnlock(texture, true, TEX_FORMAT_BGR);
	}
	int gm_update(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		_update();
		return ReturnThis(a_thread);
	}

	/*Bitmap* snapshotBitmap() {
		// get capture dimensions
		coDword capWidth, capHeight;
		CaptureGetImage(0, &capWidth, &capHeight);

		// create bitmap
		Bitmap* bitmap = new Bitmap();
		bitmap->create(capWidth, capHeight);

		// capture image and unlock texture
		CaptureGetImage(bitmap->image, &capWidth, &capHeight);

		return bitmap;
	}
	int gm_snapshotBitmap(gmThread* a_thread) {
		Bitmap* bitmap = snapshotBitmap();
		return bitmap->ReturnThis(a_thread);
	}*/

	/*Texture* snapshot() {
		// get capture dimensions
		coDword capWidth, capHeight;
		CaptureGetImage(0, &capWidth, &capHeight);

		// create texture
		Texture* texture = new Texture();
		texture->create(capWidth, capHeight);

		// make fake image from locked texture
		IMAGE imageTmp;
		imageTmp.width = texture->texture->width;
		imageTmp.height = texture->texture->height;
		imageTmp.data = (PIXEL*)TextureLock(texture->texture, false, TEX_FORMAT_BGRA);

		// capture image and unlock texture
		CaptureGetImage(&imageTmp, &texture->texture->width, &texture->texture->height);
		TextureUnlock(texture->texture, true, TEX_FORMAT_BGRA);

		return texture;
	}
	int gm_snapshot(gmThread* a_thread) {
		Texture* texture = snapshot();
		return texture->ReturnThis(a_thread);
	}*/

	void render() {
		if(texture) {
			RenderTextureQuad(texture, position->x, position->y, pivot->x, pivot->y, dimension->x, dimension->y, 0, rotation->z, 1.0, 1.0);
		}
		RenderChilds();
	}

public:
	void* video;
};


#endif


#endif