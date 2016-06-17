#include "corela.h"
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

bool FileExists(const char* file) {
	FILE* hf = fopen(file, "rb");
	if(hf == 0) {
		char workDir[2048];
		getcwd(workDir, 2047);
		workDir[2047] = '\0';
		Log(LOG_DEBUG, "File '%s' not found or is locked in function FileExists. CWD is '%s'", file, workDir);
		return false;
	}
	return true;
}

coDword FileGetSize(const char* file) {
	FILE* hf = fopen(file, "rb");
	if(hf == 0) {
		Log(LOG_ERROR, "File \"%s\" not found!", file);
		return 0;
	}
	fseek(hf, 0, SEEK_END);
	unsigned int size = ftell(hf);
	fclose(hf);
	return size;
}

char* FileReadBuffer(const char* file, char* buffer, coDword* maxSize) {
	FILE* hf = fopen(file, "rb");
	if(hf == 0) {
		Log(LOG_ERROR, "File \"%s\" not found!", file);
		return buffer;
	}
	*maxSize = fread(buffer, 1, *maxSize, hf);
	fclose(hf);
	return buffer;
}

coDword FileReadText(const char* file, char* bufferOut, coDword* sizeInOut) {
	// get total file size
	coDword fsize = FileGetSize(file);
	if(fsize == 0) { return 0; }

	// read file in temp buffer
	unsigned char* buffer = (unsigned char*)malloc(fsize + 2);
	FileReadBuffer(file, (char*)buffer, &fsize);

	// add two zero bytes for string / wide-string operations
	buffer[fsize] = '\0';
	buffer[fsize+1] = '\0';

	// detect UTF-16 LE
	if( (buffer[0] == 0xff && buffer[1] == 0xfe) || (buffer[0] == 0xfe && buffer[1] == 0xff)) {
		Log(LOG_WARNING, "Multibyte Unicode detected in file '%s', better use UTF-8 encoding instead.", file);

#ifdef _WIN32
		// swap endianness in buffer for (UTF-16 BE)
		if((buffer[0] == 0xfe && buffer[1] == 0xff)) {
			for(int n=0; n<fsize-1; n+=2) {
				unsigned char tmp = buffer[n];
				buffer[n] = buffer[n+1];
				buffer[n+1] = tmp;
			}
		}

		// allocate new buffer for utf-8 encoding
		unsigned char* bufferUTF8 = (unsigned char*)malloc(fsize + 1);
		memset(bufferUTF8, 0, fsize+1);

		// convert to utf-8 - (the result in bufferUTF8 will contain a NULL byte at the end)
		int size = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)&buffer[2], -1, (LPSTR)bufferUTF8, fsize, NULL, NULL);
		free(buffer);

		// handle sizeInOut
		if(sizeInOut) {
			if(*sizeInOut < size && bufferOut) { size = *sizeInOut; }
			else { *sizeInOut = size; }
		}

		// copy utf-8 buffer to output buffer
		if(bufferOut) {
			memcpy(bufferOut, bufferUTF8, size);
		}
		free(bufferUTF8);

		// return size
		return size;
#else
		Log(LOG_FATAL, "Multibyte Unicode found in '%s', this is currently not supported under Linux.", file);
#endif
	}

	// detect UTF-8
	if(buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {

		// subtract BOM from size and add zero byte char
		int size = fsize - 3 + 1;

		// handle sizeInOut
		if(sizeInOut) {
			if(*sizeInOut < size && bufferOut) { size = *sizeInOut; }
			else { *sizeInOut = size; }
		}

		// copy utf-8 buffer to output buffer
		if(bufferOut) {
			memcpy(bufferOut, &buffer[3], size);
		}
		free(buffer);

		// return size
		return size;
	}

	// handle normal ASCII below

	// add null-terminating byte to size
	int size = fsize + 1;

	// handle sizeInOut
	if(sizeInOut) {
		if(*sizeInOut < size && bufferOut) { size = *sizeInOut; }
		else { *sizeInOut = size; }
	}

	// copy utf-8 buffer to output buffer
	if(bufferOut) {
		memcpy(bufferOut, buffer, size);
	}
	free(buffer);

	// return size
	return size;
}

void FileSaveBuffer(const char* file, const char* buffer, coDword length) {
	if(length <= 0) { length = strlen(buffer); }
	FILE* hf = fopen(file, "wb");
	fwrite(buffer, length, 1, hf);
	fclose(hf);
}
