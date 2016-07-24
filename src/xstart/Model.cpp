#include <corela.h>
#include "Model.h"

bool IsEmptyCharacter(char c) {
	if(c=='\n' || c=='\f' || c=='\r' || c=='\t' || c=='\v' || c=='\a') return true;
	return false;
}

void Load3D_OBJ(Mesh* mesh, const char* file) {
	// read file
	char* buffer;
	coDword fsize;
	FileReadText(file, 0, &fsize);
	buffer = (char*)malloc(fsize);
	FileReadText(file, buffer, 0);

	// parse
	char c, sym[64], param[64][5];
	int i_sym = 0, n_params = 0, i_param[5] = {0,0,0,0,0};
	for(coDword i=0; i<fsize; i++) {
		c = buffer[i];

		switch(c) {
		case ' ':
		case '\t':
			// eat any preceeding space
			while(c == ' ' || c == '\t') { c = buffer[++i]; }

			// parse parameters here
			for(; i<fsize; i++) {
				c = buffer[i];

				switch(c) {
				case ' ':
				case '\t':
					// eat space
					while(c == ' ' || c == '\t') { c = buffer[++i]; }
					n_params++; // next param
					break;
				case '\n':
					if(i_sym) {
						// execute symbol

					}
					n_params = 0;
					for(int k=0; k<5; k++) { i_param[k]=0; }
					i_sym = 0;
				default:
					// add character to current parameter
					param[i_param[n_params]++][n_params] = c;
					break;
				}
				if(c == '\n') { break; }
			}
			break;
		case '\n':
			// eat space
			while(IsEmptyCharacter(c) && i<fsize) { c = buffer[++i]; }
			break;
		case '#':
			// eat till newline
			while(c != '\n' && c != '\0' && i < fsize) { c = buffer[++i]; }
			break;
		default:
			// read symbol
			if(i_sym < 64) { sym[i_sym++] = c; }
			break;
		}
	}
}
