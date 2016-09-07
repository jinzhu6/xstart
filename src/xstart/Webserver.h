#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include "ScriptObject.h"
#include "Data.h"
#include <mongoose.h>

class HttpServer;

static void HttpEventHandler(struct mg_connection *nc, int ev, void *ev_data);

class HttpServer : public ScriptObject {
public:
	HttpServer() {
		id = "HttpServer";
		help = "Spawns a HTTP server. Note that bindung to port 80 may need root privileges. It is neccessary to call the poll() method since there is no background thread handling the requests.";
		ctor = "({string} port, (optional) {string} docroot, (optional) {bool} dirListing)";
		
		nc = 0;
		
		// TODO: open() and close() for HttpServer
		BindFunction("poll", (SCRIPT_FUNCTION)&HttpServer::gm_poll, "[this] poll({float} timeoutSeconds)", "Polls for incoming http requests.");
		BindFunction("onRequest", (SCRIPT_FUNCTION)&HttpServer::gm_onRequest, "onRequest({string} url, {string} get, {string} post, {string} auth)", "Request handler/callback, the default request handler sends a 404 http error code to all request. Override this handler to your own logic.");
		BindFunction("sendFile", (SCRIPT_FUNCTION)&HttpServer::gm_sendFile, "sendFile((optional){string} file)", "When called inside the request-handler, it send a file to the client. If the file parameter is omitted, it sends the requested file from the URI.");
		BindFunction("send", (SCRIPT_FUNCTION)&HttpServer::gm_send, "[this] send({string} data)", "When called inside the request-handler, it sends a string back to the client.");
		//BindFunction("sendData", (SCRIPT_FUNCTION)&HttpServer::gm_sendData, "[this] sendData([Data] data)", "When called inside the request-handler, it sends the content of a Data object back to the client.");
		BindFunction("send404", (SCRIPT_FUNCTION)&HttpServer::gm_send404, "[this] send404()", "When called inside the request-handler, it sends a a 404 error back to the client.");
		BindFunction("sendAuthRequest", (SCRIPT_FUNCTION)&HttpServer::gm_sendAuthRequest, "[this] sendAuthRequest({string} message)", "When called inside the request-handler, it sends a authorizsation request ('401 Access Denied') back to the client.");
	}

	~HttpServer() {
		if(nc) mg_mgr_free(&mgr);
	}

	int Initialize(gmThread* a_thread) {
		const char* port = a_thread->ParamString(0, "80");
		const char* docroot = a_thread->ParamString(1, ".");
		bool dirListing = a_thread->ParamInt(2, 0);
		if(a_thread->GetNumParams() > 0) init(port, docroot, dirListing);
		return GM_OK;
	}


	void init(const char* port, const char* docroot, bool dirListing) {
		mg_mgr_init(&mgr, this);
		nc = mg_bind(&mgr, port, HttpEventHandler);
		if (!nc) { Log(LOG_FATAL, "Binding HttpServer to port '%s' has failed!", port); }
		mg_set_protocol_http_websocket(nc);

		memset(&s_http_server_opts, 0, sizeof(mg_serve_http_opts));
		s_http_server_opts.document_root = docroot;
		s_http_server_opts.enable_directory_listing = dirListing ? "yes" : "no";
	}

	void poll(int msWait) {
		mg_mgr_poll(&mgr, msWait);
	}
	int gm_poll(gmThread* a_thread) {
		float wait;
		a_thread->ParamFloat(0, wait, 0.0);
		poll((int)(wait*1000.0));
		return ReturnThis(a_thread);
	}

	void sendFile(const char* file="") {
		mg_serve_http(nc_req, (http_message*)ev_data, s_http_server_opts, file);
	}
	int gm_sendFile(gmThread* a_thread) {
		const char* file;
		a_thread->ParamString(0, file, "");
		sendFile(file);
		return ReturnThis(a_thread);
	}

	void send(const char* data) {
		mg_printf(nc_req, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html; charset=utf-8\r\n\r\n%s", strlen(data), data);
	}
	int gm_send(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(data, 0);
		send(data);
		return ReturnThis(a_thread);
	}

	void send404() {
		mg_printf(nc_req, "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nnot found");
	}
	int gm_send404(gmThread* a_thread) {
		send404();
		return ReturnThis(a_thread);
	}

	void sendAuthRequest(const char* message) {
		mg_printf(nc_req, "HTTP/1.1 401 Access Denied\r\nWWW-Authenticate: Basic realm=\"%s\"\r\nContent-Length: 21\r\n\r\nauthentication failed", message);
	}
	int gm_sendAuthRequest(gmThread* a_thread) {
		const char* message = a_thread->ParamString(0, "Please enter your credentials:");
		sendAuthRequest(message);
		return ReturnThis(a_thread);
	}

	int gm_onRequest(gmThread* a_thread) {
		send404();
		return GM_OK;
	}

public:
	mg_mgr mgr;
	mg_connection *nc;
	mg_serve_http_opts s_http_server_opts;
	mg_connection *nc_req;
	void *ev_data;
};

static void HttpEventHandler(struct mg_connection *nc, int ev, void *ev_data) {
	HttpServer* server = (HttpServer*)nc->mgr->user_data;
	server->ev_data = ev_data;
	server->nc_req = nc;
	mbuf *io = &nc->recv_mbuf;

	switch(ev) {
	case MG_EV_HTTP_REQUEST: {
		http_message* msg = (http_message*)ev_data;

		gmStringObject* authString;
		char auth_dst[1024];
		mg_str* _auth = mg_get_http_header(msg, "Authorization");
		if(_auth) {
			mg_base64_decode((const unsigned char*)&_auth->p[6], _auth->len-6, auth_dst);
			std::string auth(&auth_dst[0], strlen(&auth_dst[0]));
			authString = machine->AllocStringObject(auth.c_str());
		}

		std::string uri(msg->uri.p, msg->uri.len);
		std::string query(msg->query_string.p, msg->query_string.len);
		std::string post(msg->body.p, msg->body.len);

		gmStringObject* uriString = machine->AllocStringObject(uri.c_str());
		gmStringObject* queryString = machine->AllocStringObject(query.c_str());
		gmStringObject* postString = machine->AllocStringObject(post.c_str());

		int retval = 0;
		gmVariable fnVar;
		gmFunctionObject* fnObj;
		gmCall call;

		std::replace(uri.begin(), uri.end(), '/', '_');
		fnVar = server->table->Get(machine, (std::string("")+uri).c_str());
		if(!fnVar.IsNull()) {
			if(fnVar.m_type == GM_FUNCTION) {
				fnObj = fnVar.GetFunctionObjectSafe();
				if(call.BeginTableFunction(machine, (std::string("")+uri).c_str(), server->table, gmVariable(server->userObject), false)) {
					call.AddParamString(uriString);
					call.AddParamString(queryString);
					call.AddParamString(postString);
					if(_auth) { call.AddParamString(authString); }
					if(call.DidReturnVariable()) {
						call.GetReturnedInt(retval);
					}
					call.End();
				}
			}
		} else {
			fnVar = server->table->Get(machine, "onRequest");
			if(!fnVar.IsNull()) {
				if(fnVar.m_type == GM_FUNCTION) {
					fnObj = fnVar.GetFunctionObjectSafe();
					if(call.BeginTableFunction(machine, "onRequest", server->table, gmVariable(server->userObject), false)) {
						call.AddParamString(uriString);
						call.AddParamString(queryString);
						call.AddParamString(postString);
						if(_auth) { call.AddParamString(authString); }
						if(call.DidReturnVariable()) {
							call.GetReturnedInt(retval);
						}
						call.End();
					}
				}
			}
		}
	}
	break;

	case MG_EV_HTTP_PART_BEGIN:
	case MG_EV_HTTP_PART_DATA:
	case MG_EV_HTTP_PART_END:
		mg_file_upload_handler(nc, ev, ev_data, "upload.dat");
		break;
	}
}

#endif
