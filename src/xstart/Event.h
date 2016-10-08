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
Common events are: <b>Close, MouseMove, MouseDown, MouseUp, MouseEnter, MouseLeave, KeyDown, KeyUp</b>.<br/>\
These are received by setting these event handlers on the receiver object: <b>onClose, onMouseMove, onMouseDown, onMouseUp, onMouseEnter, onMouseLeave, onKeyDown, onKeyUp</b>. \
Mouse events are send to all elements under them, if an event handler returns true, the event is not send further down the scene-tree hierarchy - however, you should then avoid using sleep() or yield() in the event handler.";

		node = 0;
		key = 0;
		text = "";
		x = y = 0;
		prevX = prevY = 0;
		button = 0;

		BindMember("node", &node, TYPE_OBJECT, 0, "[Object] node", "Receiving node of the event, this is set to the object on which the event was rised.");
		BindMember("key", &key, TYPE_INT, 0, "{int} key", "For keyboard events, this is set to the keycode.");
		BindMember("text", &text, TYPE_STRING, 0, "{string} text", "<b>EXPERIMENTAL</b> Text representation of input key.");
		BindMember("button", &button, TYPE_INT, 0, "{int} button", "For mouse events, this is set to the button index.");
		BindMember("x", &x, TYPE_FLOAT, 0, "{float} x", "For mouse move events, this is the mouse x-coordinate.");
		BindMember("y", &y, TYPE_FLOAT, 0, "{float} y", "For mouse move events, this is the mouse y-coordinate.");
		BindMember("prevX", &prevX, TYPE_FLOAT, 0, "{float} prevX", "For mouse move events, the previous mouse x-coordinate.");
		BindMember("prevY", &prevY, TYPE_FLOAT, 0, "{float} prevY", "For mouse move events, the previous mouse y-coordinate.");
	}

	void SetSender(ScriptObject* node) {
		SetMemberObject("node", node, 0);
	}

public:
	ScriptObject* node;
	int key;
	std::string text;
	float x;
	float y;
	float prevX;
	float prevY;
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
		help = "MAY NOT BE OF USE IN SCRIPTS. Base class for all [Node] objects that receive events of type [Event].";
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
	e->prevX = (float)fe->prevX;
	e->prevY = (float)fe->prevY;
	e->key = fe->key;
	e->text = fe->text;
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
	case EVENT_MOUSE_ENTER:
		e->id = "MouseEnter";
		receiver->HandleEvent(e);
		break;
	case EVENT_MOUSE_LEAVE:
		e->id = "MouseLeave";
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
