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
		textures = 0;
		enabled = 0;
		recordWidth = 0;
		recordHeight = 0;

		BindFunction("save", (SCRIPT_FUNCTION)&Recorder::gm_save, "[this] save({string} folder", "Saves the stored sequence into the given folder.");
		BindFunction("getFrame", (SCRIPT_FUNCTION)&Recorder::gm_getframe, "[Texture] getFrame({int} index)", "Gets the Nth frame texture of the stored sequence.");
		BindMember("numFrames", &numFrames, TYPE_INT, 0, "{int} numFrames", "Number to frames in sequence to store.");
		BindMember("currentFrame", &currentFrame, TYPE_INT, 0, "{int} currentFrame", "The current frame that gets rendered into.");
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
		if(textures) {
			delete[] textures;
			textures = 0;
		}
		currentFrame = 0;
		valid = false;
	}

	virtual void validate() {
		if(numFrames <= 0) { return; }

		float texWidth = dimension->x;
		float texHeight = dimension->y;

		if(recordWidth != 0) { texWidth = recordWidth; }
		if(recordHeight != 0) { texHeight = recordHeight; }

		if (!textures && texWidth > 0 && texHeight > 0) {
			textures = new Texture[numFrames];

			for (int n = 0; n < numFrames; n++) {
				textures[n].textureFlags = TEX_CLAMP | TEX_NOMIPMAP | TEX_TARGET | TEX_CLEAR;
				textures[n].create(texWidth, texHeight);
			}
		}
	}

	void render() {
		if(numFrames <= 0 || enabled == 0) {
			RenderChilds();
		} else {
			if (currentFrame >= numFrames) { currentFrame = 0; }

			TEXTURE* prevRT = glUtilsGetRenderTarget();
			glUtilsSetRenderTarget(textures[currentFrame].texture);
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			RenderChilds();
			glUtilsSetRenderTarget(prevRT);

			RenderTextureQuad(textures[currentFrame].texture, position->x, position->y, pivot->x, pivot->y, dimension->x, dimension->y, 0, rotation->z, 1.0, 1.0);

			currentFrame++;
		}
	}

	bool save(const char* folder, int width, int height) {
		for(int i=0; i<numFrames; i++) {
			int n = (i + currentFrame) % numFrames;
			if(true) {
				char fileName[512];
				sprintf(fileName, "%s%d.png", folder, i);
				
				//TextureSave(frames[n], fileName, frames[n]->width, frames[n]->height);
				
				IMAGE* img = ImageCreate(textures[n].texture->width, textures[n].texture->height);
				TextureGetImage(textures[n].texture, img);
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
		if (!textures) { return ReturnNull(a_thread); }
		return textures[index].ReturnThis(a_thread, true);
/*		Texture* texture = new Texture();
		texture->texture = frames[index];
		texture->dimension->x = frames[index]->width;
		texture->dimension->y = frames[index]->height;
		return texture->ReturnThis(a_thread);*/
	}

public:
	Texture* textures;
	int numFrames;
	int currentFrame;
	int enabled;
	int recordWidth;
	int recordHeight;
};


#endif
