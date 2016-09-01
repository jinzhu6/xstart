#ifndef _LOGO_H_
#define _LOGO_H_

#include "ScriptObject.h"
#include <libnodave/nodave.h>
#include <libnodave/openSocket.h>

class Logo : public ScriptObject {
public:

	Logo() : ScriptObject() {
		id = "Logo";
		help = "NOT WORKING YET, NEEDS FIXING! Data transfer to Siemens Logo via ethernet.";

		connected = 0;
		di = 0;
		dc = 0;
		mpi = 0;
		rack = 1;
		slot = 0;

		BindFunction("setDebug", (SCRIPT_FUNCTION)&Logo::gm_setDebug);
		BindFunction("connect", (SCRIPT_FUNCTION)&Logo::gm_connect, "[this] connect({string} ip)", "(Re-)connects to a Siemens Logo device on the given IP.");
		BindFunction("close", (SCRIPT_FUNCTION)&Logo::gm_close, "close()", "Closes the active connection.");
		BindFunction("readByte", (SCRIPT_FUNCTION)&Logo::gm_readByte);
		BindFunction("readWord", (SCRIPT_FUNCTION)&Logo::gm_readWord);
		BindFunction("readDword", (SCRIPT_FUNCTION)&Logo::gm_readDword);
		BindFunction("readChar", (SCRIPT_FUNCTION)&Logo::gm_readChar);
		BindFunction("readShort", (SCRIPT_FUNCTION)&Logo::gm_readShort);
		BindFunction("readInt", (SCRIPT_FUNCTION)&Logo::gm_readInt);
		BindFunction("writeByte", (SCRIPT_FUNCTION)&Logo::gm_writeByte);
		BindFunction("writeWord", (SCRIPT_FUNCTION)&Logo::gm_writeWord);
		BindFunction("writeDword", (SCRIPT_FUNCTION)&Logo::gm_writeDword);
		BindFunction("writeChar", (SCRIPT_FUNCTION)&Logo::gm_writeChar);
		BindFunction("writeShort", (SCRIPT_FUNCTION)&Logo::gm_writeShort);
		BindFunction("writeInt", (SCRIPT_FUNCTION)&Logo::gm_writeInt);
		BindMember("connected", &connected, TYPE_INT, 0, "{int} connected", "Nonzero if a connection was made.");
		BindMember("mpi", &mpi, TYPE_INT);
		BindMember("rack", &rack, TYPE_INT);
		BindMember("slot", &slot, TYPE_INT);
	}

	~Logo() {
		close();
	}

	virtual int Initialize(gmThread* a_thread) {
		gmVariable varNull;
		varNull.Nullify();
		if(a_thread->GetNumParams() >= 1) {
			gmVariable var = a_thread->Param(0, varNull);
			if(!var.IsNull()) {
				const char* ip = var.GetCStringSafe();
				connect(ip);
			}
		}
		return GM_OK;
	}

	int gm_setDebug(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(_debug, 0);
		daveSetDebug(_debug);
		return ReturnThis(a_thread);
	}

	bool connect(const char* ip) {
		close();
		Log(LOG_DEBUG, "Connecting to Siemens Logo on IP '%s' (mpi:%d, rack:%d, slot:%d) ...", ip, mpi, rack, slot);
		fds.rfd = openSocket(102, ip);
		fds.wfd = fds.rfd;
		if (fds.rfd <= 0) { Log(LOG_ERROR, "Siemens Logo on IP '%s' could not be reached! Check IP!", ip); close(); return false; }
		di = daveNewInterface(fds, "IF1", 0, daveProtoISOTCP, daveSpeed187k);
		if(di == 0) { Log(LOG_ERROR, "Creating interface for Siemens Logo on IP '%s' failed!", ip); close(); return false; }
//		int res = daveInitAdapter(di);
//		if(res != 0) { Log(LOG_ERROR, "Initializing adapter for Siemens Logo on IP '%s' failed: %s", ip, daveStrerror(res)); close(); return false; }
		dc = daveNewConnection(di, mpi, rack, slot);
		if(dc == 0) { Log(LOG_ERROR, "Creating connection to Siemens Logo on IP '%s' failed!", ip); close(); return false; }
		daveSetTimeout(di, 10000000);
		int res = daveConnectPLC(dc);
		if(res != 0) { Log(LOG_ERROR, "Connection to Siemens Logo PLC on IP '%s' failed: %s (%d)", ip, daveStrerror(res), res); close(); return true; }
		connected = 1;
		return true;
	}
	int gm_connect(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(ip, 0);
		if(connect(ip)) { return ReturnThis(a_thread); }
		else return ReturnNull(a_thread);
	}

	void close() {
		if(dc) {
			Log(LOG_DEBUG, "Closing connection to Siemens Logo ...");
			if(connected) { daveDisconnectPLC(dc); }
			if(fds.rfd) { closeSocket(fds.rfd); fds.rfd = 0; }
			if(di) { daveDisconnectAdapter(di); /*free(di);*/ }
			if(dc) { free(dc); }
			connected = 0;
			dc = 0;
			di=0;
		}
	}
	int gm_close(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		close();
		return GM_OK;
	}

	bool readBytes(int addr, int len, void* buffer) {
		if(!dc) return 0;
		int res = daveReadBytes(dc, daveDB, 0, addr, len, buffer);
		if (res != 0) { Log(LOG_ERROR, "Error while reading data from Siemens Logo: %s", daveStrerror(res)); return false; }
		return true;
	}

	unsigned char readByte(int addr) {
		if(readBytes(addr, 1, 0)) return daveGetU8(dc);
		return -1;
	}

	unsigned short readWord(int addr) {
		if(readBytes(addr, 2, 0)) return daveGetU16(dc);
		return -1;
	}

	unsigned int readDword(int addr) {
		if(readBytes(addr, 4, 0)) return daveGetU32(dc);
		return -1;
	}

	signed char readChar(int addr) {
		if(readBytes(addr, 1, 0)) return daveGetS8(dc);
		return -1;
	}

	signed short readShort(int addr) {
		if(readBytes(addr, 2, 0)) return daveGetS16(dc);
		return -1;
	}

	signed int readInt(int addr) {
		if(readBytes(addr, 4, 0)) return daveGetS32(dc);
		return -1;
	}

	int writeBytes(int addr, int len, void* buffer) {
		if(!dc) return 0;
		return daveWriteBytes(dc, daveDB, 0, addr, len, buffer);
	}
	
	void writeByte(int addr, unsigned char val) {
		writeBytes(addr, 1, &val);
	}

	void writeWord(int addr, unsigned short val) {
		val = daveSwapIed_16(val);
		writeBytes(addr, 2, &val);
	}

	void writeDword(int addr, unsigned int val) {
		val = daveSwapIed_32(val);
		writeBytes(addr, 4, &val);
	}

	void writeChar(int addr, signed char val) {
		writeBytes(addr, 1, &val);
	}

	void writeShort(int addr, signed short val) {
		val = daveSwapIed_16(val);
		writeBytes(addr, 2, &val);
	}

	void writeInt(int addr, signed int val) {
		val = daveSwapIed_32(val);
		writeBytes(addr, 4, &val);
	}

	int gm_readByte(gmThread* a_thread)   { GM_CHECK_INT_PARAM(addr,0); a_thread->PushInt(readByte(addr)); return GM_OK; }
	int gm_readWord(gmThread* a_thread)   { GM_CHECK_INT_PARAM(addr,0); a_thread->PushInt(readWord(addr)); return GM_OK; }
	int gm_readDword(gmThread* a_thread)  { GM_CHECK_INT_PARAM(addr,0); a_thread->PushInt(readDword(addr)); return GM_OK; }
	int gm_readChar(gmThread* a_thread)   { GM_CHECK_INT_PARAM(addr,0); a_thread->PushInt(readChar(addr)); return GM_OK; }
	int gm_readShort(gmThread* a_thread)  { GM_CHECK_INT_PARAM(addr,0); a_thread->PushInt(readShort(addr)); return GM_OK; }
	int gm_readInt(gmThread* a_thread)    { GM_CHECK_INT_PARAM(addr,0); a_thread->PushInt(readInt(addr)); return GM_OK; }
	int gm_writeByte(gmThread* a_thread)  { GM_CHECK_INT_PARAM(addr,0); GM_CHECK_INT_PARAM(val,1); writeByte(addr,val); return GM_OK; }
	int gm_writeWord(gmThread* a_thread)  { GM_CHECK_INT_PARAM(addr,0); GM_CHECK_INT_PARAM(val,1); writeWord(addr,val); return GM_OK; }
	int gm_writeDword(gmThread* a_thread) { GM_CHECK_INT_PARAM(addr,0); GM_CHECK_INT_PARAM(val,1); writeDword(addr,val); return GM_OK; }
	int gm_writeChar(gmThread* a_thread)  { GM_CHECK_INT_PARAM(addr,0); GM_CHECK_INT_PARAM(val,1); writeChar(addr,val); return GM_OK; }
	int gm_writeShort(gmThread* a_thread) { GM_CHECK_INT_PARAM(addr,0); GM_CHECK_INT_PARAM(val,1); writeShort(addr,val); return GM_OK; }
	int gm_writeInt(gmThread* a_thread)   { GM_CHECK_INT_PARAM(addr,0); GM_CHECK_INT_PARAM(val,1); writeInt(addr,val); return GM_OK; }

public:
	int connected;
	int mpi;
	int rack;
	int slot;
	_daveOSserialType fds;
	_daveInterface* di;
	_daveConnection* dc;
};

#endif
