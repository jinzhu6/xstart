#ifndef _RECORDER_H_
#define _RECORDER_H_

#include <corela.h>
#include <glutils.h>
#include "ScriptObject.h"
#include "Texture.h"


class Recorder : public Node {

public:

	Recorder() : Node() {
		id = "Recorder";
		help = "A Recorder node can store a sequence of frames of its childs in a ringbuffer of textures. However, it must be noted that for realtime purpose, the storage is limited by the OpenGL texture memory.";

		numFrames = 10;
		currentFrame = 0;
		frames = 0;
		enabled = 0;
		recordWidth = 0;
		recordHeight = 0;

		BindFunction("save", (SCRIPT_FUNCTION)&Recorder::gm_save, "[this] save({string} folder", "Saves the stored sequence into the given folder.");
		BindFunction("getFrame", (SCRIPT_FUNCTION)&Recorder::gm_getframe, "[Texture] getFrame({int} index)", "Gets the Nth frame texture of the stored sequence.");
		BindMember("numFrames", &numFrames, TYPE_INT, 0, "{int} numFrames", "Number to frames in sequence to store.");
		BindMember("enabled", &enabled, TYPE_INT);
		BindMember("record", &enabled, TYPE_INT, 0, "{int} record", "Set to non-zero to record frames.");
		BindMember("recordWidth", &recordWidth, TYPE_INT, 0, "{int} recordWidth", "Width of the record resolution.");
		BindMember("recordHeight", &recordHeight, TYPE_INT, 0, "{int} recordHeight", "Height of the record resolution.");
	}

	~Recorder() {
		invalidate();
	}

	void SetDot(const char* key, gmVariable &var) {
		if(strcmp(key, "numFrames") == 0) {
			invalidate();
		}

		// call parent method
		Node::SetDot(key, var);
	}

	virtual void invalidate() {
		if(frames) {
			for(int n=0; n < numFrames; n++) {
				if(frames[n]) {
					TextureDestroy(frames[n]);
				}
			}

			free(frames);
			frames = 0;
		}
		valid = false;
	}

	virtual void validate() {
		if(numFrames <= 0) { return; }

		float texWidth = dimension->x;
		float texHeight = dimension->y;

		if(recordWidth != 0) { texWidth = recordWidth; }
		if(recordHeight != 0) { texHeight = recordHeight; }

		if(!frames) {
			frames = (TEXTURE**)malloc(sizeof(void*) * numFrames);
			memset(frames, 0, sizeof(void*) * numFrames);
		}

		for(int n=0; n < numFrames; n++) {
			if(frames[n]) {
				if(frames[n]->width == texWidth && frames[n]->height == texHeight) {
					// frame texture is ok
					continue;
				}
				TextureDestroy(frames[n]);
			}

			// (re-)create frame texture
			frames[n] = TextureCreate(texWidth, texHeight, TEX_CLAMP | TEX_NOMIPMAP | TEX_TARGET, "#00000000");
		}

	}

	void render() {
		if(numFrames <= 0 || enabled == 0) {
			RenderChilds();
		} else {
			TEXTURE* prevRT = glUtilsGetRenderTarget();
			glUtilsSetRenderTarget(frames[currentFrame]);
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			RenderChilds();
			glUtilsSetRenderTarget(prevRT);

			RenderTextureQuad(frames[currentFrame], position->x, position->y, pivot->x, pivot->y, dimension->x, dimension->y, 0, rotation->z, 1.0, 1.0);

			if(++currentFrame >= numFrames) { currentFrame = 0; }
		}
	}

	bool save(const char* folder, int width, int height) {
		for(int i=0; i<numFrames; i++) {
			int n = (i + currentFrame) % numFrames;
			if(frames[n]) {
				char fileName[512];
				sprintf(fileName, "%s%d.png", folder, i);
				IMAGE* img = ImageCreate(frames[n]->width, frames[n]->height);
				TextureGetImage(frames[n], img);
				ImageSavePNG(img, fileName);
				ImageDestroy(img);
			}
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

	int gm_getframe(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(index, 0);
		index = (index + currentFrame) % numFrames;
		Texture* texture = new Texture();
		texture->texture = frames[index];
		texture->dimension->x = frames[index]->width;
		texture->dimension->y = frames[index]->height;
		return texture->ReturnThis(a_thread);
	}

public:
	TEXTURE** frames;
	int numFrames;
	int currentFrame;
	int enabled;
	int recordWidth;
	int recordHeight;
};


#endif
