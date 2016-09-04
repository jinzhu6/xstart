#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <gm/gmThread.h>
#include <gm/gmMachine.h>
#include <gm/gmCall.h>

#define NAME "xstart"
#define VERSION "0.3.0"


extern gmType GM_TYPE_OBJECT;  // script object types (will be set by GM)

extern gmMachine* machine;
volatile extern bool exitState;
extern int argc;
extern char **argv;

void MachineCreate(bool debug);
int MachineDestroy();
bool MachineRun(FILE* in);
bool MachineRunFile(const char* file);
void MachineRegisterFunction(const char* name, gmCFunction fn, std::string declaration = "", std::string help = "");
void MachineRegisterClass(const char* typeName, gmCFunction ctor);
std::string MachineGlobalHelp();
int MachineCreateClassInstance(gmThread* a_thread, const char* typeName);
std::string MachineGetClassList();
void RegisterCommonAPI();
char* PreprocessScript(const char* file);
void SetLibraryPath(const char* path);
std::string FindFile(const char* file);


/******************************************************************************
* _MachineNewClass
*******************************************************************************/
template<class T> int GM_CDECL _MachineNewClass(gmThread* a_thread) {
	int r = GM_OK;

	T* o = new T();

	o->userObject = machine->AllocUserObject(o, GM_TYPE_OBJECT);
	a_thread->PushUser(o->userObject);

	r = o->Initialize(a_thread);

	return r;
}


/******************************************************************************
* MachineRegisterClass
*******************************************************************************/
template<class T> void MachineRegisterClass(const char* typeName) {
	Log(LOG_NOBREAK_DEBUG, "%s ", typeName);
	machine->RegisterLibraryFunction(typeName, _MachineNewClass<T>);
	MachineRegisterClass(typeName, _MachineNewClass<T>);
//	Log(LOG_NOBREAK, "[ok]");
}


#endif
