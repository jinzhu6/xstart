#ifndef _DATA_H_
#define _DATA_H_

#include "ScriptObject.h"


class Data : public ScriptObject {
public:

	Data() : ScriptObject(), data(0), size(0), readCursor(0), writeCursor(0), loop(true) {
		id = "Data";
		help = "";

		BindFunction("loadRaw", (SCRIPT_FUNCTION)&Data::gm_loadRaw, "[this] loadRaw({string} file)", "Loads the given file into data memory as is.");
		BindFunction("saveRaw", (SCRIPT_FUNCTION)&Data::gm_saveRaw, "[this] saveRaw({string} file)", "Saves the data into a file as is.");
		BindFunction("appendRaw", (SCRIPT_FUNCTION)&Data::gm_appendRaw, "[this] appendRaw({string} file)", "Saves the data into a file as is.");
		BindFunction("peek", (SCRIPT_FUNCTION)&Data::gm_peek, "{int} peek({int} addr)", "Reads a single byte from the given address.");
		BindFunction("poke", (SCRIPT_FUNCTION)&Data::gm_poke, "[this] poke({int} addr, {int} value)", "Writes a single byte to the given address.");
		BindFunction("resize", (SCRIPT_FUNCTION)&Data::gm_resize, "{int} resize({int} newsize)", "Resizes the data array length.");
		BindFunction("copyFrom", (SCRIPT_FUNCTION)&Data::gm_copyFrom, "[this] copyFrom({Data} source)", "Makes a copy of the source data. Any old data in this object will be lost.");
		BindFunction("insert", (SCRIPT_FUNCTION)&Data::gm_insert, "[this] insert({Data} source, {int} position)", "Copies the source data to the given position. The data may be resized to fit the added content.");
		BindMember("size", &size, TYPE_INT, 0, "{int} size", "(readonly) Size of data array.");
		BindMember("loop", &loop, TYPE_INT);
		BindMember("readCursor", &readCursor, TYPE_INT);
		BindMember("writeCursor", &readCursor, TYPE_INT);
	}

	~Data() {
		if(data) { free(data); }
	}

	bool loadRaw(const char* file) {
		if(!FileExists(_FILE(file))) { return false; }
		unsigned long fsize = FileGetSize(_FILE(file));
		if(!this->resize(fsize)) { return false; }
		FileReadBuffer(_FILE(file), (char*)data, &fsize);
		return true;
	}
	int gm_loadRaw(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		loadRaw(file);
		return ReturnThis(a_thread);
	}

	bool saveRaw(const char* file) {
		FILE* hf = fopen(file, "wb");
		if(!hf) { Log(LOG_ERROR, "Error in saveRaw() while opening '%s' for writing!", file); return false; }
		fwrite(data, size, 1, hf);
		fclose(hf);
		return true;
	}
	int gm_saveRaw(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		saveRaw(file);
		return ReturnThis(a_thread);
	}

	bool appendRaw(const char* file) {
		FILE* hf = fopen(file, "a");
		if(!hf) { Log(LOG_ERROR, "Error in appendRaw() while opening '%s' for writing!", file); return false; }
		fwrite(data, size, 1, hf);
		fclose(hf);
		return true;
	}
	int gm_appendRaw(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		appendRaw(file);
		return ReturnThis(a_thread);
	}

	virtual std::string toString() {
		// todo: use character encoding for binary data
		return std::string((char*)data, size);
	}

	int peek(int addr) {
		if (addr >= (int)size) { Log(LOG_ERROR, "Access to address %d denied! Size of data is %d!", addr, size); return 0; /*return GM_EXCEPTION;*/ }
		return data[addr];
	}
	int gm_peek(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(addr, 0);
		a_thread->PushInt(peek(addr));
		return GM_OK;
	}

	void poke(int addr, int value) {
		if (addr >= (int)size) {
			Log(LOG_ERROR, "Write to address %d denied! Size of data is %d!", addr, size); return; /*return GM_EXCEPTION;*/
		}
		data[addr] = value;
	}
	int gm_poke(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_INT_PARAM(addr, 0);
		GM_CHECK_INT_PARAM(value, 1);
		poke(addr, value);
		return ReturnThis(a_thread);
	}

	bool resize(unsigned long newsize) {
		if(newsize == size) { return true; }
		if(newsize == 0 && data != 0) { free(data); data = 0; size = 0; return true; }
		if(data == 0) { data = (unsigned char*)malloc(newsize);  size = newsize; }
		else { data = (unsigned char*)realloc(data, newsize);  size = newsize; }
		if(data == 0) {
			Log(LOG_FATAL, "Out of memory while resizing data to size %d!", newsize);
			return false;
		}
		return true;
	}
	int gm_resize(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(newsize, 0);
		a_thread->PushInt( resize(newsize) );
		return GM_OK;
	}

	void copyFrom(Data* src) {
		resize(src->size);
		memcpy(data, src->data, src->size);
	}
	int gm_copyFrom(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Data*, GM_TYPE_OBJECT, src, 0);
		copyFrom(src);
		return ReturnThis(a_thread);
	}

	bool insert(unsigned char* dataIn, unsigned long len, int pos = 0) {
		if(pos + len > size) { if(!resize(pos + len)) { return false; } }
		memcpy(&data[pos], dataIn, len);
		return true;
	}
	int gm_insert(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Data*, TYPE_OBJECT, din, 0);
		int pos; a_thread->ParamInt(1, pos, 0);
		if(!insert(din->data, din->size, pos)) { return ReturnNull(a_thread); }
		return ReturnThis(a_thread);
	}

	void clear(int clearTo) {
		memset(data, clearTo, size);
	}
	void clearRange(int offset, int length, int clearTo) {
		while(length > 0) {
			// clamp length to buffer
			unsigned long chunkLength = length;
			unsigned long bufferPos = offset % this->size;
			if(chunkLength > this->size - bufferPos) { chunkLength = this->size - bufferPos; }
			if(chunkLength == 0) { Log(LOG_ERROR, "Error in clearEx() - chunk-length was zero!"); return; }

			// copy to ringbuffer
			memset(&this->data[bufferPos], 0, chunkLength);
			offset += chunkLength;
			length -= chunkLength;
		}
	}

	virtual void _readRingBytes(unsigned long offset, unsigned char* buffer, int length) {
		while(length > 0) {
			// clamp length to buffer
			unsigned long chunkLength = length;
			unsigned long bufferPos = offset % this->size;
			if(chunkLength > this->size - bufferPos) { chunkLength = this->size - bufferPos; }
			if(chunkLength == 0) { Log(LOG_ERROR, "Error in readRingBytes() - chunk-length was zero!"); return; }

			// copy to ringbuffer
			memcpy(buffer, &this->data[bufferPos], chunkLength);
			offset += chunkLength;
			buffer += chunkLength;
			length -= chunkLength;
		}
	}

	virtual void _writeRingBytes(unsigned long offset, unsigned char* buffer, unsigned long length) {
		while(length > 0) {
			// clamp length to buffer
			unsigned long chunkLength = length;
			unsigned long bufferPos = offset % this->size;
			if(chunkLength > this->size - bufferPos) { chunkLength = this->size - bufferPos; }
			if(chunkLength == 0) { Log(LOG_ERROR, "Error in writeRingBytes() - chunk-length was zero!"); return; }

			// copy to ringbuffer
			memcpy(&this->data[bufferPos], buffer, chunkLength);
			offset += chunkLength;
			buffer += chunkLength;
			length -= chunkLength;
		}
	}

	/*virtual unsigned long readBytes(unsigned long offset, unsigned char* buffer, unsigned long length) {
		if(offset >= this->size) { return 0; }
		if(length > this->size - offset) { length = this->size - offset; }
		memcpy(buffer, &data[offset], length);
		return length;
	}*/

	// readBytes - Reads "length" bytes of data into "buffer" and increases the "readCursor". If "loop" is enabled, it will use the ringbuffer to loop the data, otherwise - at the end of the buffer - it will return less bytes than requested.
	virtual unsigned long readBytes(unsigned char* buffer, unsigned long length) {
		if(!loop && readCursor+length > this->size) { length = this->size - readCursor; }
		else { readCursor %= this->size; }
		_readRingBytes(readCursor, buffer, length);
		readCursor = readCursor + length;
		return length;
	}

	// writeBytes - Writes "length" bytes of data into "buffer" at the current write cursor. The "writeCursor" is incremented accordingly. Data will wrap-around at the end of the ring-buffer.
	virtual unsigned long writeBytes(unsigned char* buffer, unsigned long length) {
		if(length > this->size) { Log(LOG_WARNING, "Error in AudioData.writeBytes(), buffer too small (%d size, % to write)! Data will overlap!", this->size, length); }
		_writeRingBytes(writeCursor, buffer, length);
		writeCursor = (writeCursor + length) % this->size;
		return length;
	}

	// transferData - Writes data from "in" into "this". The "writeCursor" of "this" and the "readCursor" of "in" are changed accordingly.
	virtual unsigned long transferData(Data* in, unsigned long length) {
		while(length) {
			unsigned long l = (in->size - in->readCursor) > (this->size - this->readCursor) ? this->size - this->readCursor : in->size - in->readCursor;
			if(l > length) { l = length; }
			writeBytes(&in->data[in->readCursor], l);
			in->readCursor = (in->readCursor + l) % in->size;
			length -= l;
		}
		return length;
	}

	// rewind read and write cursors
	virtual void rewindCursors() {
		readCursor = 0;
		writeCursor = 0;
	}

public:
	bool loop; // loop readBytes(). other functions are not affected, writes are always looping.
	unsigned char* data;  // data of bytes
	unsigned long size;   // size of data
	unsigned long readCursor;
	unsigned long writeCursor;
};


#endif
