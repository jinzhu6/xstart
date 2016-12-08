#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "ScriptObject.h"
#include "Data.h"

#include "../ssockets/SimpleSocket.h"
#include "../ssockets/ActiveSocket.h"
#include "../ssockets/PassiveSocket.h"

#undef GetObject


class Socket : public ScriptObject {
public:
	Socket() : ScriptObject(), socket(0) {
		id = "Socket";
		help = "Socket class.";
		data = new Data();

		BindFunction("open", (SCRIPT_FUNCTION)&Socket::gm_open);
		BindFunction("openUDP", (SCRIPT_FUNCTION)&Socket::gm_open_udp);
		BindFunction("close", (SCRIPT_FUNCTION)&Socket::gm_close);
		BindFunction("send", (SCRIPT_FUNCTION)&Socket::gm_send);
		BindFunction("receive", (SCRIPT_FUNCTION)&Socket::gm_receive);
		BindFunction("setTimeout", (SCRIPT_FUNCTION)&Socket::gm_setTimeout, "[this] setTimeout({int} milliseconds)", "Sets the timeout (in milliseconds) to wait for data while receiving. A value of 0 sets the socket to non-blocking.");
		BindMember("data", &data, TYPE_OBJECT, 0, "[Data] data", "Received data buffer. You may use <i>toString()</i> on this to get a string representation.");
	}
	~Socket() {
		if(socket) delete socket;
	}

	void setTimeout(int t) {
		if(!socket) { Log(LOG_ERROR, "Failed to set timeout on non-opened socket! Please open the socket first!"); return; }
		if(t > 0) socket->SetBlocking();
		else socket->SetNonblocking();
		socket->SetConnectTimeout(t,0);
		socket->SetReceiveTimeout(t,0);
		socket->SetSendTimeout(t,0);
	}
	int gm_setTimeout(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(timeout, 0);
		setTimeout(timeout);
		return ReturnThis(a_thread);
	}

	int open(const char* addr, int port, int udp) {
		if(socket) {
			close();
			delete socket;
			socket = 0;
		}

		if(udp) socket = new CActiveSocket(CSimpleSocket::SocketTypeUdp);
		else socket = new CActiveSocket();

		socket->Initialize();
		int res = socket->Open((const uint8*)addr, port);
		return res;
	}
	int gm_open(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(addr, 0);
		GM_CHECK_INT_PARAM(port, 1);
		a_thread->PushInt(open(addr, port, 0));
		return GM_OK;
	}
	int gm_open_udp(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(addr, 0);
		GM_CHECK_INT_PARAM(port, 1);
		a_thread->PushInt(open(addr, port, 1));
		return GM_OK;
	}

	void close() {
		if(!socket) { return; }
		socket->Close();
		delete socket;
		socket = 0;
	}
	int gm_close(gmThread* a_thread) {
		close();
		return GM_OK;
	}

	int send(const unsigned char* bytes, int len) {
		if(!socket) { Log(LOG_ERROR, "Attempt to send to non-opened socket!"); return 0; }
		if(!data) { Log(LOG_ERROR, "Attempt to send null data to socket!"); return 0; }
		return socket->Send((const uint8*)bytes, (size_t)len);
	}
	int gm_send(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		if(a_thread->ParamType(0) == TYPE_OBJECT) {
			GM_CHECK_USER_PARAM(Data*, TYPE_OBJECT, data, 0);
			a_thread->PushInt(send(data->data, data->size));
		}
		else if(a_thread->ParamType(0) == TYPE_STRING) {
			GM_CHECK_STRING_PARAM(str, 0);
			a_thread->PushInt(send((unsigned char*)str, strlen(str)));
		}
		else {
			Log(LOG_ERROR, "Attempt to send unsupported data type to socket!");
			return ReturnNull(a_thread);
		}
		return GM_OK;
	}

	int receive(int max) {
		if(!socket) { Log(LOG_ERROR, "Attempt to receive from non-opened socket!");  return 0; }
		int res = socket->Receive(max);
		if(res > 0) {
			data->resize(res);
			data->insert(socket->GetData(), res, 0);
		}
		return res;
	}
	int gm_receive(gmThread* a_thread) {
		int max; a_thread->ParamInt(0, max, 4096);
		a_thread->PushInt(receive(max));
		return GM_OK;
	}

public:
	CActiveSocket* socket;
	Data* data;
};


class SocketListener : public ScriptObject {
public:
	SocketListener() : ScriptObject(), socket(0) {
		id = "Listener";
		help = "Listener class.";
		data = new Data();
		
		BindFunction("bind", (SCRIPT_FUNCTION)&SocketListener::gm_bind);
		BindFunction("bindUDP", (SCRIPT_FUNCTION)&SocketListener::gm_bind_udp);
		BindFunction("listen", (SCRIPT_FUNCTION)&SocketListener::gm_listen);
		BindFunction("listenUDP", (SCRIPT_FUNCTION)&SocketListener::gm_listen_udp);
		BindFunction("accept", (SCRIPT_FUNCTION)&SocketListener::gm_accept);
		BindFunction("receive", (SCRIPT_FUNCTION)&SocketListener::gm_receive);
		BindFunction("send", (SCRIPT_FUNCTION)&SocketListener::gm_send);
		BindFunction("close", (SCRIPT_FUNCTION)&SocketListener::gm_close);
		BindFunction("setTimeout", (SCRIPT_FUNCTION)&SocketListener::gm_setTimeout, "[this] setTimeout({int} seconds)", "Sets the timeout (in seconds) to wait for data while receiving. A value of 0 sets the socket to non-blocking.");
		BindMember("data", &data, TYPE_OBJECT, 0, "[Data] data", "Received data buffer. You may use <i>toString()</i> on this to get a string representation.");
		BindMember("client", &client, TYPE_STRING, 0, "{string} client", "String of IP address of the last accepted client.");
	}
	~SocketListener() {
		if(socket) delete socket;
	}

	void close() {
		if(!socket) { return; }
		socket->Close();
		delete socket;
		socket = 0;
	}
	int gm_close(gmThread* a_thread) {
		close();
		return GM_OK;
	}

	void setTimeout(int t) {
		if(!socket) { Log(LOG_ERROR, "Failed to set timeout on non-opened socket! Please open the socket first!"); return;	}
		if(t > 0) socket->SetBlocking();
		else socket->SetNonblocking();
		socket->SetConnectTimeout(t,0);
		socket->SetReceiveTimeout(t,0);
		socket->SetSendTimeout(t,0);
	}
	int gm_setTimeout(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(timeout, 0);
		setTimeout(timeout);
		return ReturnThis(a_thread);
	}

	int bind(const char* addr, int port, int udp) {
		close();

		if(udp) socket = new CPassiveSocket(CSimpleSocket::SocketTypeUdp);
		else socket = new CPassiveSocket();

		socket->Initialize();
		bool res = socket->BindMulticast(NULL, (const uint8*)addr, port);

		return res;
	}
	int gm_bind(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(addr, 0);
		GM_CHECK_INT_PARAM(port, 1);
		a_thread->PushInt(bind(addr, port, 0));
		return GM_OK;
	}
	int gm_bind_udp(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(addr, 0);
		GM_CHECK_INT_PARAM(port, 1);
		a_thread->PushInt(bind(addr, port, 1));
		return GM_OK;
	}

	int listen(const char* addr, int port, int udp) {
		close();

		if(udp) socket = new CPassiveSocket(CSimpleSocket::SocketTypeUdp);
		else socket = new CPassiveSocket();

		socket->Initialize();
		int res = socket->Listen((const uint8*)addr, port);
		socket->SetMulticast(false);

		return res;
	}
	int gm_listen(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(addr, 0);
		GM_CHECK_INT_PARAM(port, 1);
		a_thread->PushInt(this->listen(addr, port, 0));
		return GM_OK;
	}
	int gm_listen_udp(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(addr, 0);
		GM_CHECK_INT_PARAM(port, 1);
		if (this->listen(addr, port, 1)) { return ReturnThis(a_thread); }
		else { a_thread->PushInt(0); return GM_OK; }
	}

	Socket* accept() {
		CActiveSocket* clientSocket = socket->Accept();
		if(clientSocket) {
			Socket* clientCon = new Socket();
			clientCon->socket = clientSocket;
			return clientCon;
		}
		return 0;
	}
	int gm_accept(gmThread* a_thread) {
		Socket* clientCon = accept();
		if(clientCon) return clientCon->ReturnThis(a_thread, false);
		else return ReturnNull(a_thread);
	}

	int send(const unsigned char* bytes, int len) {
		if(!socket) { Log(LOG_ERROR, "Attempt to send to non-opened socket!"); return 0; }
		if(!data) { Log(LOG_ERROR, "Attempt to send null data to socket!"); return 0; }
		int result = socket->Send((const uint8*)bytes, (size_t)len);
		if (result <= 0) { Log(LOG_ERROR, "Error while sending data '%s' of length %d (Code:%d)!", bytes, len, socket->GetSocketError()); }
		return result;
	}
	int gm_send(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		if(a_thread->ParamType(0) == TYPE_OBJECT) {
			GM_CHECK_USER_PARAM(Data*, TYPE_OBJECT, data, 0);
			a_thread->PushInt(send(data->data, data->size));
		}
		else if(a_thread->ParamType(0) == TYPE_STRING) {
			GM_CHECK_STRING_PARAM(str, 0);
			a_thread->PushInt(send((unsigned char*)str, strlen(str)));
		}
		else {
			Log(LOG_ERROR, "Attempt to send unsupported data type to socket!");
			return ReturnNull(a_thread);
		}
		return GM_OK;
	}

	int receive(int max) {
 		if(!socket) { Log(LOG_ERROR, "Attempt to receive from non-opened socket!");  return 0; }
		data->resize(max);
		int res = socket->Receive(data->size);
		if(res == -1) {
			CSimpleSocket::CSocketError err = socket->GetSocketError();
			if (err == CSimpleSocket::SocketBufferToSmall) {
				Log(LOG_ERROR, "The buffer on the socket listener was too small, try increasing the max.size!");
				return 0;
			}
			if(err != CSimpleSocket::SocketEwouldblock) {
				Log(LOG_DEBUG, "There was an error on the socket while receiving (%d)!", err);
				return 0;
			}
		}
		if(res > 0) {
			data->resize(res);
			data->insert(socket->GetData(), res, 0);
			this->client = std::string((char*)socket->GetClientAddr());
		}
		return res > 0 ? res : 0;
	}
	int gm_receive(gmThread* a_thread) {
		int max; a_thread->ParamInt(0, max, 4096);
		a_thread->PushInt(receive(max));
		return GM_OK;
	}


public:
	CPassiveSocket* socket;
	Data* data;
	std::string client;
};


#endif
