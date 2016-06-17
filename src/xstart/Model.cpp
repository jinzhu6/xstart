#include <corela.h>

void Load3D_OBJ(const char* file) {
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
			// eat any additional space
			while(c == ' ' || c == '\t') { c = buffer[++i]; }

			// parse parameters here
			for(; i<fsize; i++) {
				c = buffer[i];

				switch(c) {
				case ' ':
				case '\t':
					// eat space
					while(c == ' ' || c == '\t') { c = buffer[++i]; }
					n_params++;
					break;
				case '\n':
					if(i_sym) {
						// TODO: flush line
						n_params = 0;
						i_sym = 0;
					}
				default:
					// add character to current parameter
					param[i_param[n_params]++][n_params] = c;
					break;
				}
				if(c == '\n') { break; }
			}
			// TODO: parameter mode
			break;
		case '\n':
			// eat space
			while(c == ' ' || c == '\t') { c = buffer[++i]; }
			break;
		case '#':
			// eat till newline
			while(c != '\n') { c = buffer[++i]; }
			break;
		default:
			if(i_sym < 64) { sym[i_sym++] = c; }
			break;
		}
	}
}
