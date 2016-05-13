#ifndef _EVENT_H_
#define _EVENT_H_


#include "ScriptObject.h"
#include "List.h"
#include "Map.h"


class Event : public ScriptObject {
public:

	Event() : ScriptObject() {
		id = "Event";
		help = "Event object that is received by all events as the first parameter. \
[Handler] is the base class for all objects that can receive events. \
The @id on this, is set to the event name.<br/><br/>\
Common events are: <b>Close, MouseMove, MouseDown, MouseUp, KeyDown, KeyUp</b>.<br/>\
These are received by setting these event handlers on the receiver object: <b>onClose, onMouseMove, onMouseDown, onMouseUp, onKeyDown, onKeyUp</b>. \
Mouse events are send to all elements under them, if an event handler returns true, the event is not send further down the scene-tree hierarchy - however, you should then avoid using sleep() or yield() in the event handler.";

		sender = 0;
		key = 0;
		x = y = 0;
		button = 0;

		BindMember("sender", &sender, TYPE_OBJECT, 0, "[Object] sender", "Origin of the event, this is set to the object on which the event was rised.");
		BindMember("key", &key, TYPE_INT, 0, "{int} key", "For keyboard events, this is set to the keycode.");
		BindMember("button", &button, TYPE_INT, 0, "{int} button", "For mouse events, this is set to the button.");
		BindMember("x", &x, TYPE_FLOAT, 0, "{float} x", "For mouse move events, this is the mouse x-coordinate");
		BindMember("y", &y, TYPE_FLOAT, 0, "{float} y", "For mouse move events, this is the mouse y-coordinate");
	}

	void SetSender(ScriptObject* sender) {
		SetMemberObject("sender", sender, 0);
	}

public:

	ScriptObject* sender;
	int key;
	float x;
	float y;
	int button;
};


inline bool HandleEvent(Event* e, ScriptObject* receiver) {
	gmVariable fnVar;
	gmFunctionObject* fnObj;
	gmCall call;

	e->SetSender(receiver);

	int cancelEvent = 0;
	fnVar = receiver->table->Get(machine, (std::string("on")+std::string(e->id)).c_str());
	if(!fnVar.IsNull()) {
		if(fnVar.m_type == GM_FUNCTION) {
			fnObj = fnVar.GetFunctionObjectSafe();
			if(call.BeginTableFunction(machine, (std::string("on")+std::string(e->id)).c_str(), receiver->table, gmVariable(receiver->userObject), false)) {
				call.AddParamUser(e->userObject);
				if(e->id == "Close") {
					call.End();
					return true;
				} else {
					call.End();
					if(call.DidReturnVariable()) {
						call.GetReturnedInt(cancelEvent);
					}
				}
			}
		}
	}
	return cancelEvent != 0;
}


class Handler : public ScriptObject {
public:

	Handler() : ScriptObject() {
		id = "Handler";
		help = "Base class for all [Node] objects that receive events of type [Event].";
		isEventHandler = true;
	}

	virtual bool HandleEvent(Event* e) {
		return ::HandleEvent(e, this);
	}

	int gm_event(gmThread* a_thread) {
		return GM_OK;
	}

public:

	List* events;
};


static int OnEventGlobal(FRAME_EVENT* fe) {
	// receiver is the Frame object
	Handler* receiver = (Handler*)fe->user;
	if(!receiver->userObject) {
		Log(LOG_DEBUG, "Frame script user object (fe->user) was zero in event handler!");
		return 0;
	}

	Event* e = new Event();
	e->userObject = machine->AllocUserObject(e, GM_TYPE_OBJECT);
	machine->AddCPPOwnedGMObject(e->userObject);

	e->x = (float)fe->x;
	e->y = (float)fe->y;
	e->key = fe->key;
	e->button = fe->button;

	switch(fe->id) {
	case EVENT_CLOSE:
		e->id = "Close";
		receiver->HandleEvent(e);
		break;
	case EVENT_UPDATE:
		e->id = "Update";
		receiver->HandleEvent(e);
		break;
	case EVENT_RENDER:
		e->id = "Render";
		receiver->HandleEvent(e);
		break;
	case EVENT_MOUSE_MOVE:
		e->id = "MouseMove";
		receiver->HandleEvent(e);
		break;
	case EVENT_MOUSE_BUTTON_DOWN:
		e->id = "MouseDown";
		receiver->HandleEvent(e);
		break;
	case EVENT_MOUSE_BUTTON_UP:
		e->id = "MouseUp";
		receiver->HandleEvent(e);
		break;
	case EVENT_KEY_DOWN:
		e->id = "KeyDown";
		receiver->HandleEvent(e);
		break;
	case EVENT_KEY_UP:
		e->id = "KeyUp";
		receiver->HandleEvent(e);
		break;
	}

	machine->RemoveCPPOwnedGMObject(e->userObject);

	return 0;
}


static int RaiseEvent(EVENT_ID eid, Handler* receiver) {
	FRAME_EVENT fe;
	memset(&fe, 0, sizeof(FRAME_EVENT));
	fe.id = eid;
	fe.user = receiver;
	return OnEventGlobal(&fe);
}


#endif
