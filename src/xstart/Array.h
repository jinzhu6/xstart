#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "ScriptObject.h"
#include <string>


class ArrayObject : public ScriptObject {
public:

	ArrayObject() : ScriptObject() {
		id = "Array";
		help = "Dynamic array class for storing a linear array of objects. <b>Please note, that currently only objects derived from [Object] can be stored, no primitive types like string or int. This may change in the future.</b>";

		length = 0;

		BindFunction("get", (SCRIPT_FUNCTION)&ArrayObject::gm_get, "[Object] get({int} index)", "Get the object at the given index.");
		BindFunction("add", (SCRIPT_FUNCTION)&ArrayObject::gm_add, "[Object] add([Object] object)", "Adds an object to the end of the array. The added object is returned.");
		BindFunction("set", (SCRIPT_FUNCTION)&ArrayObject::gm_set, "[this] set({int} index, [Object] object)", "Sets the given object to the given index.");
		BindFunction("remove", (SCRIPT_FUNCTION)&ArrayObject::gm_remove, "[this] remove({int} index)", "Removes the element at the given index from the array.");
		BindFunction("find", (SCRIPT_FUNCTION)&ArrayObject::gm_find, "{int} find([Object] object)", "Finds the index of the given object, returns -1 if the object is not in the array.");
		BindFunction("length", (SCRIPT_FUNCTION)&ArrayObject::gm_length, "{int} length()", "Returns the length of the array.");
		BindFunction("parse", (SCRIPT_FUNCTION)&ArrayObject::gm_parse, "[this] parse({string} json)", "Parses a JSON representation to an object.");
		// TODO: Resize/Size array function
	}

	~ArrayObject() { }

	gmVariable iteratorGet(int n) {
		if(n >= length) { gmVariable v; v.Nullify(); return v; }
		char _key[12];  sprintf(_key, "%d", n);
		return table->Get(machine, _key);
	}

	void GetDot(const char* key, gmVariable &res) {
		int i = atoi(key);
		if(i == 0 && key[0] != '0') { return; }
		gmVariable _res = iteratorGet(i);
		res = _res;
	}
	void SetDot(const char* key, gmVariable &var) {
		int i = atoi(key);
		if(i == 0 && key[0] != '0') { return; }
		set(i, var);
		return;
	}

	int addObject(ScriptObject* value, gmVariable* var = 0) {
		length += 1;

		char _key[12];
		sprintf(_key, "%d", length-1);
		BindMember(_key, &value, TYPE_OBJECT, var);

		return length-1;
	}
	int add(gmVariable& value) {
		length += 1;

		char _key[12];
		sprintf(_key, "%d", length-1);
//		BindMember(_key, &value, TYPE_OBJECT, var);
		table->Set(machine, _key, value);

		return length-1;
	}
	int gm_add(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		add(a_thread->Param(0));
		a_thread->Push(a_thread->Param(0));
		return GM_OK;
	}

//	ScriptObject* get(int n) {
	gmVariable get(int n) {
		if(n >= length) {
			gmVariable nullVar;
			nullVar.Nullify();
			return nullVar;
		}
		//return _array[n];
		return iteratorGet(n);
	}
	int gm_get(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(index, 0);
		a_thread->Push(iteratorGet(index));
		return GM_OK;
	}

//	bool set(int index, ScriptObject* value, gmVariable* var = 0) {
	bool set(int index, gmVariable& value) {
		if(index >= length) { Log(LOG_ERROR, "Index in Array.set() was set to %d, array is only %d in size.", index, length); return false; }

//		_array[index] = &value;

		char _key[12];
		sprintf(_key, "%d", index);
//		BindMember(_key, &value, TYPE_OBJECT, var);
		table->Set(machine, _key, value);

		return true;
	}
	int gm_set(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_INT_PARAM(index, 0);
//		GM_CHECK_USER_PARAM(ScriptObject*, GM_TYPE_OBJECT, object, 1);
//		set(index, object, &a_thread->Param(1));
		set(index, a_thread->Param(1));
		return ReturnThis(a_thread);
	}

	bool remove(int index) {
		if(index >= length) { Log(LOG_ERROR, "Index in Array.remove() can not be set beyond the current array length."); return false; }
		if(index < 0) { return false; }

		length--;

		char _key1[12];
		char _key2[12];
		int i = index;
		for(; i < length; i++) {
			sprintf(_key1, "%d", i);
			sprintf(_key2, "%d", i+1);
			gmVariable v = table->Get(machine, _key2);
			table->Set(machine, _key1, v);
		}

		gmVariable var = gmVariable();
		var.Nullify();

		sprintf(_key2, "%d", i);
		table->Set(machine, _key2, var);

		return true;
	}
	int gm_remove(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(index, 0);
		remove(index);
		return ReturnThis(a_thread);
	}

	int find(gmVariable& var) {
		for(int i = 0; i < length; i++) {
			gmVariable b = iteratorGet(i);
			if(var.m_type == b.m_type && var.m_value.m_int == b.m_value.m_int) { return i; }
		}
		return -1;
	}
	int gm_find(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		a_thread->PushInt(find(a_thread->Param(0)));
		return GM_OK;
	}

	int _length() {
		return length;
	}
	int gm_length(gmThread* a_thread) {
		a_thread->PushInt(_length());
		return GM_OK;
	}

	// TODO: Parse simple lists (strings/numbers)
	int parse(const char* raw) {
		int n = 0;
		char key[64];
		int n_key = 0;
		char value[64];
		int n_value = 0;
		ArrayObject* o, *a;

		bool valueDone = false;
		bool valueIsString = false;
		bool valueIsNum = false;
		bool valueIsArray = false;
		int n_array = 0;

		while(raw[n] != '\0') {
			char c = raw[n++];

			switch(c) {
			case '}':
				return n;
			case ':':
				// read value
				n_value = 0;
				valueDone = false;
				valueIsString = false;
				valueIsNum = true;
				valueIsArray = false;

				while(raw[n] != '\0' && !valueDone) {
					char c = raw[n++];
					switch(c) {
					case '[':
						// create array object
						n_array = 0;
						valueIsArray = true;
						a = new ArrayObject();
						a->id = key;
						BindMember(key, &a, TYPE_OBJECT, 0);
						break;
					case '{':
						// decent into new object
						valueIsString = false;
						valueIsNum = false;
						o = new ArrayObject();
						o->id = key;
						n += o->parse(&raw[n]);
						if(valueIsArray) {
							a->addObject(o);
//							a->add( gmVariable(machine->AllocUserObject(o, GM_TYPE_OBJECT)) );
						} else {
							BindMember(key, &o, TYPE_OBJECT, 0);
						}
						if(!valueIsArray) { valueDone = true; }
						break;
					case '"':
						// read string value
						valueIsString = true;
						valueIsNum = false;
						while(raw[n] != '\0') {
							c = raw[n++];
							if(c == '"') { break; }
							value[n_value++] = c;
							value[n_value] = '\0';
						}
						break;
					case ' ': // ignore whitespace
					case '\n':
					case '\r':
					case '\t':
						break;
					case ']':
						valueDone = true;
						break;
					case ',':
						if(!valueIsArray) { valueDone = true; }
						break;
//					case '\n':
					case '}':
						valueDone = true;
						break;
					default:
						value[n_value++] = c;
						value[n_value] = '\0';
						break;
					}
				}
				// store value
				if(valueIsString) {
					SetTableVar(key, value);
					if(raw[n-1] == '}') { n--; } // push back delimiter, if present
				}
				if(valueIsNum) {
					SetTableVar(key, (float)atof(value));
					if(raw[n-1] == '}') { n--; } // push back delimiter, if present
				}
				break;

			case '"':
				// read key
				n_key = 0;
				while(raw[n] != '\0' && n_key < 63) {
					char c = raw[n++];
					if(c == '"') { break; }
					key[n_key++] = c;
					key[n_key] = '\0';
				}
				break;
			}
		}
		return n;
	}
	int gm_parse(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(json, 0);
		parse(json);
		return ReturnThis(a_thread);
	}

public:
	int capacity;
	int length;
};


#endif
