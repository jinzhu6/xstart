#include "ScriptObject.h"
#include "Data.h"
#include "Socket.h"
#include "List.h"
#include "corela.h"
#include "portaudio/portaudio.h"
#include "soundy/sdy.h"
#include "soundy/strm_wav.h"
#include "soundtouch/SoundTouch.h"
#include "libsoxr/soxr.h"
#include <stdio.h>
#include <string>
#include <list>
#undef GetObject

#pragma GCC diagnostic ignored "-Wwrite-strings"

using namespace soundtouch;

static char* paHostApis[] = {"DirectSound","ASIO","SoundManager","CoreAudio","N/A","OSS","ALSA","AL","BeOS","WDMKS","JACK","WASAPI","AudioScienceHPI"}; // may not be accurate on all OS
#define WAIT_FOR_SOUNDCALLBACK() { while(this->locked){TimeSleep(0.002);} }  // experimental thread lock
int PortAudioCallback(const void* input, void* output, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* user);


class AudioData : public Data {
public:

	AudioData() : Data(), isDuplicate(false), volume(1.0), isPlaying(0), multiplay(true), hf(0), outputFile(""), bytesAvailable(0) {
		id = "AudioData";
		help = "";
		
		dataIn = new Data();
		locked = false;

		BindFunction("convert", (SCRIPT_FUNCTION)&AudioData::gm_convert, "[this] convert({int} sampleRate, {int} bitsPerSample");
		BindMember("multiplay", &multiplay, TYPE_INT, 0, "{int} multiplay", "Allow to duplicate this audiobuffer for playing multiple instances simultaneously.");
		BindMember("playing", &isPlaying, TYPE_INT, 0, "{int} playing", "Audiobuffer is currently playing.");
		BindMember("volume", &volume, TYPE_FLOAT, 0, "{float} volume", "Volume of audio playback for this buffer. Ranges from 0.0 to 1.0.");
	}

	~AudioData() {
		delete dataIn;
	}

	bool operator==(const AudioData& rhs) {
		if(this->data == rhs.data) { return true; }
		return false;
	}

	bool saveBegin(const char* fileName) {
//		if (hf) { Log(LOG_ERROR, "Error in AudioData::saveBegin() - Filehandle to '%s' already open!", outputFile.c_str()); }
		// write header to file
		FILE* _hf = fopen(fileName, "wb");
		if(!_hf) { Log(LOG_ERROR, "Error in saveWav() while opening '%s' for writing!", fileName); return false; }
		fwrite("RIFF", 4, 1, _hf);
		unsigned long val = size+40-8;  fwrite(&val, 4, 1, _hf);
		fwrite("WAVE", 4, 1, _hf);
		fwrite("fmt ", 4, 1, _hf);
		val = 16;  fwrite((const void*)&val, 4, 1, _hf);
		SDY_WFX wfx;  memset(&wfx, 0, sizeof(SDY_WFX));
		wfx = this->format;
		fwrite(&wfx, 24-8, 1, _hf);
		fwrite("data", 4, 1, _hf);
		val = size; fwrite(&val, 4, 1, _hf);
		fclose(_hf);
	}
	bool saveEnd(const char* fileName) {
//		if (hf) { Log(LOG_ERROR, "Error in AudioData::saveEnd() - Filehandle to '%s' still open!", outputFile.c_str()); }
		// write final buffer-size in header
		FILE* _hf = fopen(fileName, "r+b");
		if(!_hf) { Log(LOG_ERROR, "Error in saveWav() while patching '%s'!", fileName); return false; }
		fseek(_hf, 0, SEEK_END);
		unsigned long length = ftell(_hf);
		fseek(_hf, 4, SEEK_SET);
		length -= 8; fwrite(&length, 4, 1, _hf);
		fseek(_hf, 40, SEEK_SET);
		length += 8; length -= 44; fwrite(&length, 4, 1, _hf);
		fclose(_hf);
	}
	virtual void stopRecording() {
		if(hf) {
			fclose(hf);
			hf = 0;
			saveEnd(outputFile.c_str());
		}
	}

	void setFormat(int sampleRate, int bitsPerSample, int channels) {
		switch(bitsPerSample) {
		case 16: { format.wFormatTag = SDY_FMT_S16; break; }
		case 32: { format.wFormatTag = SDY_FMT_F32; break; }
		default: { Log(LOG_ERROR, "Unsupported 'bitsPerSample' (%d) in setFormat()!", bitsPerSample); format.wFormatTag = SDY_FMT_S16; break; }
		}
		format.nChannels = channels;
		format.nSamplesPerSec = sampleRate;
		format.nAvgBytesPerSec = sampleRate * channels * format.wFormatTag==SDY_FMT_S16 ? 2 : 4;
		format.nBlockAlign = format.wFormatTag==SDY_FMT_S16 ? 2 : 4;
		format.wBitsPerSample = format.wFormatTag==SDY_FMT_S16 ? 16 : 32;
		format.cbSize = 0;
	}

	unsigned long getSamplesFromMilliseconds(unsigned long ms) {  return format.nSamplesPerSec / 1000 * ms;  }
	unsigned long getMillisecondsFromSamples(unsigned long samples) {  return samples * 1000 / format.nSamplesPerSec;  }
	unsigned long getBytesFromSamples(unsigned long samples) {  return samples * format.nBlockAlign * format.nChannels;  }
	unsigned long getSamplesFromBytes(unsigned long bytes) {  return bytes / (format.nBlockAlign * format.nChannels);  }

	// TODO: Do converting also in real-time playback.
	bool convert(int sampleRate, int bitsPerSample) {
		if(this->format.nSamplesPerSec == sampleRate && this->format.wBitsPerSample == bitsPerSample) { return true; }
		Log(LOG_DEBUG, "Converting audio from %dx%d to %dx%d ...", this->format.nSamplesPerSec, this->format.wBitsPerSample, sampleRate, bitsPerSample);

		soxr_io_spec_t spec;
		spec.itype = format.wFormatTag == SDY_FMT_F32 ? SOXR_FLOAT32_I : SOXR_INT16_I;
		spec.otype = bitsPerSample == 32 ? SOXR_FLOAT32_I : SOXR_INT16_I;
		spec.scale = 1.0;
		spec.e = 0;
		spec.flags = 0;

		unsigned long samplesPerChannel = size / format.nBlockAlign / format.nChannels;
		unsigned long outputBufferSize = samplesPerChannel * format.nChannels * (bitsPerSample / 8);

		void* out = (void*)malloc(outputBufferSize);

		soxr_error_t error;
		soxr_t soxr = soxr_create(format.nSamplesPerSec, sampleRate, format.nChannels, &error, &spec, 0, 0);
		if(error) { Log(LOG_ERROR, "Error while converting audio data!"); free(out); return false; }

		size_t odone;
		error = soxr_process(soxr, (soxr_in_t)data, (size_t)samplesPerChannel, NULL, (soxr_out_t)out, (size_t)samplesPerChannel, &odone);
		soxr_delete(soxr);
		if(error) { Log(LOG_ERROR, "Error while converting audio data!"); free(out); return false; }
		if(odone != samplesPerChannel) { /* todo */ }

		free(data);
		data = (unsigned char*)out;
		size = odone * format.nChannels * (bitsPerSample / 8);
		rewindCursors();
		setFormat(sampleRate, bitsPerSample, format.nChannels);
		return true;
	}
	int gm_convert(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(sampleRate, 0);
		GM_CHECK_INT_PARAM(bitsPerSample, 1);
		convert(sampleRate, bitsPerSample);
		return ReturnThis(a_thread);
	}

	// mixes "sourceData" in "this" and advances "readCursor" and "bytesAvailable" (both from "sourceData") too.
	virtual bool mix(AudioData* sourceData, unsigned long numSamplesPerChannel, float volume = 1.0) {
		if(format.wBitsPerSample != 32) { Log(LOG_ERROR, "Error while mixing audio buffers. Target must be float (32-bit) to mix, please convert beforehand."); return false; }
		if (!sourceData->data) { Log(LOG_WARNING, "AUDIO-ERROR: Source data is zero in mix()."); return false; }
//		if(sourceData->format.wBitsPerSample != 32) { Log(LOG_WARNING, "Audio data has been converted to float (32-bit) to be able to mix."); sourceData->convert(format.nSamplesPerSec, format.wBitsPerSample); }

		unsigned long requiredDstSize = numSamplesPerChannel * format.nChannels * format.nBlockAlign;
		if(this->size < requiredDstSize) { this->resize(requiredDstSize); }

		float* dst = (float*)this->data;
		float* src = (float*)sourceData->data;
		short* srci = (short*)sourceData->data;
		int dst_size = this->size / sizeof(float);
		int src_size = sourceData->size / sizeof(float);
		int srci_size = sourceData->size / sizeof(short);

		// TODO: Optimize! Move modulo (si) and if() from inner loop to outer loop. Also maybe optimize channel duplication by using values computed before.
		float div = 32767.0f / (sourceData->volume * volume);
		unsigned long di = writeCursor / format.nBlockAlign;  // NOTE: expects the writeCursor to be aligned to the data format size.
		unsigned long si = sourceData->readCursor / sourceData->format.nBlockAlign / sourceData->format.nChannels;
		for(unsigned long n=0; n<numSamplesPerChannel; n++) {
			si = sourceData->readCursor / sourceData->format.nBlockAlign + (n * sourceData->format.nChannels);  // NOTE: expects the readCursor to be aligned to the data format size.
			for(unsigned long c=0; c<format.nChannels; c++) {
				float v = dst[di % dst_size];
				if(sourceData->format.wBitsPerSample == 32) {  // 32-bit float
					v += src[si % src_size] * sourceData->volume * volume;
				} else { // 16-bit integer
					v += (float)(srci[si % srci_size]) / div;
				}
				// volume clamping
				if (v > 0.96) v = 0.96;
				if (v < -0.96) v = -0.96;
				dst[di % dst_size] = v;
				di++;
				if(c >= sourceData->format.nChannels) { si++; }  // duplicate source channels if neccessary (mono to stereo)
			}
		}
		sourceData->readCursor += numSamplesPerChannel * sourceData->format.nBlockAlign * sourceData->format.nChannels;
		sourceData->bytesAvailable -= numSamplesPerChannel * sourceData->format.nBlockAlign * sourceData->format.nChannels;
		if(sourceData->loop) { sourceData->readCursor = sourceData->readCursor % sourceData->size; }

		return true;
	}

	virtual AudioData* duplicate() {
		AudioData* d = new AudioData();
		// TODO: Copy!
		return d;
	}

	virtual unsigned long stream(unsigned long samples) {
		// TODO
		return samples;
	}
	
public:
	SDY_WFX format;
	int isPlaying;
	bool isDuplicate;
	bool stop;
	bool locked;
	int multiplay;

	float volume;
	Data* dataIn;
	unsigned long bytesAvailable;

	std::string outputFile;
	FILE* hf;
};


class AudioFilter : public AudioData {
public:

	AudioFilter() : AudioData() {
		id = "AudioFilter";
		help = "";
		ctor = "([AudioData] source)";

		loop = true;
		source = 0;
//		sourceCursor = 0;
		seconds = 5;
		enabled = true;

		source = new AudioData();
		history = new AudioData();

		BindMember("enabled", &enabled, TYPE_INT, 0);
		BindMember("seconds", &seconds, TYPE_INT, 0, "[int] seconds", "Number of seconds for the history buffer.");
		BindMember("source", &source, TYPE_OBJECT, 0, "[AudioData] source", "Source for streaming audio data to a device.");
		BindMember("history", &history, TYPE_OBJECT, 0, "[AudioData] history", "Audio buffer for story the history of the sound.");
		BindMember("outputFile", &outputFile, TYPE_STRING, 0);
	}

	~AudioFilter() {
		if(hf) { fclose(hf); hf =0; }
		delete source;
		delete history;
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) {
			source = (AudioData*)a_thread->ParamUser_NoCheckTypeOrParam(0);
			format = source->format;
			history->format = source->format;
		}
		return GM_OK;
	}

	// filter from history to this; in this case a simple copy from the history buffer into "this" buffer
	virtual unsigned long filter(unsigned long samples) {
		unsigned long sampleSize = source->format.nBlockAlign * source->format.nChannels;
		unsigned long samplesBytes = samples * sampleSize;
		this->transferData(history, samplesBytes);
		return samples;
	}

	// stream/prepare audio data
	virtual unsigned long stream(unsigned long samples) {
		unsigned long sampleSize = source->format.nBlockAlign * source->format.nChannels;
		unsigned long samplesBytes = samples * sampleSize;
		unsigned long reqSize = sampleSize * source->format.nSamplesPerSec * this->seconds;

		// ensure history buffer size fits
		if(this->size != samplesBytes) { this->resize(samplesBytes); }
		if(history->size != reqSize) { history->resize(reqSize); history->clear(0); }
		this->clear(0);
		this->rewindCursors();

		// shift history data to the front of the buffer
		memmove(&history->data[0], &history->data[samplesBytes], history->size - samplesBytes);
		history->readCursor = history->size - samplesBytes;
		history->writeCursor = history->size - samplesBytes;

		// append new data
		source->loop = this->loop;
		source->stream(samples);
		history->transferData(source, samplesBytes);

		// apply filter (eg. pitch)
		if(enabled) { filter(samples); }
		else { AudioFilter::filter(samples); }

		// save to file
		if(outputFile != "") {
			if(hf == 0) {
				Log(LOG_DEBUG, "Starting audio recording of file '%s' ...", outputFile.c_str());
				saveBegin(outputFile.c_str());
				hf = fopen(outputFile.c_str(), "r+b");
				fseek(hf, 0, SEEK_END);
				if(!hf) { outputFile = ""; Log(LOG_ERROR, "Error while writing file '%s'!", outputFile.c_str()); }
			}
			fwrite(this->data, this->size, 1, hf);
		}

		// transfer data to output
		this->rewindCursors();
		this->loop = true;
		return 0;
	}

public:
	AudioData* source;
	AudioData* history;
//	unsigned long sourceCursor;
	unsigned long seconds;  // history buffer
	float volume;
	int enabled;
};


class AudioPitch : public AudioFilter {
public:

	AudioPitch() : AudioFilter() {
		id = "AudioPitch";
		help = "";
		ctor = "([AudioData] source)";

		pitch = 1.0;
		prevPitch = pitch;
		underflowSamples = 1024*6;
		bufferedSamples = 0;

		pitchFilter = new SoundTouch();
//		pitchFilter->setChannels(2);
//		pitchFilter->setSampleRate(44100);
		pitchFilter->setPitch(prevPitch);

		BindMember("pitch", &pitch, TYPE_FLOAT, 0, "{float} pitch");
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) {
			source = (AudioData*)a_thread->ParamUser_NoCheckTypeOrParam(0);
			a_thread->ParamFloat(1, pitch, 1.0);
			if(source) {
				prevPitch = pitch;
				format = source->format;
				history->format = source->format;
				pitchFilter->setChannels(source->format.nChannels);
				pitchFilter->setSampleRate(source->format.nSamplesPerSec);
				pitchFilter->setPitch(prevPitch);
			}
		}
		return GM_OK;
	}

	~AudioPitch() {
		delete pitchFilter;
	}

	// filter from history to this
	virtual unsigned long filter(unsigned long samples) {
		if(this->pitch < 0.1) { this->pitch = 0.1; }
		if(this->pitch > 10.0) { this->pitch = 10.0; }
		if(this->pitch != this->prevPitch) { pitchFilter->setPitch(this->pitch); this->prevPitch = this->pitch; }
		pitchFilter->putSamples((SAMPLETYPE*)&history->data[history->readCursor], samples);
		bufferedSamples += samples;
		unsigned long readSamples = pitchFilter->numSamples();
		if(bufferedSamples >= underflowSamples) {
			if(readSamples >= samples) {
				readSamples = pitchFilter->receiveSamples((SAMPLETYPE*)this->data, samples);
				bufferedSamples -= readSamples;
			}
			if(readSamples < samples) {
				underflowSamples += samples - readSamples;
				Log(LOG_DEBUG, "AudioFilterPitch underflow by %d samples! Underflow buffer increased to %d samples.", samples - readSamples, underflowSamples);
			}
			if(readSamples > samples) {
				Log(LOG_ERROR, "AudioFilterPitch overflow!");
			}
		}
		return readSamples;
	}

public:
	SoundTouch* pitchFilter;
	float pitch;
	float prevPitch;
	unsigned long underflowSamples;
	unsigned long bufferedSamples;
};


class AudioDelay : public AudioFilter {
public:
	AudioDelay() : AudioFilter() {
		id = "AudioDelay";
		help = "";
		ctor = "([AudioData] source)";

		delay = 0.0;
		delayVolume = 0.5;

		BindMember("delay", &delay, TYPE_FLOAT, 0, "{float} delay", "Delay in seconds.");
		BindMember("delayVolume", &delayVolume, TYPE_FLOAT, 0, "{float} delayVolume", "Volume of delayed audio.");
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) {
			source = (AudioData*)a_thread->ParamUser_NoCheckTypeOrParam(0);
			a_thread->ParamFloat(1, delay, 1.0);
			a_thread->ParamFloat(2, delayVolume, 0.5);
			format = source->format;
			history->format = source->format;
		}
		return GM_OK;
	}

	virtual unsigned long filter(unsigned long samples) {
		if(delay > seconds) { delay = seconds; } // the delay can not be bigger than the history buffer
		if(delay < 0.0) { delay = 0.0; }
		unsigned long samplesDelay = getSamplesFromMilliseconds(delay * 1000);
		unsigned long sampleSize = source->format.nBlockAlign * source->format.nChannels;
		unsigned long samplesBytes = samples * sampleSize;
		if(samplesBytes <= 0) { return 0; }
		this->transferData(history, samplesBytes);
		history->readCursor = history->size - samplesBytes - samplesDelay;
		if(history->readCursor < 0) { history->readCursor = 0; }
		this->mix(history, samples, delayVolume);
		return samples;
	}

public:
	float delay;
	float delayVolume;
};


class AudioFile : public AudioData {
public:
	AudioFile() : AudioData() {
		id = "AudioFile";
		help = "";
		ctor = "({string} file)";

		BindFunction("load", (SCRIPT_FUNCTION)&AudioFile::gm_load, "[this] load({string} fileName)", "Loads the given RIFF/WAV file into a audio buffer in memory.");
		BindFunction("save", (SCRIPT_FUNCTION)&AudioFile::gm_save, "[this] save({string} fileName)", "Saves the current audio buffer to a RIFF/WAV file.");
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) {
			GM_CHECK_STRING_PARAM(file,0);
			if(!FileExists(_FILE(file))) { GM_EXCEPTION_MSG("File does not exist '%s'.", file); return GM_EXCEPTION; }
			if(!load(_FILE(file))) { GM_EXCEPTION_MSG("Error while loading file '%s'.", file); return GM_EXCEPTION; }
		}
		return GM_OK;
	}

	bool save(const char* fileName) {
		// write header to file
		FILE* hf = fopen(fileName, "wb");
		if(!hf) { Log(LOG_ERROR, "Error in saveWav() while opening '%s' for writing!", fileName); return false; }
		fwrite("RIFF", 4, 1, hf);
		unsigned long val = size+40-8;  fwrite(&val, 4, 1, hf);
		fwrite("WAVE", 4, 1, hf);
		fwrite("fmt ", 4, 1, hf);
		val = 16;  fwrite(&val, 4, 1, hf);
		SDY_WFX wfx;  memset(&wfx, 0, sizeof(SDY_WFX));
		wfx = this->format;
		fwrite(&wfx, 24-8, 1, hf);
		fwrite("data", 4, 1, hf);
		val = size; fwrite(&val, 4, 1, hf);

		// save data (raw)
		fwrite(data, size, 1, hf);
		fclose(hf);

		// fix sizes in header
		/*hf = fopen(fileName, "r+b");
		if(!hf) { Log(LOG_ERROR, "Error in saveWav() while patching '%s'!", fileName); return false; }
		fseek(hf, 0, SEEK_END);
		unsigned long length = ftell(hf);
		fseek(hf, 4, SEEK_SET);
		length -= 8; fwrite(&length, 4, 1, hf);
		fseek(hf, 40, SEEK_SET);
		length -= 44; fwrite(&length, 4, 1, hf);
		fseek(hf, SEEK_END, 0);
		fclose(hf);*/
		return true;
	}
	int gm_save(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(fileName, 0);
		save(fileName);
		return ReturnThis(a_thread);
	}

	bool load(const char* fileName) {
		Log(LOG_INFO, "Loading sound '%s' ...", fileName);

		// set i/o to standard file io
		SDY_IO io;
		io.open = (sdyio_fopen)fopen;
		io.read = (sdyio_fread)fread;
		io.seek = (sdyio_fseek)fseek;
		io.close = (sdyio_fclose)fclose;
		io.tell = (sdyio_ftell)ftell;
		io.write = (sdyio_fwrite)fwrite;

		// reset data
		readCursor = 0;
		writeCursor = 0;
		resize(0);

		// open stream
		void* stream = PCM_Open(_FILE(fileName), &format, &io, 0);
		if(stream == 0) { Log(LOG_ERROR, "Error while opening sound file '%s'!", fileName); return false; }

		// load complete stream into memory
		unsigned long dataSize = 0;
		unsigned long dataRead = 0;
		const unsigned long loadChunkSize = 1048576;
		do {
			resize(dataSize + loadChunkSize);
			dataRead = PCM_Stream(stream, &data[dataSize], loadChunkSize);
			dataSize += dataRead;
		} while(dataRead == loadChunkSize);
		resize(dataSize);

		// close stream
		PCM_Close(stream);
		stream = 0;

		setFormat(format.nSamplesPerSec, format.wBitsPerSample, format.nChannels);

		Log(LOG_DEBUG, "Audio format for '%s' is %dx%dx%d.", fileName, format.nSamplesPerSec, format.wBitsPerSample, format.nChannels);
		return true;
	}
	int gm_load(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		load(file);
		return ReturnThis(a_thread);
	}

};


class AudioDevice : public ScriptObject {
	friend int PortAudioCallback(const void* input, void* output, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* user);

public:
	AudioDevice() : ScriptObject(), stream(0) {
		id = "AudioDevice";
		ctor = "({int} sampleRate, {int} channels, {int} bufferSize, {int} deviceInIndex, {int} deviceOutIndex)";
		help = "Audio device for recording, playing and processing audio.";

		Pa_Initialize();
		dataIn = new AudioData();
		dataOut = new AudioData();

		BindFunction("open", (SCRIPT_FUNCTION)&AudioDevice::gm_open, "[this] open({int} sampleRate, {int} channels, {int} bufferSize, {int} deviceInIndex, {int} deviceOutIndex)", "Opens the audio device with the given settings. If the device is already open it will be closed first.");
		BindFunction("close", (SCRIPT_FUNCTION)&AudioDevice::gm_close, "[this] close()", "Closes the audio device.");
		BindFunction("pause", (SCRIPT_FUNCTION)&AudioDevice::gm_pause, "[this] pause()", "Pauses the audio stream.");
		BindFunction("unpause", (SCRIPT_FUNCTION)&AudioDevice::gm_unpause, "[this] unpause()", "Unpauses the audio stream.");
		BindFunction("play", (SCRIPT_FUNCTION)&AudioDevice::gm_play, "[this] play([AudioData] source)");
		BindFunction("stop", (SCRIPT_FUNCTION)&AudioDevice::gm_stop, "[this] stop([AudioData] buffer");
		BindFunction("process", (SCRIPT_FUNCTION)&AudioDevice::gm_process);
		BindMember("input", &dataIn, TYPE_OBJECT, 0);
		BindMember("output", &dataOut, TYPE_OBJECT, 0);
		dataIn->SetCppOwned(true);
		dataOut->SetCppOwned(true);
	}

	~AudioDevice() {
		close();
		delete(dataIn);
		delete(dataOut);
		Pa_Terminate();
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) { gm_open(a_thread); }
		return GM_OK;
	}

	void close() {
		if (!stream) { return; }
		Log(LOG_INFO, "Closing Audio device ...");
		WAIT_FOR_SOUNDCALLBACK();
		for (std::list<AudioData*>::iterator i = this->buffers.begin(); i != this->buffers.end(); i++) {
			this->stop((AudioData*)*i);
		}
		while (this->buffers.size() > 0) {
			this->stop(this->buffers.front());
			Log(LOG_DEBUG, "Waiting for %d audio buffers to stop...", this->buffers.size());
			TimeSleep(0.1);
		}
		pause();
		TimeSleep(0.5);
		WAIT_FOR_SOUNDCALLBACK();
		Pa_CloseStream(stream);
		stream = 0;
		dataIn->resize(0);
		dataOut->resize(0);
		Log(LOG_INFO, "Audio device closed.");
	}
	int gm_close(gmThread* a_thread) {
		close();
		return ReturnThis(a_thread);
	}

	bool open(int sampleRate = 22050, int channels = 1, unsigned long bufferSize = 256, int deviceInIndex = -1, int deviceOutIndex = -1) {
		close();  Log(LOG_INFO, "Opening Audio device (%dx%d) ...", sampleRate, channels);

		// get default device indices
		PaDeviceIndex devIn  = Pa_GetDefaultInputDevice();
		PaDeviceIndex devOut = Pa_GetDefaultOutputDevice();

		// overwrite device indices
		if(deviceInIndex >= -1) { devIn = deviceInIndex; }
		if(deviceOutIndex >= -1) { devOut = deviceOutIndex; }
		if(devIn != paNoDevice || devOut != paNoDevice) {
			// specify input device
			const PaDeviceInfo* deviceInfo;
			PaStreamParameters inputParams;
			if(devIn != paNoDevice) {
				inputParams.device = devIn;
				if(inputParams.device == paNoDevice) { Log(LOG_ERROR, "No audio input device found!"); return false; }
				deviceInfo = Pa_GetDeviceInfo(inputParams.device);
				Log(LOG_INFO, "Using '%s' with API '%s' for audio input...", deviceInfo->name, paHostApis[deviceInfo->hostApi]);
				inputParams.hostApiSpecificStreamInfo = 0;
				inputParams.channelCount= channels;
				inputParams.sampleFormat = paFloat32;
				inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;// * 0.35;
			}

			// specify output device
			PaStreamParameters outputParams;
			if(devOut != paNoDevice) {
				outputParams.device = devOut;
				if(outputParams.device == paNoDevice) { Log(LOG_ERROR, "No audio output device found!"); return false; }
				deviceInfo = Pa_GetDeviceInfo(outputParams.device);
				Log(LOG_INFO, "Using '%s' with API '%s' for audio output...", deviceInfo->name, paHostApis[deviceInfo->hostApi]);
				outputParams.hostApiSpecificStreamInfo = 0;
				outputParams.channelCount = channels;
				outputParams.sampleFormat = paFloat32;
				outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;// * 0.35;
			}

			// open stream
			PaError err = Pa_OpenStream(&stream, devIn != paNoDevice ? &inputParams : 0, devOut != paNoDevice ? &outputParams : 0, sampleRate, bufferSize, paClipOff, PortAudioCallback, (void*)this);
			if(err != paNoError) { Log(LOG_ERROR, "Error while creating Audio device (%s)!", Pa_GetErrorText(err)); return false; }
		}

		// set format
		this->dataIn->setFormat(sampleRate, 32, channels);
		this->dataOut->setFormat(sampleRate, 32, channels);

		// start stream
		unpause();
		return true;
	}
	int gm_open(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(sampleRate, 0);
		GM_CHECK_INT_PARAM(channels, 1);
		GM_CHECK_INT_PARAM(bufferSize, 2);
		GM_CHECK_INT_PARAM(devIn, 3);
		GM_CHECK_INT_PARAM(devOut, 4);
		this->open(sampleRate, channels, bufferSize, devIn, devOut);
		return GM_OK;
	}

	bool unpause() {
		if(!stream) { return false; }

		PaError err = Pa_StartStream(stream);
		if(err < 0) { Log(LOG_ERROR, "Error while unpausing AudioDevice stream (Pa_StartStream(): %s)!", Pa_GetErrorText(err)); return false; }

		int c=0; while(Pa_IsStreamActive(stream) != 1 && c < 20) { c++; TimeSleep(0.05); }
		if(err = Pa_IsStreamActive(stream) != 1) { Log(LOG_ERROR, "Error while unpausing AudioDevice stream (Pa_IsStreamActive(): %s)!", Pa_GetErrorText(err)); return false;}

		return true;
	}
	
	int gm_unpause(gmThread* a_thread) {
		this->unpause();
		return ReturnThis(a_thread);
	}

	bool pause() {
		if(!stream) { return true; }

		PaError err = Pa_IsStreamActive(stream);
		if(err < 0) { Log(LOG_ERROR, "Error while pausing AudioDevice stream (Pa_IsStreamActive(): %s)!", Pa_GetErrorText(err)); return false; }
		if(err == 0) { return true; }

		err = Pa_StopStream(stream);
		if(err < 0) { Log(LOG_ERROR, "Error while pausing AudioDevice stream (Pa_StopStream(): %s)!", Pa_GetErrorText(err)); return false; }

		int c=0; while(Pa_IsStreamStopped(stream) != 1 && c < 20) { c++; TimeSleep(0.05); }
		if(err = Pa_IsStreamStopped(stream) != 1) { Log(LOG_ERROR, "Error while pausing AudioDevice stream (Pa_IsStreamStopped(): %s)!", Pa_GetErrorText(err)); return false;}

		return true;
	}
	int gm_pause(gmThread* a_thread) {
		this->pause();
		return ReturnThis(a_thread);
	}

	AudioData* play(AudioData* buffer) {
		// TODO: multiplay
		WAIT_FOR_SOUNDCALLBACK();
		if(buffer->isPlaying && !buffer->multiplay) {
			buffer->rewindCursors();
			buffer->stop = false;
			return buffer;
		}
		if(buffer->isPlaying) {
			return buffer;
//			buffer = buffer->duplicate();
		}
		buffer->readCursor = 0;
		buffer->stop = false;
		buffer->isPlaying = 1;
		buffers.push_back(buffer);
		return buffer;
	}
	int gm_play(gmThread* a_thread) {
		int loop = 0;
		GM_CHECK_USER_PARAM(AudioData*, GM_TYPE_OBJECT, buffer, 0);
		AudioData* bufferPlay = play(buffer);
		return bufferPlay->ReturnThis(a_thread, bufferPlay->isDuplicate);
	}

	void stop(AudioData* buffer) {
//		WAIT_FOR_SOUNDCALLBACK();
//		buffers.remove(buffer);
		buffer->stop = true;
	}
	int gm_stop(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		GM_CHECK_USER_PARAM(AudioData*, GM_TYPE_OBJECT, buffer, 0);
		stop(buffer);
		return ReturnThis(a_thread, false);
	}

	void processInput(float* input, unsigned long numSamplesPerChannel) {
		// lock buffer
		this->dataIn->locked = true;
		
		// check
		if(!input) { return; }
		int bufferBytesSizeIn  = numSamplesPerChannel * sizeof(float) * this->dataIn->format.nChannels;

		// resize/create buffer if its to small
		if(this->dataIn->size < bufferBytesSizeIn) { this->dataIn->resize(bufferBytesSizeIn); }

		// remember write position
		unsigned long prevWriteCursor = this->dataIn->writeCursor;
		
		// copy input-data to devices input-buffer
		this->dataIn->writeBytes((unsigned char*)input, bufferBytesSizeIn);
		Log(LOG_INFO, "Reading %d bytes...", bufferBytesSizeIn);
		this->dataIn->bytesAvailable += bufferBytesSizeIn;

		// TODO: Do we need that???
		// set read cursor to previous write position
//		this->dataIn->readCursor = prevWriteCursor;
		
		// unlock buffer
		this->dataIn->locked = false;
	}

	void processOutput(float* output, unsigned long numSamplesPerChannel) {
		int bufferBytesSizeOut = numSamplesPerChannel * sizeof(float) * this->dataOut->format.nChannels;
		Log(LOG_INFO, "Writing %d bytes...", bufferBytesSizeOut);

		// ensure output data buffer is large enough
		if(this->dataOut->size < bufferBytesSizeOut) { this->dataOut->resize(bufferBytesSizeOut); }

		// clear output buffer first
		this->dataOut->clearRange(this->dataOut->writeCursor, bufferBytesSizeOut, 0);

		for(std::list<AudioData*>::iterator i = this->buffers.begin(); i != this->buffers.end();) {
			AudioData* buffer = *i;

			if (buffer->bytesAvailable < bufferBytesSizeOut) continue;
			//while (buffer->locked || buffer->bytesAvailable < bufferBytesSizeOut) { Log(LOG_DEBUG, "Waiting for data..."); }

			bool bufferActive = false;
			if( !buffer->stop ) {
				bufferActive = true;

				// compute number of samples per channel to mix
				long samplesLeft = 0, numSamplesPerChannelRead = numSamplesPerChannel;
				if(!buffer->loop) {

					// HOTFIX: detect sound-end
					/*if(buffer->readCursor+numSamplesPerChannel >= buffer->size) {
						bufferActive = false; buffer->stop = true;
						//break;
					}*/

					long samplesLeft = (buffer->size - buffer->readCursor) / buffer->format.nChannels / buffer->format.nBlockAlign;
					numSamplesPerChannelRead = samplesLeft;
					if(numSamplesPerChannelRead > (long)numSamplesPerChannel) {
						numSamplesPerChannelRead = numSamplesPerChannel;
					} else {
						bufferActive = false; buffer->stop = true;
					}
				}

				//Log(LOG_INFO, "%d  -  %d  -  %d", numSamplesPerChannelRead, numSamplesPerChannel, samplesLeft);

				// ???
				if(numSamplesPerChannelRead < 1/* (long)numSamplesPerChannel*/) {
					break;
				}

				// request data
				buffer->stream(numSamplesPerChannelRead);
				
				// check how much data is in the buffer
				/*int underflow = numSamplesPerChannelRead * this->dataOut->format.nBlockAlign * this->dataOut->format.nChannels - buffer->bytesAvailable;
				if (underflow > 0) {
					Log(LOG_WARNING, "Audio buffer underflow by %d bytes.");
				}
				else {
					Log(LOG_WARNING, "Avail: %d   Samp-Need:%d", buffer->bytesAvailable, numSamplesPerChannelRead);*/
					// mix!
					if (buffer->volume > 0.0) {
						this->dataOut->mix(buffer, numSamplesPerChannelRead);
					}
				//}
			}

			// remove finished audio-buffers from list (non-looping)
			if(!bufferActive) {
				buffer->stopRecording();
				buffer->isPlaying = 0;
				std::list<AudioData*>::iterator i_del = i++;
				this->buffers.erase(i_del);
				if( buffer->isDuplicate ) { delete buffer; }
			} else { i++; }
		}

		// copy buffer to port-audio for playback
		if(output) { this->dataOut->readBytes((unsigned char*)output, bufferBytesSizeOut); }
	}
	int gm_process(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(numSamplesPerChannel, 0);
		processOutput(0, numSamplesPerChannel);
		return GM_OK;
	}

public:
	PaStream* stream;
	AudioData* dataIn;
	AudioData* dataOut;
	std::list<AudioData*> buffers;
	volatile bool locked;
};


class AudioDeviceInfo : public ScriptObject {
public:
	AudioDeviceInfo() : ScriptObject(), index(-1), name(""), hostApi(0), hostApiName(""), maxInputChannels(0), maxOutputChannels(0), defaultSampleRate(0), defaultLowInputLatency(0), defaultLowOutputLatency(0), defaultHighInputLatency(0), defaultHighOutputLatency(0) {
		id = "AudioDeviceInfo";
		ctor = "({int} index, (optional) {string} name, (optional) {string} api, (optional) {int} minOutputChannels, (optional) {int} minInputChannels)";
		help = "Audio device information. Find and enumerate audio devices.";

		Pa_Initialize();
		numDevices = Pa_GetDeviceCount();
		BindMember("numDevices", &numDevices, TYPE_INT);
		BindMember("index", &index, TYPE_INT);
		BindMember("name", &name, TYPE_STRING);
		BindMember("apiIndex", &hostApi, TYPE_INT);
		BindMember("api", &hostApiName, TYPE_STRING);
		BindMember("inputChannels", &maxInputChannels, TYPE_INT);
		BindMember("outputChannels", &maxOutputChannels, TYPE_INT);
		BindMember("sampleRate", &defaultSampleRate, TYPE_FLOAT);
		BindMember("lowInputLatency", &defaultLowInputLatency, TYPE_FLOAT);
		BindMember("lowOutputLatency", &defaultLowOutputLatency, TYPE_FLOAT);
		BindMember("highInputLatency", &defaultHighInputLatency, TYPE_FLOAT);
		BindMember("highOutputLatency", &defaultHighOutputLatency, TYPE_FLOAT);
		BindFunction("getDevice", (SCRIPT_FUNCTION)&AudioDeviceInfo::gm_getDevice, "[this] getDevice({int} index, (optional) {string} name, (optional) {string} api, (optional) {int} minInputChannels, (optional) {int} minOutputChannels)", "Gets the device that matches the name. Only the given length will be compared, case is ignored. If no devices matches, the index is set to -1.");
	}

	~AudioDeviceInfo() {
		Pa_Terminate();
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) { gm_getDevice(a_thread); }
		return GM_OK;
	}

	bool _FindInStringNoCase(const char* a, const char* b) {
		int b_len = strlen(b);
		if(b_len <= 0) { return true; }
		int a_len = strlen(a);
		for(int c=0,i=0; i<a_len; i++) {
			if(tolower(a[i]) == tolower(b[c])) {
				if(++c == b_len) { return true; }
			} else { c = 0; }
		}
		return false;
	}

	bool getDevice(int subindex, const char* name, const char* api, int minInputChannels, int minOutputChannels) {
		this->index = -1;
		PaDeviceIndex numDevices = Pa_GetDeviceCount();
		for(int i=0; i<numDevices; i++) {
			const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
			if(info->maxOutputChannels >= minOutputChannels && info->maxInputChannels >= minInputChannels && _FindInStringNoCase(info->name, name) && _FindInStringNoCase(paHostApis[info->hostApi], api)) {
				if(!subindex--) {
					const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
					this->index = i;
					this->name = info->name;
					this->hostApi = info->hostApi;
					this->hostApiName = paHostApis[info->hostApi];
					this->maxInputChannels = info->maxInputChannels;
					this->maxOutputChannels = info->maxOutputChannels;
					this->defaultSampleRate = info->defaultSampleRate;
					this->defaultLowInputLatency = info->defaultLowInputLatency;
					this->defaultLowOutputLatency = info->defaultLowOutputLatency;
					this->defaultHighInputLatency = info->defaultHighInputLatency;
					this->defaultHighOutputLatency = info->defaultHighOutputLatency;
					return true;
				}
			}
		}
		Log(LOG_WARNING, "No audio device found that meets the given specifications.");
		return false;
	}
	int gm_getDevice(gmThread* a_thread) {
		int minOutputChannels = 0, minInputChannels = 0;
		const char* name;
		const char* api;

		GM_CHECK_INT_PARAM(subindex, 0);
		a_thread->ParamString(1, name, "\0");
		a_thread->ParamString(2, api, "\0");
		a_thread->ParamInt(3, minInputChannels, 0);
		a_thread->ParamInt(4, minOutputChannels, 0);

		a_thread->PushInt(getDevice(subindex, name, api, minInputChannels, minOutputChannels));
		return ReturnThis(a_thread);
	}

public:
	int numDevices;
	int index;
	std::string name;
	std::string hostApiName;
	int hostApi;
	int maxInputChannels;
	int maxOutputChannels;
	float defaultSampleRate;
	float defaultLowInputLatency;
	float defaultLowOutputLatency;
	float defaultHighInputLatency;
	float defaultHighOutputLatency;
};

