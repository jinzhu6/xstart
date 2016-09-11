#ifdef _WIN32


#ifndef _SOUND_H_
#define _SOUND_H_

#include "soundy/sdy.h"
#include "ScriptObject.h"

//#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\dsound.lib")
//#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\dxguid.lib")

int g_SoundYInitialized = false;

class Sound : public ScriptObject {
public:

	Sound() : ScriptObject() {
		id = "Sound";
		ctor = "({string} file)";
		help = "A sound.";

		sound = 0;

		if(!g_SoundYInitialized) {
			sdyInit(0, 0);
			g_SoundYInitialized = true;
		}

		BindFunction("open", (SCRIPT_FUNCTION)&Sound::gm_open, "{int} open({string} file)");
		BindFunction("close", (SCRIPT_FUNCTION)&Sound::gm_close, "[this] close()");
		BindFunction("play", (SCRIPT_FUNCTION)&Sound::gm_play, "[this] play({int} loop)");
		BindFunction("stop", (SCRIPT_FUNCTION)&Sound::gm_stop, "[this] stop()");
		BindFunction("isPlaying", (SCRIPT_FUNCTION)&Sound::gm_isPlaying, "{int} isPlaying()");
		/*BindFunction("setPriority", (SCRIPT_FUNCTION)&Sound::gm_setPriority);
		BindFunction("setMaxCopies", (SCRIPT_FUNCTION)&Sound::gm_setMaxCopies);
		BindFunction("setVolume", (SCRIPT_FUNCTION)&Sound::gm_setVolume);
		BindFunction("setPanning", (SCRIPT_FUNCTION)&Sound::gm_setPanning);
		BindFunction("setFrequency", (SCRIPT_FUNCTION)&Sound::gm_setFrequency);
		BindFunction("set3DPosition", (SCRIPT_FUNCTION)&Sound::gm_set3DPosition);
		BindFunction("setVelocity", (SCRIPT_FUNCTION)&Sound::gm_setVelocity);
		BindFunction("setDistance", (SCRIPT_FUNCTION)&Sound::gm_setDistance);*/
	}

	~Sound() {
		if(sound) {
			sdyClose(sound);
		}
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) {
			if(a_thread->ParamType(0) == GM_STRING) {
				const char* file = a_thread->ParamString(0, "");
				open(file);
			}
		}
		return GM_OK;
	}

	int open(const char* file) {
		if(sound) {
			sdyClose(sound);
		}
		sound = sdyOpen(_FILE(file), false, false);
		if(!sound) { Log(LOG_ERROR, "Error while opening sound file '%s'!", file); }
		return sound ? true : false;
	}
	int gm_open(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt(open(file));
		return GM_OK;
	}

	void close() {
		if(sound) {
			sdyClose(sound);
		}
	}
	int gm_close(gmThread* a_thread) {
		close();
		return ReturnThis(a_thread);
	}

	void play(bool loop) {
		if(!sound) {
			return;
		}
		sdyPlay(sound, loop);
	}
	int gm_play(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(loop, 0);
		play(loop != 0);
		return ReturnThis(a_thread);
	}

	void stop() {
		if(!sound) {
			return;
		}
		sdyStop(sound);
	}
	int gm_stop(gmThread* a_thread) {
		stop();
		return ReturnThis(a_thread);
	}

	int isPlaying() {
		if(!sound) {
			return false;
		}
		return sdyIsPlaying(sound);
	}
	int gm_isPlaying(gmThread* a_thread) {
		a_thread->PushInt(isPlaying());
		return GM_OK;
	}

public:
	SDY_SOUND* sound;
};


#endif


#endif
