#include <stdio.h>
#include <corela.h>
#include <gm/gmMachine.h>

static char _libPath[1024];

void SetLibraryPath(const char* path) {
	strncpy(_libPath, path, 1023);
}

// This is a helper function to find files in lib path
std::string FindFile(const char* file) {
	if(FileExists(file)) { return std::string(file); }
	std::string libFile = (std::string(_libPath) + std::string("/") + std::string(file));
	if(FileExists(libFile.c_str())) { return libFile; }
	Log(LOG_ERROR, "File '%s' not found!", file);
	return std::string(file);
}

// Fast preprocessor
char* _PreprocessScript(const char* file, char* out, unsigned long* cursor, unsigned long* maxSize, CONT* definitions, CONT* ifdefStack, gmMachine* machine) {
	// if no output buffer was given, create one.
	if(!out) {
		*cursor = 0;
		*maxSize = 2048;
		out = (char*)malloc(*maxSize);
	}

	// load file
	coDword fsize = 0;
	if(FileExists(file)) { fsize = FileReadText(file, 0, 0); }
	if(fsize == 0) {
		char libfile[1024+1024];
#ifdef _WIN32
		sprintf(libfile, "%s\\%s", _libPath, file);
#else
		sprintf(libfile, "%s/%s", _libPath, file);
#endif
		if(FileExists(libfile)) { fsize = FileReadText(libfile, 0, 0); }
		file = libfile;
	}
	if(fsize == 0) {
		Log(LOG_FATAL, "File not found '%s'!", file);
		exit(0);
	}
	char* buffer = (char*)malloc(fsize);
	FileReadText(file, buffer, 0);

	// FIX: add source code to script engine
	machine->AddSourceCode(buffer, file);

	// variables for parsing
	coDword fileCursor = 0;
	coDword lineCursor = 0;
	coDword lineLength = 0;
	char command[16+1];
	char param[64+1];

	// parse buffer
	while(buffer[fileCursor] != 0) {
		// line pointer into array
		lineCursor = fileCursor;
		char* line = &buffer[lineCursor];

		// fetch line length
		for(lineLength=0; buffer[fileCursor+lineLength] != '\n' && buffer[fileCursor+lineLength] != '\0'; lineLength++) {}
		lineLength++;

		// ignore whitespace
		for(; (buffer[fileCursor] == ' ' || buffer[fileCursor] == '\t') && fileCursor <= lineCursor + lineLength; fileCursor++) {}

		// check preprocessor directive
		if(buffer[fileCursor] == '#') {
			// read preprocessor command
			int i=0;
			for(++fileCursor; buffer[fileCursor] >= 'a' && buffer[fileCursor] <= 'z'; fileCursor++) {
				if(i < 16) {
					command[i++] = buffer[fileCursor];
				}
			}
			command[i] = '\0';

			if(strncmp(command, "include", 16) == 0) {
				// parse parameter
				for(; (buffer[fileCursor] == ' ' || buffer[fileCursor] == '\t') && fileCursor <= lineCursor + lineLength; fileCursor++) {}
				char delim = buffer[fileCursor++];
				int i=0;
				for(; buffer[fileCursor] != delim; fileCursor++) {
					param[i++] = buffer[fileCursor];
				}
				param[i] = '\0';

				// do include ...
				out = _PreprocessScript(param, out, cursor, maxSize, definitions, ifdefStack, machine);
//				(*cursor)--;  // trim trailing null-byte
				/*} else if(strncmp(command, "define", 16) == 0) {
					// parse parameter
					for(; (buffer[fileCursor] == ' ' || buffer[fileCursor] == '\t') && fileCursor <= lineCursor + lineLength; fileCursor++) {}

					*/
			} else {
				Log(LOG_ERROR, "Unknown preprocessor command '%s'.", command);
			}

			fileCursor = lineCursor + lineLength;
			if(buffer[fileCursor-1] == '\0') {
				break;
			}
			continue;
		}

		// put line into output buffer
		if(*maxSize < *cursor + lineLength + 1) { // resize buffer
			*maxSize = *cursor + lineLength + 1 + 2048;
			out = (char*)realloc(out, *maxSize);
		}
		memcpy(&out[*cursor], line, lineLength);
		*cursor += lineLength;
		out[*cursor] = '\0';
		fileCursor = lineCursor + lineLength;
		if(buffer[fileCursor-1] == '\0') {
			break;
		}
	}

	return out;
}

char* PreprocessScript(const char* file, gmMachine* machine) {
	CONT* definitions = cnew();
	CONT* ifdefStack = cnew();

#ifdef _DEBUG
	cpush(definitions, "DEBUG");
#else
	cpush(definitions, "RELEASE");
#endif

	unsigned long cursor=0, maxSize=0;
	char* buffer = _PreprocessScript(file, 0, &cursor, &maxSize, definitions, ifdefStack, machine);

	// TODO!!!
	while(ccount(definitions)) {
		cpop(definitions);
	}
	while(ccount(ifdefStack)) {
		cpop(ifdefStack);
	}
	cdel(definitions);
	cdel(ifdefStack);

	return buffer;
}
