#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include <corela.h>
#include <glutils.h>
#include "ScriptObject.h"
#include "Texture.h"


class Framebuffer : public Texture {
public:

	Framebuffer() : Texture() {
		id = "Framebuffer";
		help = "Framebuffer for off-screen, special-effects or capture rendering. All childs are rendered into the framebuffer. Then framebuffer itself can be then be rendered too.";
		textureFlags = TEX_CLAMP | TEX_NOMIPMAP | TEX_TARGET;

		enabled = true;

		BindMember("enabled", &enabled, TYPE_INT, 0, "{int} enabled", "Enables or disables framebuffer update. When disabled (enabled = false), updating and child-rendering is skipped and the current content is reused when rendering the node.");
	}

	void begin() {
		if(!texture) {
			Texture::create((int)dimension->x, (int)dimension->y);
			Texture::clear();
		}
		if(texture->width != (int)dimension->x || texture->height != (int)dimension->y) {
			Texture::create((int)dimension->x, (int)dimension->y);
			Texture::clear();
		}

		// remember previous render target and set current
		prevRT = glUtilsGetRenderTarget();
		glUtilsSetRenderTarget(texture);

		// clear color and alpha
		glClearColor(0.0, 0.0, 0.0, 0.0); // important: clear alpha
		glClear(GL_COLOR_BUFFER_BIT);

		// enable alpha blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		// disable depth test
		glDisable(GL_DEPTH_TEST);

		// disable alpha test
		glDisable(GL_ALPHA_TEST);

		// disable face backface culling
		glDisable(GL_CULL_FACE);

		// disable lighting
		glDisable(GL_LIGHTING);

		// load identity model-view
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	}

	void end() {
		// restore previous render target
		glUtilsSetRenderTarget(prevRT);

		// restore model-view matrix
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	void render() {
		if(enabled) {
			begin();
			RenderChilds();
			end();
		}

		if(texture) {
			RenderTextureQuad(texture, position->x, position->y, pivot->x * scaling->x * scaling->z, pivot->y * scaling->y * scaling->z, dimension->x * scaling->x * scaling->z, dimension->y * scaling->y * scaling->z, 0, rotation->z, 1.0, opacity);
		}
	}

public:
	TEXTURE* prevRT;
	bool enabled;
};


#endif
