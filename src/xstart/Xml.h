#ifndef _XML_H_
#define _XML_H_

#include "ScriptObject.h"
#include "Array.h"
#include "Map.h"

#include <string>
#include <cstring>
#include <list>
#include <vector>


class XMLNode : public ArrayObject {
public:

	XMLNode() : ArrayObject() {
		id = "XMLNode";
		help = "XML node class.";

		name = "";
		content = "";
		attributes = new Map();
		isCDATA=false;

		BindMember("name", &name, TYPE_STRING);
		BindMember("content", &content, TYPE_STRING);
		BindMember("isCDATA", &isCDATA, TYPE_INT);
		BindMember("attributes", &attributes, TYPE_OBJECT);
		BindFunction("getChild", (SCRIPT_FUNCTION)&XMLNode::gm_getChild);
	}

	~XMLNode() { }

	int Initialize(gmThread* a_thread) {
		switch(a_thread->GetNumParams()) {
		case 0:
			return GM_OK;
		case 1:
			name = a_thread->ParamString(0, "");
			return GM_OK;
		case 2:
			name = a_thread->ParamString(0, "");
			content = a_thread->ParamString(1, "");
			return GM_OK;
		}
		return GM_OK;
	}

	void addChild(XMLNode* node) {
		this->addObject(node);
		// NOTE: The line below creates a memory leak!
//		this->add(gmVariable(machine->AllocUserObject(node, GM_TYPE_OBJECT)));
	}
//	void removeChild(const XMLNode* node) { childs.remove(*node); }

	XMLNode* getChild(std::string name="", int index = 0) {
		int i=0;
		while(XMLNode* n = (XMLNode*)this->iteratorGet(i++).GetUserSafe(GM_TYPE_OBJECT)) {
			if(strncmp( n->name.c_str(), name.c_str(), strlen(name.c_str()) ) == 0)
				if(index-- <= 0) { return n; }
		}
		return 0;
	}
	int gm_getChild(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(name, 0);
		GM_CHECK_INT_PARAM(index, 1);
		XMLNode* node = getChild(name, index);
		if(node) { return node->ReturnThis(a_thread); }
		else { return this->ReturnNull(a_thread); }
	}

	XMLNode* setAttribute(std::string key, std::string value) {
		attributes->set(key, value);
		return this;
	}
	std::string getAttribute(std::string key, int index = 0) {
		return attributes->get(key);
	}

	std::string toXML(int depth=0) {
		std::string out;

		bool multiline = false;

		if(this->length) {
			multiline = true;
			if(depth != 0) { out += "\n"; }
		}

		for(int k=0; k<depth; k++) { out += std::string("    "); }
		out += std::string("<") + name;

		out += attributes->toXML(0) + ">";
		if(isCDATA && content.length()) { out += "<![CDATA["; }
		out += content;
		if(isCDATA && content.length()) { out += "]]>"; }

		if(this->length) {
			if(multiline) { out += "\n"; }

			int i=0;
			while(XMLNode* n = (XMLNode*)this->iteratorGet(i++).GetUserSafe(GM_TYPE_OBJECT)) {
				out += n->toXML(depth+1);
			}

			if(multiline)
				for(int k=0; k<depth; k++) { out += std::string("    "); }
		}

		out += "</" + name + ">\n";
		if(multiline && depth != 0) { out += "\n"; }

		return out;
	}

public:
	bool isCDATA;
	std::string name;
	std::string content;
	Map* attributes;
};


#if 0
class XMLNode : public ScriptObject {
public:

	XMLNode() : ScriptObject() {
		id = "XMLNode";
		help = "XML node class.";

		name = "";
		content = "";
		childs = new ArrayObject();
		attributes = new Map();
		isCDATA=false;

		BindMember("name", &name, TYPE_STRING);
		BindMember("content", &content, TYPE_STRING);
		BindMember("isCDATA", &isCDATA, TYPE_INT);
		BindMember("attributes", &attributes, TYPE_OBJECT);
		BindMember("childs", &childs, TYPE_OBJECT);
		BindFunction("getChild", (SCRIPT_FUNCTION)&XMLNode::gm_getChild);
	}

	~XMLNode() {
		//delete childs;
	}

	int Initialize(gmThread* a_thread) {
		switch(a_thread->GetNumParams()) {
		case 0:
			return GM_OK;
		case 1:
			name = a_thread->ParamString(0, "");
			return GM_OK;
		case 2:
			name = a_thread->ParamString(0, "");
			content = a_thread->ParamString(1, "");
			return GM_OK;
		}
		return GM_OK;
	}

	void addChild(XMLNode* node) {
//		childs->addObject(node);
		childs->add(gmVariable(machine->AllocUserObject(node, GM_TYPE_OBJECT)));
	}
//	void removeChild(const XMLNode* node) { childs.remove(*node); }

	XMLNode* getChild(std::string name="", int index = 0) {
		int i=0;
		while(XMLNode* n = (XMLNode*)childs->iteratorGet(i++).GetUserSafe(GM_TYPE_OBJECT)) {
			if(strncmp( n->name.c_str(), name.c_str(), strlen(name.c_str()) ) == 0)
				if(index-- <= 0) { return n; }
		}
		return 0;
	}
	int gm_getChild(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(name, 0);
		GM_CHECK_INT_PARAM(index, 1);
		XMLNode* node = getChild(name, index);
		if(node) { return node->ReturnThis(a_thread); }
		else { return this->ReturnNull(a_thread); }
	}

	XMLNode* setAttribute(std::string key, std::string value) {
		attributes->set(key, value);
		return this;
	}
	std::string getAttribute(std::string key, int index = 0) {
		return attributes->get(key);
	}

	std::string toXML(int depth=0) {
		std::string out;

		bool multiline = false;
		if(childs) {
			if(childs->length > 0) {
				multiline = true;
				if(depth != 0) { out += "\n"; }
			}
		}

		for(int k=0; k<depth; k++) { out += std::string("    "); }
		out += std::string("<") + name;

		out += attributes->toXML(0) + ">";
		if(isCDATA && content.length()) { out += "<![CDATA["; }
		out += content;
		if(isCDATA && content.length()) { out += "]]>"; }

		if(childs) {
			if(multiline) { out += "\n"; }

			int i=0;
			while(XMLNode* n = (XMLNode*)childs->iteratorGet(i++).GetUserSafe(GM_TYPE_OBJECT)) {
				out += n->toXML(depth+1);
			}
			/*			for(i=0; i<childs->length; i++) {
							gmVariable vari = childs->iteratorGet(i);
							XMLNode* n = (XMLNode*)vari.GetUserSafe(GM_TYPE_OBJECT);
							if(!n) { continue; }
							out += n->toXML(depth+1);
						}*/

			if(multiline)
				for(int k=0; k<depth; k++) { out += std::string("    "); }
		}

		out += "</" + name + ">\n";
		if(multiline && depth != 0) { out += "\n"; }

		return out;
	}

public:
	std::string name;
	std::string content;
	bool isCDATA;

private:
	Map* attributes;
	ArrayObject* childs;
};
#endif


class XMLDocument : public ScriptObject {
public:

	XMLDocument() : ScriptObject() {
		id = "XMLDocument";
		help = "XML document class.";

		root = 0;
		BindFunction("load", (SCRIPT_FUNCTION)&XMLDocument::gm_load);
		BindFunction("save", (SCRIPT_FUNCTION)&XMLDocument::gm_save);
		BindFunction("toXML", (SCRIPT_FUNCTION)&XMLDocument::gm_toXML);
		BindFunction("setRoot", (SCRIPT_FUNCTION)&XMLDocument::gm_setRoot);
		BindMember("root", &root, TYPE_OBJECT);
	}
	~XMLDocument() {
//		while(_popNode()) {}
	}

	int Initialize(gmThread* a_thread) {
		switch(a_thread->GetNumParams()) {
		case 0:
			return GM_OK;
		case 1:
			load(std::string(a_thread->ParamString(0, "")));
			return GM_OK;
		}
		return GM_OK;
	}

	bool load(const std::string &file);
	int gm_load(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(fileName, 0);
		a_thread->PushInt((int)load(fileName));
		return GM_OK;
	}

	bool save(const std::string &file);
	int gm_save(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_STRING_PARAM(fileName, 0);
		a_thread->PushInt((int)save(fileName));
		return GM_OK;
	}

	std::string toXML() {
		return root->toXML();
	}
	int gm_toXML(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushNewString(toXML().c_str());
		return GM_OK;
	}

	int gm_setRoot(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(ScriptObject*, GM_TYPE_OBJECT, object, 0);
		SetMemberObject("root", object, &a_thread->Param(0));
		return GM_OK;
	}

	XMLNode* _pushNode(XMLNode* node) {
		// this essentially builds the tree with the nodes (parents/childs)
		if(stack.size() > 0) {
			stack.back()->addChild(node);
		} else {
			SetMemberObject("root", node);
		}
		stack.push_back(node);
		return node;
	}
	int _popNode() {
		// the last element (the root) should not be popped from the stack
		if(stack.size() > 1) {
//			XMLNode* node = stack.back();
			stack.pop_back();
//			free(node);
		}
		return stack.size();
	}
	XMLNode* _getCurrent() {
		return stack[stack.size()-1];
	}

private:
	XMLNode* root;
	std::vector<XMLNode*> stack;  // temporary stack for building the XML structure in code.
};


#endif
