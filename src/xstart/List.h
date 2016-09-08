#ifndef _LIST_H_
#define _LIST_H_

#include "ScriptObject.h"

class List : public ScriptObject {
public:

	List() : ScriptObject() {
		id = "List";
		help = "Double linked list for fast insertion and removal of objects or natives.";

		cont = cnew();
		index = 0;

		BindFunction("count", (SCRIPT_FUNCTION)&List::gm_count, "{int} count()", "Counts number of items in the list.");
		BindFunction("clear", (SCRIPT_FUNCTION)&List::gm_clear, "[this] clear()", "Clears the list by removing all items from the list.");
		BindFunction("first", (SCRIPT_FUNCTION)&List::gm_first, "[Object] first()", "Gets the first item from the list.");
		BindFunction("last", (SCRIPT_FUNCTION)&List::gm_last, "[Object] last()", "Gets the last item from the list.");
		BindFunction("fromIndex", (SCRIPT_FUNCTION)&List::gm_fromIndex, "[Object] fromIndex({int} index)", "Gets the item at the given index position.");
		BindFunction("pop", (SCRIPT_FUNCTION)&List::gm_popFirst, "[this] pop()", "Removes the first item from list.");
		BindFunction("popFirst", (SCRIPT_FUNCTION)&List::gm_popFirst, "[this] popFirst()", "Removes the first item from list.");
		BindFunction("popLast", (SCRIPT_FUNCTION)&List::gm_popLast, "[this] popLast()", "Removes the last item from list.");
		//BindFunction("randomPop", (SCRIPT_FUNCTION)&List::gm_randomPop, "[this] randomPop([Object] object)", "Removes the given item from list.");
		//BindFunction("remove", (SCRIPT_FUNCTION)&List::gm_randomPop, "[this] remove([Object] object)", "Removes the given item from list.");
		BindFunction("push", (SCRIPT_FUNCTION)&List::gm_pushBack, "[this] push([Object] object)", "Pushes the given object at the end of the list.");
		BindFunction("pushBack", (SCRIPT_FUNCTION)&List::gm_pushBack, "[this] pushBack([Object] object)", "Pushes the given object at the end of the list.");
		BindFunction("pushFront", (SCRIPT_FUNCTION)&List::gm_pushFront, "[this] pushFront([Object] object)", "Pushes the given object at the start of the list.");
	}

	~List() {
		while(ccount(cont))	{ cpop(cont); }
		cdel(cont);
	}

	gmVariable iteratorGet(int n) {
		if(n >= count()) {
			gmVariable v;
			v.Nullify();
			return v;
		}

		return getFromIndex(n);
	}

	gmVariable findObject(ScriptObject* o) {
		CONT_ITEM* ci = cfirst(cont);
		while(ci) {
			if(ci->pData == o) {
				char key[32];  sprintf(key, "_%d", ci->nCustomId);
				return this->table->Get(machine, key);
			}
			ci = cinext(ci);
		}

		gmVariable nullvar;
		nullvar.Nullify();
		return nullvar;
	}

	int count() { return cont->nItems; }
	int gm_count(gmThread* a_thread) {
		a_thread->PushInt(count());
		return GM_OK;
	}

	void clear() {
		while(CONT_ITEM* ci = cfirst(cont)) {
			char key[32];  sprintf(key, "_%d", ci->nCustomId);
			RemoveMember(key);
			cpop(cont);
		}
	}
	int gm_clear(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		clear();
		return ReturnThis(a_thread);
	}

	gmVariable first() {
		if(count() <= 0) { gmVariable nullvar; nullvar.Nullify(); return nullvar; }

		CONT_ITEM* ci = cfirst(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		return this->table->Get(machine, key);
	}
	int gm_first(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(first());
		return GM_OK;
	}

	gmVariable last() {
		if(count() <= 0) { gmVariable nullvar; nullvar.Nullify(); return nullvar; }

		CONT_ITEM* ci = clast(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		return this->table->Get(machine, key);
	}
	int gm_last(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(last());
		return GM_OK;
	}

	gmVariable getFromIndex(int i) {
		if(count() <= 0) { gmVariable nullvar; nullvar.Nullify(); return nullvar; }
		CONT_ITEM* ci = cfirst(cont);
		while(ci) {
			if(i--<=0) {
				char key[32];  sprintf(key, "_%d", ci->nCustomId);
				gmVariable var = this->table->Get(machine, key);
				return var;
			}
			ci = cinext(ci);
		}
		gmVariable nullvar; nullvar.Nullify(); return nullvar;
	}
	int gm_fromIndex(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(i, 0);
		a_thread->Push(getFromIndex(i));
		return GM_OK;
	}

	void popFirst() {
		if(count() <= 0) { return; }

		CONT_ITEM* ci = cfirst(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		RemoveMember(key);
		cpop_front(cont);
	}
	int gm_popFirst(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(first());
		popFirst();
		return ReturnThis(a_thread);
	}

	void popLast() {
		if(count() <= 0) { return; }

		CONT_ITEM* ci = clast(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		RemoveMember(key);
		cpop(cont);
	}
	int gm_popLast(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(last());
		popLast();
		return ReturnThis(a_thread);
	}

	gmVariable* randomPop(gmVariable& value) {
		CONT_ITEM* ci = cfirst(cont);
		while(ci) {
			gmVariable* valueIn = (gmVariable*)cidata(ci);
			if(gmVariable::Compare(value, *valueIn)) {
				char key[32];  sprintf(key, "_%d", ci->nCustomId);
				crndpop(cont, valueIn);
				gmVariable var = gmVariable();  var.Nullify();  table->Set(machine, ci->nCustomId, var);
				return valueIn;
			}
			ci = cinext(ci);
		}
		return 0;
	}
	int gm_randomPop(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		a_thread->Push(randomPop(a_thread->Param(0)));
		return GM_OK;
	}

	void pushBack(gmVariable& value) {
		cpush(cont, &value, index);
		char key[32];  sprintf(key, "_%d", index);
		table->Set(machine, key, value);
		index++;
	}
	int gm_pushBack(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		pushBack(a_thread->Param(0));
		return ReturnThis(a_thread);
	}

	void pushFront(gmVariable& value) {
		cpush_front(cont, &value, index);
		char key[32];  sprintf(key, "_%d", index);
		table->Set(machine, key, value);
		index++;
	}
	int gm_pushFront(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		pushFront(a_thread->Param(0));
		return ReturnThis(a_thread);
	}

public:
	CONT* cont;
	unsigned long index;
};





#if 0

class List : public ScriptObject {
public:

	List() : ScriptObject() {
		id = "List";
		help = "Double linked list for fast insertion and removal of script objects.";

		cont = cnew();
		index = 0;

		BindFunction("count", (SCRIPT_FUNCTION)&List::gm_count, "{int} count()", "Counts number of items in the list.");
		BindFunction("clear", (SCRIPT_FUNCTION)&List::gm_clear, "[this] clear()", "Clears the list by removing all items from the list.");
		BindFunction("first", (SCRIPT_FUNCTION)&List::gm_first, "[Object] first()", "Gets the first item from the list.");
		BindFunction("last", (SCRIPT_FUNCTION)&List::gm_last, "[Object] last()", "Gets the last item from the list.");
		BindFunction("fromIndex", (SCRIPT_FUNCTION)&List::gm_fromIndex, "[Object] fromIndex({int} index)", "Gets the item at the given index position.");
		BindFunction("index", (SCRIPT_FUNCTION)&List::gm_fromIndex, "", "<b>Deprecated, use fromIndex() instead.</b>");
		BindFunction("pop", (SCRIPT_FUNCTION)&List::gm_popFirst, "[this] pop()", "Removes the first item from list.");
		BindFunction("popFirst", (SCRIPT_FUNCTION)&List::gm_popFirst, "[this] popFirst()", "Removes the first item from list.");
		BindFunction("popLast", (SCRIPT_FUNCTION)&List::gm_popLast, "[this] popLast()", "Removes the last item from list.");
		BindFunction("randomPop", (SCRIPT_FUNCTION)&List::gm_randomPop, "[this] randomPop([Object] object)", "Removes the given item from list.");
		BindFunction("remove", (SCRIPT_FUNCTION)&List::gm_randomPop, "[this] remove([Object] object)", "Removes the given item from list.");
		BindFunction("push", (SCRIPT_FUNCTION)&List::gm_pushBack, "[this] push([Object] object)", "Pushes the given object at the end of the list.");
		BindFunction("pushBack", (SCRIPT_FUNCTION)&List::gm_pushBack, "[this] pushBack([Object] object)", "Pushes the given object at the end of the list.");
		BindFunction("pushFront", (SCRIPT_FUNCTION)&List::gm_pushFront, "[this] pushFront([Object] object)", "Pushes the given object at the start of the list.");
	}

	~List() {
		while(ccount(cont))	{ cpop(cont); }
		cdel(cont);
	}

	gmVariable iteratorGet(int n) {
		if(n >= count()) {
			gmVariable v;
			v.Nullify();
			return v;
		}

		return getFromIndex(n);
	}

	virtual std::string toString() {
		std::string out;

		char buffer[128];
		sprintf(buffer, "<%s::@%.8X/>\n", this->id.c_str(), this);
		out += buffer;

		CONT_ITEM* ci = cfirst(cont);
		while(ci) {
			ScriptObject* o = (ScriptObject*)cidata(ci);
			sprintf(buffer, "    <%s::@%.8X/>\n", o->id.c_str(), o);
			out += buffer;
			ci = cinext(ci);
		}

		sprintf(buffer, "</%s::@%.8X>\n", this->id.c_str(), this);
		out += buffer;

		return out;
	}

	gmVariable findObject(ScriptObject* o) {
		CONT_ITEM* ci = cfirst(cont);
		while(ci) {
			if(ci->pData == o) {
				char key[32];  sprintf(key, "_%d", ci->nCustomId);
				return this->table->Get(machine, key);
			}
			ci = cinext(ci);
		}

		gmVariable nullvar;
		nullvar.Nullify();
		return nullvar;
	}

	int count() { return cont->nItems; }
	int gm_count(gmThread* a_thread) {
		a_thread->PushInt(count());
		return GM_OK;
	}

	void clear() {
		while(CONT_ITEM* ci = cfirst(cont)) {
			char key[32];  sprintf(key, "_%d", ci->nCustomId);
			RemoveMember(key);
			cpop(cont);
		}
	}
	int gm_clear(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		clear();
		return ReturnThis(a_thread);
	}

	gmVariable first() {
		if(count() <= 0) { gmVariable nullvar; nullvar.Nullify(); return nullvar; }

		CONT_ITEM* ci = cfirst(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		return this->table->Get(machine, key);
	}
	int gm_first(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(first());
		return GM_OK;
	}

	gmVariable last() {
		if(count() <= 0) { gmVariable nullvar; nullvar.Nullify(); return nullvar; }

		CONT_ITEM* ci = clast(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		return this->table->Get(machine, key);
	}
	int gm_last(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(last());
		return GM_OK;
	}

	gmVariable getFromIndex(int i) {
		if(count() <= 0) { gmVariable nullvar; nullvar.Nullify(); return nullvar; }

		CONT_ITEM* ci = cfirst(cont);
		while(ci) {
			ScriptObject* o = (ScriptObject*)cidata(ci);
			if(i--<=0) {
				char key[32];  sprintf(key, "_%d", ci->nCustomId);
				gmVariable var = this->table->Get(machine, key);
#if _WIN32
				if(var.IsNull()) { __debugbreak(); }
#endif
				return var;
			}
			ci = cinext(ci);
		}

		gmVariable nullvar; nullvar.Nullify(); return nullvar;
	}
	int gm_fromIndex(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(i, 0);
		a_thread->Push(getFromIndex(i));
		return GM_OK;
	}

	void popFirst() {
		if(count() <= 0) { return; }

		CONT_ITEM* ci = cfirst(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		RemoveMember(key);
		cpop_front(cont);
	}
	int gm_popFirst(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(first());
		popFirst();
		return ReturnThis(a_thread);
	}

	void popLast() {
		if(count() <= 0) { return; }

		CONT_ITEM* ci = clast(cont);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		RemoveMember(key);
		cpop(cont);
	}
	int gm_popLast(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->Push(last());
		popLast();
		return ReturnThis(a_thread);
	}

	void randomPop(ScriptObject* o) {
		CONT_ITEM* ci = csearch(cont, o);
		char key[32];  sprintf(key, "_%d", ci->nCustomId);
		RemoveMember(key);
		crndpop(cont, o);
	}
	int gm_randomPop(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(ScriptObject*, GM_TYPE_OBJECT, obj, 0);
		randomPop(obj);
		return ReturnThis(a_thread);
	}

	void pushBack(ScriptObject* o, gmVariable* var = 0) {
		cpush(cont, o, index);
		char key[32];  sprintf(key, "_%d", index);
		BindMember(key, &o, TYPE_OBJECT, var);
		index++;
	}
	int gm_pushBack(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(ScriptObject*, GM_TYPE_OBJECT, obj, 0);
		pushBack(obj, &a_thread->Param(0));
		return ReturnThis(a_thread);
	}

	void pushFront(ScriptObject* o, gmVariable* var = 0) {
		cpush_front(cont, o, index);
		char key[32];  sprintf(key, "_%d", index);
		BindMember(key, &o, TYPE_OBJECT, var);
		index++;
	}
	int gm_pushFront(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(ScriptObject*, GM_TYPE_OBJECT, obj, 0);
		pushFront(obj, &a_thread->Param(0));
		return ReturnThis(a_thread);
	}

	/*void pushAfter(ScriptObject* o, ScriptObject* after, gmVariable* var = 0)
	{
		CONT_ITEM* ci = csearch(cont, after);
		cpush_after(cont, ci, o, index);
		char key[32];  sprintf(key, "_%d", index);
		BindMember(key, &o, TYPE_OBJECT, var);
		index++;
	}

	void pushBefore(ScriptObject* o, ScriptObject* before, gmVariable* var = 0)
	{
		CONT_ITEM* ci = csearch(cont, before);
		cpush_before(cont, ci, o, index);
		char key[32];  sprintf(key, "_%d", index);
		BindMember(key, &o, TYPE_OBJECT, var);
		index++;
	}*/

public:
	CONT* cont;
	unsigned long index;
};
#endif


#endif
