#ifndef _JSON_H_
#define _JSON_H_

// OBSOLETE: Use  "Array.parse({string})"  to parse JSON to an object representation (see examples).

#if 0
#include "ScriptObject.h"

class Json : public ScriptObject {
public:
	Json() : ScriptObject() {
		id = "Json";
		help = "JSON parser.";

	}

	int parse(const char* raw) {
		int n = 0;
		char key[64];
		int n_key = 0;
		char value[64];
		int n_value = 0;
		Json* o;

		while(raw[n] != '\0') {
			char c = raw[n++];

			switch(c) {
			case '}':
				return n;
			case ':':
				// read value
				n_value = 0;
				bool valueDone = false;
				bool valueIsString = false;
				while(raw[n] != '\0' && !valueDone) {
					char c = raw[n++];

					switch(c) {
					case '[':
						break;
					case '{':
						// decent into new object
						o = new Json();
						n += o->parse(&raw[n]);
						SetMemberObject(key, o);
						break;
					case '"':
						// read string value
						valueIsString = true;
						while(raw[n] != '\0') {
							c = raw[n++];
							if(c == '"') { break; }
							value[n_value++] = c;
						}
						break;
					case ' ': // ignore whitespace
						break;
					case ',':
					case '\n':
					case '}':
					case ']':
						valueDone = true;
						break;
					default:
						value[n_value++] = c;
						break;
					}
				}
				// store value
				SetTableVar(key, value);
				if(raw[n-1] == '}') { n--; } // push back delimiter, if present
				break;

			case '"':
				// read key
				n_key = 0;
				c = raw[++n];
				while(c != '"' && n_key < 64) {
					key[n_key++] = c;
					c = raw[++n];
				}
				break;
			}

			n++;
		}
	}

};
#endif

#endif
