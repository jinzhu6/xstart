#ifndef _MIDI_H_
#define _MIDI_H_

#include "ScriptObject.h"
#include <midifile/MidiFile.h>


class MidiCommand : public ScriptObject {
public:

	MidiCommand() : ScriptObject() {
		id = "MidiCommand";
		ctor = "";
		help = "";

		tick = 0;
		seconds = 0.0;
		duration = 0.0;
		key = -1;
		track = 0;
		command = 0;
		channel = 0;
		param = -1;
		paramex = -1;

		BindMember("tick", &tick, TYPE_INT);
		BindMember("seconds", &seconds, TYPE_FLOAT);
		BindMember("duration", &duration, TYPE_FLOAT);
		BindMember("key", &key, TYPE_INT);
		BindMember("track", &track, TYPE_INT);
		BindMember("command", &command, TYPE_INT);
		BindMember("channel", &channel, TYPE_INT);
		BindMember("param", &param, TYPE_INT);
		BindMember("paramex", &paramex, TYPE_INT);
	}

	int tick;
	gmfloat seconds;
	gmfloat duration;
	int key;
	int track;
	int command;
	int channel;
	int param;
	int paramex;
};


class Midi : public ScriptObject {
public:

	Midi() : ScriptObject() {
		id = "Midi";
		ctor = "";
		help = "";

		fileName = "";

		BindFunction("load", (SCRIPT_FUNCTION)&Midi::gm_load);
		BindFunction("getTrackCount", (SCRIPT_FUNCTION)&Midi::gm_getTrackCount);
		BindFunction("getEventCount", (SCRIPT_FUNCTION)&Midi::gm_getEventCount);
		BindFunction("getEvent", (SCRIPT_FUNCTION)&Midi::gm_getEvent);
		BindFunction("joinTracks", (SCRIPT_FUNCTION)&Midi::gm_joinTracks);
	}

	~Midi() {
	}

	int Initialize(gmThread* a_thread) {
		return GM_OK;
	}

	bool load(const char* file) {
		midi.clear();
		midi.read(_FILE(file));
		if(!midi.status()) { return false; }
		midi.sortTracks();
		midi.absoluteTicks();
		midi.doTimeAnalysis();
		midi.linkNotePairs();
		return true;
	}
	int gm_load(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt(load(file));
		return GM_OK;
	}

	void save(const char* file) {
	}

	void joinTracks() {
		midi.joinTracks();
	}
	int gm_joinTracks(gmThread* a_thread) {
		joinTracks();
		return GM_OK;
	}

	int getTrackCount() {
		return midi.getTrackCount();
	}
	int gm_getTrackCount(gmThread* a_thread) {
		a_thread->PushInt(getTrackCount());
		return GM_OK;
	}

	int getEventCount(int track) {
		return midi.getEventCount(track);
	}
	int gm_getEventCount(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(track, 0);
		a_thread->PushInt(getEventCount(track));
		return GM_OK;
	}

	MidiCommand* getEvent(int track, int index) {
		if(track >= getTrackCount()) { return 0; }
		if(index >= getEventCount(track)) { return 0; }

		MidiCommand* command = new MidiCommand();
		command->tick = midi[track][index].tick;
		command->seconds = midi[track][index].seconds;
		command->duration = midi[track][index].getDurationInSeconds();
		command->key = midi[track][index].getKeyNumber();
		command->track = midi[track][index].track;
		command->command = midi[track][index].getCommandNibble();
		command->channel = midi[track][index].getChannelNibble();
		command->param = midi[track][index][1];
		if(midi[track][index].size() > 2) { command->paramex = midi[track][index][2]; }
		return command;
	}
	int gm_getEvent(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(track, 0);
		GM_CHECK_INT_PARAM(index, 1);
		MidiCommand* command = getEvent(track, index);
		if(!command) { return ReturnNull(a_thread); }
		return command->ReturnThis(a_thread);
	}

public:
	std::string fileName;
	MidiFile midi;
	int numTracks;
};


#endif
