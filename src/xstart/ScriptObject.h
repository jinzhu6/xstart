#ifndef _SCRIPTOBJECT_H_
#define _SCRIPTOBJECT_H_

#include "corela.h"

#include <gm/gmThread.h>
#include <gm/gmMachine.h>
#include <gm/gmCall.h>

#include <string>
#include <map>
#include <vector>



// ************************************************************************
// GameMonkey machine and object type (external)
// ************************************************************************
extern gmMachine* machine;
extern gmType GM_TYPE_OBJECT;
extern char _libPath[1024];



// ************************************************************************
// GameMonkey callbacks for operators and function call forwarding to class.
// ************************************************************************
void GM_CDECL Script_Object_GetDot(gmThread* a_thread, gmVariable* a_operands);
void GM_CDECL Script_Object_SetDot(gmThread* a_thread, gmVariable* a_operands);
void GM_CDECL Script_Object_GetIndex(gmThread* a_thread, gmVariable* a_operands);
void GM_CDECL Script_Object_SetIndex(gmThread* a_thread, gmVariable* a_operands);
void GM_CDECL Script_Object_Destruct(gmMachine* a_machine, gmUserObject* a_object);
bool GM_CDECL Script_Object_Trace(gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workLeftToGo, int &a_workDone);
void GM_CDECL Script_Object_AsString(gmUserObject* a_object, char* a_buffer, int a_bufferLen);
void GM_CDECL Script_Object_Iterator(gmThread* a_thread, const gmUserObject* a_object, gmTypeIterator &a_it, gmVariable* a_key, gmVariable* a_value);
int GM_CDECL Script_Object_Function(gmThread* a_thread);



// ************************************************************************
// Script object container that is used to check for internal memory leaks
// ************************************************************************
class ScriptObject;
void Script_Objects_Add(ScriptObject* o);
void Script_Objects_Remove(ScriptObject* o);
void Script_Objects_Print();
bool Script_Object_Find(ScriptObject* o);



// ************************************************************************
// Function pointer for calling class methods directly within the script
// ************************************************************************
class ScriptObject;
typedef int (ScriptObject::*SCRIPT_FUNCTION)(gmThread*);


// ************************************************************************
// Helper to find file in library path
// ************************************************************************
extern std::string FindFile(const char* file);
#define _FILE(file) FindFile(file).c_str()


typedef enum SCRIPT_FUNCTION_PARAMS {
	PARAM_EMPTY = 0,
	PARAM_THREAD,
} SCRIPT_FUNCTION_PARAMS;

typedef enum SCRIPT_MEMBER_TYPE {
	TYPE_UNKNOWN = 0,
	TYPE_INT,      // do NOT use bool datatypes, bool may have different size in memory
	TYPE_FLOAT,    // single precision by default, but double should be possible if changed in GM too
	TYPE_STRING,   // UTF-8 character string
	TYPE_OBJECT,   // take care, may cause memory leaks if used wrong
	TYPE_TABLE,    // experimental, mostly untested; but should work
} SCRIPT_MEMBER_TYPE;

typedef struct SCRIPT_FUNCTION_DATA {
	SCRIPT_FUNCTION fn;
	SCRIPT_FUNCTION_PARAMS params;
	std::string classId;
	std::string declaration;
	std::string help;
} SCRIPT_FUNCTION_DATA;

typedef struct SCRIPT_MEMBER_DATA {
	void** member;
	SCRIPT_MEMBER_TYPE type;
	std::string classId;
	std::string declaration;
	std::string help;
} SCRIPT_MEMBER_DATA;



// ************************************************************************
// ScriptObject : Implementation for a super class for all script classes.
// ************************************************************************
class ScriptObject {
public:

	// ************************************************************************
	// ctor, dtor ... ctor required to be called by derived classes!
	// ************************************************************************
	ScriptObject() {
		id = "Object";
		help = "Object base class. Like an object in JavaScript it has a user table for its members.";
		ctor = "((optional) {string} id)";
		userObject = 0;
		isRenderable = false;
		isEventHandler = false;
		cppOwned = false;

		table = machine->AllocTableObject();
		machine->AddCPPOwnedGMObject(table);
		Script_Objects_Add(this);

		BindMember("id", &id, TYPE_STRING, 0, "{string} id", "Idenfication string of the object, initially set to class name but can be changed in script.");
		BindMember("_key", &_key, TYPE_STRING);
		BindMember("_table", &table, TYPE_TABLE);
		BindFunction("toString", (SCRIPT_FUNCTION)&ScriptObject::gm_toString, "{string} toString()", "Get string representation of object.");
		BindFunction("_help", (SCRIPT_FUNCTION)&ScriptObject::gm_help);
		BindFunction("_getTableString", (SCRIPT_FUNCTION)&ScriptObject::gm_getTableString);
	}

	virtual ~ScriptObject() {
		//Log(LOG_DEBUG, "GC-COLLECT: %s", this->id.c_str());
		// nullify script user object
		if(userObject) {
			userObject->m_user = 0;
		}
		machine->RemoveCPPOwnedGMObject(table);
		Script_Objects_Remove(this);
		machine->RemoveCPPOwnedGMObject(this->userObject);
	}


	// ************************************************************************
	// initialize class object with additional parameters in this method
	// ************************************************************************
	virtual int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) {
			if(a_thread->ParamType(0) == GM_STRING) {
				GM_CHECK_STRING_PARAM(newid, 0);
				id = newid;
			}
		}
		return GM_OK;
	}


	// ************************************************************************
	// toString method, for printing the object to the console
	// ************************************************************************
	virtual std::string toString() {
		char buffer[128];
		sprintf(buffer, "<%s::@%.8X/>\0", this->id.c_str(), this);
		return std::string(buffer);
	}
	virtual int gm_toString(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushNewString(toString().c_str());
		return GM_OK;
	}


	// ************************************************************************
	// _help method, for introspection and generating the documentation
	// ************************************************************************
	virtual std::string _help() {
		std::string out = "";
		out += this->id + '\t' + this->ctor + "\t" + this->help + "\t\n\n";

		std::vector<std::string>::iterator i;
		for(i = functionsOrder.begin(); i != functionsOrder.end(); i++) {
			SCRIPT_FUNCTION_DATA fd = functions[*i];
			out += *i + '\t' + fd.declaration + '\t' + fd.help + "\t\n";
		}

		out += "\n";
		for(i = membersOrder.begin(); i != membersOrder.end(); i++) {
			SCRIPT_MEMBER_DATA md = members[*i];

			std::string type;
			switch(md.type) {
			case TYPE_UNKNOWN:
				type="UNKNOWN";
				break;
			case TYPE_INT:
				type="{int}";
				break;
			case TYPE_FLOAT:
				type="{float}";
				break;
			case TYPE_STRING:
				type="{string}";
				break;
			case TYPE_OBJECT:
				type="{object}";
				break;
			case TYPE_TABLE:
				type="{table}";
				break;
			default:
				type="faulty";
				break;
			}

			if(md.declaration != "") {
				out +=  *i + '\t' + md.declaration + '\t' + md.help + "\t\n";
			} else {
				out += *i + '\t' + type + " " + *i + '\t' + md.help + "\t\n";
			}
		}


		out += "\n";
		return out;
	}
	int gm_help(gmThread* a_thread) {
		a_thread->PushNewString(_help().c_str());
		return GM_OK;
	}


	// ************************************************************************
	// internal operator overloading callbacks, for advanced class binding
	// ************************************************************************
	virtual void GetDot(const char* key, gmVariable &res) {
		return;
	}
	virtual void SetDot(const char* key, gmVariable &var) {
		return;
	}
//	virtual void GetIndex(const char* key, gmVariable& res) {  }
//	virtual void SetIndex(const char* key, gmVariable& var) {  }


	// ************************************************************************
	// iterator overloading, enables foreach() for the object
	// ************************************************************************
	virtual gmVariable iteratorGet(int index) {
		gmVariable v;
		v.Nullify();
		return v;
	}

	// ************************************************************************
	// BindFunction() - Bind class methods to script interface
	// ************************************************************************
	virtual void BindFunction(const char* key, SCRIPT_FUNCTION fn, std::string declaration = "", std::string help = "") {
		// add function pointer to "functions" list
		SCRIPT_FUNCTION_DATA data;
		data.fn = fn;
		data.params = PARAM_THREAD;
		data.declaration = declaration;
		data.help = help;
		functions[std::string(key)] = data;
		functionsOrder.push_back(std::string(key));

		// promote member function to script engine
		gmFunctionObject* fnObj = machine->AllocFunctionObject(Script_Object_Function);
		fnObj->m_cUserData = (void*)&functions[std::string(key)];
		table->Set(machine, key, gmVariable(fnObj));
	}

	// ************************************************************************
	// BindMember() - Bind class members to script interface
	// ************************************************************************
	virtual void BindMember(const char* key, void* pointer, SCRIPT_MEMBER_TYPE type, gmVariable* var = 0, std::string declaration = "", std::string help = "") {
		// add member to "members" list
		SCRIPT_MEMBER_DATA data;
		data.member = (void**)pointer;
		data.type = type;
		data.declaration = declaration;
		data.help = help;
		members[std::string(key)] = data;
		membersOrder.push_back(std::string(key));

		// only promote ScriptObject types to the script engine, this is mostly done for correct garbage collection
		// call-by-value types do not need this special care...
		if(type == TYPE_OBJECT) {
			ScriptObject* o = (ScriptObject*)*data.member;
			SetMemberObject(key, o, var);
		}
	}

	// ************************************************************************
	// SetMemberObject() - It is important to set object-types in the classes!
	// ************************************************************************
	bool SetMemberObject(const char* key, ScriptObject* value, gmVariable* var = 0) {
		if(members.count(std::string(key))) {
			SCRIPT_MEMBER_DATA data = members[std::string(key)];
			if(data.member) {
				memcpy((void*)data.member, &value, sizeof(ScriptObject*));

				if(value) {
					if(value->userObject) {
						table->Set(machine, key, gmVariable(value->userObject));
						return true;
					}
				}

				if(!var) {
					gmVariable var;
					if(value) {
						if(!value->userObject) {
							value->userObject = machine->AllocUserObject(value, GM_TYPE_OBJECT);
							// TODO: Why was that line here?!?
							//machine->AddCPPOwnedGMObject(value->userObject);  // was just "userObject" ?!?
						}
						var.SetUser(value->userObject);
					} else {
						var.Nullify();
					}
					table->Set(machine, key, var);
				} else {
					table->Set(machine, key, *var);
				}

			}
			return true;
		}
		return false;
	}

	// ************************************************************************
	// SetTableVar() - Sets simple data types to the table (BE CAREFUL).
	// ************************************************************************
	template <class T>
	void SetTableVar(const char* key, T value) {
		table->Set(machine, key, gmVariable(value));
	}
	void SetTableVar(const char* key, char* string) {
		gmVariable var;
		var.SetString(machine, string);
		table->Set(machine, key, var);
	}


	// ************************************************************************
	// GetGmVar()
	// ************************************************************************
	gmVariable GetGmVar(bool cppOwned = false) {
		if(!userObject) {
			userObject = machine->AllocUserObject(this, GM_TYPE_OBJECT);
			if(cppOwned) {
				machine->AddCPPOwnedGMObject(userObject);
				this->cppOwned = true;
			}
		}
		return gmVariable(userObject);
	}

	// ************************************************************************
	// ReturnThis()
	// ************************************************************************
	int ReturnThis(gmThread* a_thread, bool cppOwned = false) {
		if(!userObject) {
			userObject = machine->AllocUserObject(this, GM_TYPE_OBJECT);
			if(cppOwned) {
				machine->AddCPPOwnedGMObject(userObject);
				this->cppOwned = true;
			}
		}
		a_thread->Push(gmVariable(userObject));
		return GM_OK;
	}

	// ************************************************************************
	// ReturnNull()
	// ************************************************************************
	int ReturnNull(gmThread* a_thread) {
		gmVariable nullvar;
		nullvar.Nullify();
		a_thread->Push(nullvar);
		return GM_OK;
	}
	
	// ************************************************************************
	// IsCppOwned()
	// ************************************************************************
	bool IsCppOwned() {	
		return machine->IsCPPOwnedGMObject(userObject);
	}

	// ************************************************************************
	// SetCppOwned()
	// ************************************************************************
	void SetCppOwned(bool owned) {
		if (!userObject) { Log(LOG_WARNING, "Error in SetCppOwned() - userObject does not exist!"); }
		if (owned && !IsCppOwned()) { machine->AddCPPOwnedGMObject(userObject); cppOwned = true; }
		else if (IsCppOwned()) { machine->RemoveCPPOwnedGMObject(userObject); cppOwned = false;  }
	}

	// ************************************************************************
	// RemoveMember()
	// ************************************************************************
	void RemoveMember(const char* key) {
		std::map<std::string,SCRIPT_MEMBER_DATA>::iterator j;
		for(j = members.begin(); j != members.end(); j++) {
			std::string name = j->first;
			SCRIPT_MEMBER_DATA fd = j->second;
			if(strcmp(name.c_str(), key) == 0) {
				members.erase(j);
				break;
			}
		}

		gmVariable nullvar; nullvar.Nullify();
		table->Set(machine, key, nullvar);
	}

	// ************************************************************************
	// GetTableString()
	// ************************************************************************
	std::string GetTableString() {
		std::string out = "";
		char _key[256];
		const char* key;
		char value[1024];
		gmTableIterator iterator;
		gmTableNode* node = this->table->GetFirst(iterator);
		while(node) {
			key = node->m_key.AsString(machine, _key, 256-1);
			node->m_value.AsStringWithType(machine, value, 1024-1);
			out += key;  out += " := "; out += value; out += "\n";
			node = this->table->GetNext(iterator);
		}
		return out;
	}
	int gm_getTableString(gmThread* a_thread) {
		std::string table = GetTableString();
		a_thread->PushNewString(table.c_str());
		return GM_OK;
	}

public:

	// ************************************************************************
	// _setMember() - Set class member (here mostly from script interface)
	// ************************************************************************
	template <class T>
	bool _setMember(const char* key, T value) {
		if(members.count(std::string(key))) {
			SCRIPT_MEMBER_DATA data = members[std::string(key)];
			if(data.member) {
				memcpy((void*)data.member, &value, sizeof(T));
			}
			return true;
		}
		return false;
	}
	// Strings (std::string) must be handled slightly different, but it does the same thing
	bool _setMemberString(const char* key, std::string value) {
		if(members.count(std::string(key))) {
			SCRIPT_MEMBER_DATA data = members[std::string(key)];
			if(data.member) {
				std::string* pstr = (std::string*)data.member;
				*pstr = value;
			}
			return true;
		}
		return false;
	}

	// ************************************************************************
	// _getMemberType() - Get the type of a member
	// ************************************************************************
	SCRIPT_MEMBER_TYPE _getMemberType(const char* key) {
		if(members.count(std::string(key))) {
			return members[std::string(key)].type;
		}
		return TYPE_UNKNOWN;
	}

	// ************************************************************************
	// _hasMember() - Check if a member is bound to they given key.
	// ************************************************************************
	virtual bool _hasMember(const char* key) {
		if(members.count(std::string(key))) {
			return true;
		}
		return false;
	}

	// ************************************************************************
	// _getMember() - Get class member
	// ************************************************************************
	template <class T>
	T _getMember(const char* key) {
		if(members.count(std::string(key))) {
			SCRIPT_MEMBER_DATA data = members[std::string(key)];
			T* ret = (T*)data.member;
			return *ret;
		}
		return 0;
	}


public:
	bool isRenderable;    // to detect scene nodes
	bool isEventHandler;  // to detect event handler nodes
	bool cppOwned;
	std::string id;   // id is set to class name initially but can be changed in script
	std::string _key;
	std::string help;
	std::string ctor;
	gmTableObject* table;
	gmUserObject* userObject;
	std::map<std::string,SCRIPT_FUNCTION_DATA> functions;
	std::map<std::string,SCRIPT_MEMBER_DATA> members;
	std::vector<std::string> functionsOrder;
	std::vector<std::string> membersOrder;
};


#endif
