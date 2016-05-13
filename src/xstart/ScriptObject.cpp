#include "ScriptObject.h"
#include <corela.h>
#include <string>


void GM_CDECL Script_Object_GetDot(gmThread* a_thread, gmVariable* a_operands) {
	// get ScriptObject
	gmUserObject* user = (gmUserObject*)(a_operands[0].m_value.m_ref);
	ScriptObject* object = (ScriptObject*)user->m_user;
	if(!object) {
		a_operands[0].Nullify();
		return;
	}

	// get name of variable to get (key)
	char key[512];
	gmVariable* param = &a_operands[1];
	if(param->IsFloat() || param->IsInt()) { param->AsString(machine, key, 255); }
	else { strcpy(key, a_operands[1].GetCStringSafe()); }

	// check if the ScriptObject has bound a member to that key
	if(object->_hasMember(key)) {
		// check bound member type of key
		switch(object->_getMemberType(key)) {
		case TYPE_INT:
			a_operands[0].SetInt(object->_getMember<int>(key));
			break;
		case TYPE_FLOAT:
			a_operands[0].SetFloat(object->_getMember<float>(key));
			break;
		case TYPE_STRING:
			a_operands[0].SetString(machine, object->_getMember<std::string>(key).c_str());
			break;
		case TYPE_OBJECT:
			a_operands[0] = object->table->Get(a_operands[1]);
			break;
		case TYPE_TABLE:
			// TODO: shouldn't this be more like TYPE_OBJECT, since both are call-by-reference? probably needs fixing!
			a_operands[0].SetTable(object->_getMember<gmTableObject*>(key));
			break;
		default:
			machine->GetLog().LogEntry("WARNING: Trying to get the value of unknown member type '%s'.", key);
			a_operands[0].SetUser(machine, object->_getMember<ScriptObject*>(key), GM_TYPE_OBJECT);
			break;
		}
	} else {
		// use the normal object table for resolving the get-dot operator
//		gmVariable res = object->GetDot(a_operands[1].GetCStringSafe(), a_operands[0]);
		a_operands[0].Nullify();
		object->GetDot(key, a_operands[0]);
		if(a_operands[0].IsNull()) { a_operands[0] = object->table->Get(a_operands[1]); }
	}
}

void GM_CDECL Script_Object_SetDot(gmThread* a_thread, gmVariable* a_operands) {
	// get ScriptObject
	gmUserObject* user = (gmUserObject*)(a_operands[0].m_value.m_ref);
	ScriptObject* object = (ScriptObject*)user->m_user;
	if(!object) {
		a_operands[0].Nullify();
		return;
	}

	// get name of variable to get (key)
	char key[512];
	gmVariable* param = &a_operands[2];
	if(param->IsFloat() || param->IsInt()) { param->AsString(machine, key, 255); }
	else { strcpy(key, a_operands[2].GetCStringSafe()); }

	// call virtual method "SetDot" on object
	// TODO: Use return value of SetDot to cancel (false) or set the value (true).
	object->SetDot(key, a_operands[1]);

	// First tries to set the variable in native code
	ScriptObject* o;
	if(object->_hasMember(key)) {
		switch(object->_getMemberType(key)) {
		case TYPE_INT:
			object->_setMember<int>( key, a_operands[1].GetIntSafe() );
			break;
		case TYPE_FLOAT:
			object->_setMember<float>( key, a_operands[1].GetFloatSafe() );
			break;
		case TYPE_STRING:
			object->_setMemberString( key, std::string(a_operands[1].GetCStringSafe()) );
			break;
		case TYPE_OBJECT:
			o = (ScriptObject*)a_operands[1].GetUserSafe(GM_TYPE_OBJECT);
			if(o) {
				object->_setMember<ScriptObject*>( key, o );
				object->table->Set(a_thread->GetMachine(), a_operands[2], a_operands[1]);
			}
			break;
		default:
			object->_setMember<void*>( key, object->_getMember<void*>(key) );
			break;
		}
	} else {
		// This uses the table to set the member
		object->table->Set(a_thread->GetMachine(), a_operands[2], a_operands[1]);
	}

	//object->SetDot(a_operands[2].GetCStringSafe(), a_operands[1]);
}

void GM_CDECL Script_Object_GetIndex(gmThread* a_thread, gmVariable* a_operands) {
	/*gmVariable operands[3];
	operands[0] = a_operands[1];
	operands[1] = a_operands[0];
	operands[2] = a_operands[2];*/
	Script_Object_GetDot(a_thread, a_operands);

	/*	gmUserObject* user = (gmUserObject*)(a_operands[0].m_value.m_ref);

		ScriptObject *object = (ScriptObject*)user->m_user;
		if(!object) {
			a_operands[0].Nullify();
			return; }

		if(!object->GetIndex(a_operands[1].GetCStringSafe(), a_operands[0]))
			a_operands[0] = object->table->Get(a_operands[1]);*/
}

void GM_CDECL Script_Object_SetIndex(gmThread* a_thread, gmVariable* a_operands) {
	gmVariable operands[3];
	operands[0] = a_operands[0];
	operands[1] = a_operands[2];
	operands[2] = a_operands[1];
	Script_Object_SetDot(a_thread, operands);

	/*	gmUserObject* user = (gmUserObject*)(a_operands[0].m_value.m_ref);

		ScriptObject *object = (ScriptObject*)user->m_user;
		if(!object) {
			a_operands[0].Nullify();
			return; }

		if(!object->SetIndex(a_operands[1].GetCStringSafe(), a_operands[2]))
			object->table->Set(a_thread->GetMachine(), a_operands[1], a_operands[2]);*/
}

void GM_CDECL Script_Object_Destruct(gmMachine* a_machine, gmUserObject* a_object) {
	ScriptObject* object = (ScriptObject*)a_object->m_user;
	if(object) {
		/*if(machine->IsCPPOwnedGMObject(a_object)) {
			Log(LOG_DEBUG, "Attempted to destruct user-owned object by GC."); return;
		}*/
		a_object->m_user = 0;
		if(!Script_Object_Find(object)) {
			Log(LOG_ERROR, "GC failed, object not or no longer known: [%s] at [%x]", object->id.c_str(), object);
			return;
		}
		//Log(LOG_INFO, "Garbage collection of [%s] at [%x]", object->id.c_str(), object);
		delete object;
	}
}

bool GM_CDECL Script_Object_Trace(gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workLeftToGo, int &a_workDone) {
	a_workDone++;
	return true;
}

void GM_CDECL Script_Object_AsString(gmUserObject* a_object, char* a_buffer, int a_bufferLen) {
	ScriptObject* object = (ScriptObject*)a_object->m_user;
	if(!object) {
		sprintf(a_buffer, "%s", "<null/>");
		return;
	}

	strncpy(a_buffer, object->toString().c_str(), a_bufferLen-2);
	a_buffer[a_bufferLen-1] = '\0';
}

void GM_CDECL Script_Object_Iterator(gmThread* a_thread, const gmUserObject* a_object, gmTypeIterator &a_it, gmVariable* a_key, gmVariable* a_value) {
	ScriptObject* object = (ScriptObject*)a_object->m_user;

	if(object) {
		if(a_it < 0) {
			a_it = 0;
		}
		gmVariable var = object->iteratorGet(a_it++);
		if(!var.IsNull()) {
			ScriptObject* varObject = (ScriptObject*)var.GetUserSafe(GM_TYPE_OBJECT);
			if(varObject) {
				*a_key = gmVariable(machine->AllocStringObject(varObject->_key.c_str()));
			} else {
				*a_key = gmVariable((int)a_it);
			}

			*a_value = var;
		} else {
			*a_key = var;
			*a_value = var;
			a_it = GM_TYPE_ITR_NULL;
		}
	}
}

int GM_CDECL Script_Object_Function(gmThread* a_thread) {
	const gmVariable* var = a_thread->GetThis();
	ScriptObject* o = (ScriptObject*)((gmUserObject*)var->m_value.m_ref)->m_user;
	const gmFunctionObject* f = a_thread->GetFunctionObject();
	SCRIPT_FUNCTION_DATA* data = (SCRIPT_FUNCTION_DATA*)f->m_cUserData;

	if(data) {
		if(data->fn) {
			int r = (o->*data->fn)(a_thread);
			if(r == GM_EXCEPTION) {
				Log(LOG_USER, "EXCEPTION");
			}
			return r;
		}
	}

	return GM_OK;
}



#include <cont.h>
CONT* SCRIPT_OBJECTS = 0;

void Script_Objects_Add(ScriptObject* o) {
	if(!SCRIPT_OBJECTS) {
		SCRIPT_OBJECTS = cnew();
	}
	cpush(SCRIPT_OBJECTS, o);
//	Log(LOG_INFO, "Created [%s] at [%x]", o->id.c_str(), o);
}

void Script_Objects_Remove(ScriptObject* o) {
	if(!SCRIPT_OBJECTS) {
		return;
	}
	crndpop(SCRIPT_OBJECTS, o);
}

bool Script_Object_Find(ScriptObject* o) {
	return csearch(SCRIPT_OBJECTS, o) != 0;
}

void Script_Objects_Print() {
	if(!SCRIPT_OBJECTS) {
		return;
	}

	if(ccount(SCRIPT_OBJECTS) == 0) {
		//Log(LOG_INFO, "Good, no ScriptObjects remaining.");
		return;
	}

	Log(LOG_WARNING, "Remaining ScriptObject instances:");

	CONT_ITEM* ci = cfirst(SCRIPT_OBJECTS);
	while(ci) {
		ScriptObject* o = (ScriptObject*)cidata(ci);
		Log(LOG_WARNING, " * [%x] %s", o, o->id.c_str());
		ci = cinext(ci);
	}
}

