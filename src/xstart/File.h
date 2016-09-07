#ifndef _FILE_H_
#define _FILE_H_

#include "ScriptObject.h"
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

class File : public ScriptObject {
public:

	File() : ScriptObject() {
		id = "File";
		ctor = "((optional){string} file, (optional){string} mode=\"rb\")";
		help = "Opens a file for reading or writing.";

		fileName = "";
		hf = 0;
		bytesRead = 0;

		BindFunction("open",   (SCRIPT_FUNCTION)&File::gm_open, "[this] open({string} fileName, {string} mode)", "Opens a file in the given mode (\"r\" = read, \"w\" = write, \"rb\" = read binary, \"wb\" = write binary, \"a\" = append, \"ab\" = append binary).");
		BindFunction("close",  (SCRIPT_FUNCTION)&File::gm_close, "close()", "Closes the file. This is also done by garbage collecting the object.");
		BindFunction("write",  (SCRIPT_FUNCTION)&File::gm_write, "[this] write({string} data, (optional) {int} length)", "<b>Subject to change.</b> Write string to file.");
		BindFunction("read",   (SCRIPT_FUNCTION)&File::gm_read, "{string} read( (optional) {int} length)", "<b>Subject to change.</b> Reads 'length' of bytes from file or the whole file if length is not specified.");
		// TODO: readLine, readUntil
		BindFunction("exists", (SCRIPT_FUNCTION)&File::gm_exists, "{int} exists({string} fileName)", "Checks if a given file name exists. Returns zero on failure or non-zero if the file exists.");
		BindFunction("delete", (SCRIPT_FUNCTION)&File::gm_delete, "{bool} delete({string} fileName)", "Deletes the given file. Returns {false} only aif the file exists and can not be removed.");
		BindFunction("seek",   (SCRIPT_FUNCTION)&File::gm_seek, "[this] seek({int} position)", "Moves the file cursor to the given position. No error checking performed.");
		BindFunction("tell",   (SCRIPT_FUNCTION)&File::gm_tell, "{int} tell()", "Returns the current file cursor position.");
		BindFunction("size",   (SCRIPT_FUNCTION)&File::gm_size, "{int} size()", "Returns the file size in bytes.");

		BindMember("bytes", &bytesRead, TYPE_INT, 0, "{int} bytes", "Number of bytes read by last read operation.");
	}

	~File() { close(); }

	int Initialize(gmThread* a_thread) {
		const char* file = 0;
		const char* mode = "rb";

		if(a_thread->GetNumParams() >= 1) {
			a_thread->ParamString(0, file, "");
		}
		if(a_thread->GetNumParams() >= 2) {
			a_thread->ParamString(1, mode, "rb");
		}

		if(file) {
			open(file, mode);
		}

		return GM_OK;
	}

	void _checkError() {
		if(!hf) {
			Log(LOG_ERROR, "No valid file handle for '%s'!", fileName.c_str());
		}
		int err = ferror(hf);
		if(!err) {
			return;
		}

		char error[1024];
		sprintf(error, "Error opening file '%s'", fileName.c_str() );
		perror(error);
		Log(LOG_ERROR, error);

		if(feof(hf) == 0) {
			Log(LOG_ERROR, "End of file for '%s'.", fileName.c_str() );
		}

		clearerr(hf);
	}

	bool exists(const char* file) {
		return access(file, 0) != -1;
	}
	int gm_exists(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt(exists(file));
		return GM_OK;
	}

	bool del(const char* file) {
		if(!exists(file)) return true;
		int r = remove(file);
		if(r == 0) { return true; }
		return false;
	}
	int gm_delete(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt(del(file));
		return GM_OK;
	}

	bool open(const char* file, const char* mode) {
		if(hf) { fclose(hf); hf = 0; }
		fileName = file;
		Log(LOG_DEBUG, "Opening file '%s' in mode '%s'.", file, mode);
		int count = 0;
		while(!hf && count++ < 3) {
			hf = fopen(file, mode);
			TimeSleep(0.1);
		}
		_checkError();
		return hf != 0;
	}
	int gm_open(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		GM_CHECK_STRING_PARAM(mode, 1);
		open(file, mode);
		return ReturnThis(a_thread);
	}

	void close() {
		if(hf) {
			Log(LOG_DEBUG, "Closing file '%s'.", fileName.c_str());
			fclose(hf);
		}
		hf = 0;
		fileName = "";
	}
	int gm_close(gmThread* a_thread) {
		close();
		return ReturnThis(a_thread);
	}

	void write(const char* data, unsigned int length) {
		if(!hf) {
			return;
		}
		fwrite(data, 1, length, hf);
		_checkError();
	}

	int gm_write(gmThread* a_thread) {
		const char* data;
		int length = 0;

		a_thread->ParamString(0, data, "");
		if(a_thread->GetNumParams() > 1) {
			a_thread->ParamInt(0, length, 0);
		} else {
			length = strlen(data);
		}

		if(a_thread->GetNumParams() > 1) {
			a_thread->ParamInt(1, length, 0);
		}
		write(data, length);
		return ReturnThis(a_thread);
	}

	unsigned int size() {
		if(!hf) return 0;
		long pos = ftell(hf);
		fseek(hf, 0, SEEK_END);
		unsigned int size = ftell(hf);
		fseek(hf, pos, SEEK_SET);
		return size;
	}
	int gm_size(gmThread* a_thread) {
		a_thread->PushInt(size());
		return GM_OK;
	}

	unsigned int read(char* buffer, unsigned int length) {
		size_t bytes = fread(buffer, 1, length, hf);
		_checkError();
		return bytes;
	}
	int gm_read(gmThread* a_thread) {
		int length = 0;

		if(a_thread->GetNumParams() == 1) {
			a_thread->ParamInt(0, length, 0);
		} else {
			length = size();
		}

		char* _buffer = (char*)malloc(length+1);
		memset(_buffer, 0, length+1);
		bytesRead = read(_buffer, length);
		a_thread->PushNewString(_buffer, bytesRead);
		free(_buffer);

		return GM_OK;
	}

	void seekBegin(unsigned long position) {
		if(fseek(hf, position, SEEK_SET) != 0) { Log(LOG_ERROR, "File seeking on file '%s' to position %d failed!", fileName.c_str(), position); }
	}
	int gm_seek(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(position, 0);
		seekBegin(position);
		return ReturnThis(a_thread);
	}
	
	unsigned long tell() {
		return ftell(hf);
	}
	int gm_tell(gmThread* a_thread) {
		a_thread->PushInt(tell());
		return GM_OK;
	}
	
public:
	long bytesRead;
	std::string fileName;
	FILE* hf;
};


#endif
