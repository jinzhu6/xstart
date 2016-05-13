#ifdef __linux__

#ifndef _GSTREAMER_H_
#define _GSTREAMER_H_

#include "ScriptObject.h"
#include "Frame.h"
#include <corela/gst-backend.h>

class GStreamer : public ScriptObject {
public:
	GStreamer() : ScriptObject() {
		id = "GStreamer";

		BindFunction("play", (SCRIPT_FUNCTION)&GStreamer::gm_play);
		BindFunction("stop", (SCRIPT_FUNCTION)&GStreamer::gm_stop);
		BindFunction("isPlaying", (SCRIPT_FUNCTION)&GStreamer::gm_isPlaying);
		BindFunction("getDuration", (SCRIPT_FUNCTION)&GStreamer::gm_getDuration);
		BindFunction("getPosition", (SCRIPT_FUNCTION)&GStreamer::gm_getPosition);
		BindFunction("setPosition", (SCRIPT_FUNCTION)&GStreamer::gm_setPosition);
	}

	~GStreamer() {
		backend_stop();
		backend_deinit();
	}

	int Initialize(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Frame*, GM_TYPE_OBJECT, frame, 0);
		FRAME* _frame = (FRAME*)(frame->internalHandle);
		backend_set_window(_frame->hwnd);
		backend_init(0,0);
		return GM_OK;
	}

	int gm_play(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file,0);
		backend_play(_FILE(file));
		return GM_OK;
	}

	int gm_stop(gmThread* a_thread) {
		backend_stop();
		return GM_OK;
	}

	int gm_isPlaying(gmThread* a_thread) {
		int isPlaying = backend_query_position() > 0 && backend_query_position() < backend_query_duration();
		a_thread->PushInt(isPlaying);
		return GM_OK;
	}

	int gm_getDuration(gmThread* a_thread) {
        int duration = backend_query_duration();
        a_thread->PushInt(duration);
        return GM_OK;
	}

	int gm_getPosition(gmThread* a_thread) {
		a_thread->PushInt((int)backend_query_position());
		return GM_OK;
	}

	int gm_setPosition(gmThread* a_thread) {
	    GM_CHECK_INT_PARAM(pos, 0);
        backend_seek_absolute(pos);
        return ReturnThis(a_thread);
	}
};

#endif

#endif
