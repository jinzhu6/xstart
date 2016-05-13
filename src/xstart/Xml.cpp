#include <fstream>
#include <sstream>
#include <expat/expat.h>
#include "corela.h"
#include "strtrim.h"
#include "Xml.h"


void XMLCALL _XMLElementStart(void* user, const XML_Char* name, const XML_Char** attr) {
	XMLDocument* doc = (XMLDocument*)user;
	// TODO: Check whether the XMLNode() is freed properly through the gm-machine.
	XMLNode* node = new XMLNode();
	node->name = name;

	for(int i=0; attr[i]; i+=2) {
		node->setAttribute(attr[i], attr[i+1]);
	}

	doc->_pushNode(node);
}

void XMLCALL _XMLElementEnd(void* user, const XML_Char* name) {
	XMLDocument* doc = (XMLDocument*)user;
	doc->_popNode();

	// trim space from node content
	XMLNode* node = doc->_getCurrent();
	node->content = trim(node->content);
}

void XMLCALL _XMLCharData(void* user, const XML_Char* s, int len) {
	XMLDocument* doc = (XMLDocument*)user;
	XMLNode* node = doc->_getCurrent();

	if(node) {
		node->content += std::string(s, len);
	}
}

void XMLCALL _XMLCDataStart(void* user) {
	XMLDocument* doc = (XMLDocument*)user;
	XMLNode* node = doc->_getCurrent();
	if(node) { node->isCDATA = true; }
}

void XMLCALL _XMLCDataEnd(void* user) {
	XMLDocument* doc = (XMLDocument*)user;
}

bool XMLDocument::load(const std::string &file) {
	XML_Parser parser = XML_ParserCreate(0);
	XML_SetElementHandler(parser, _XMLElementStart, _XMLElementEnd);
	XML_SetCdataSectionHandler(parser, _XMLCDataStart, _XMLCDataEnd);
	XML_SetCharacterDataHandler(parser, _XMLCharData);
	XML_SetUserData(parser, this);

	stack.clear();

	bool fromFile = false;
	coDword fsize;
	char* xml;
	if(file[0] == '<') {
		xml = (char*)file.c_str();
		fsize = strlen(xml);
	} else {
		fromFile = true;
		fsize = FileGetSize(_FILE(file.c_str()));
		xml = (char*)malloc(fsize);
		FileReadBuffer(_FILE(file.c_str()), xml, &fsize);
	}

	XML_Parse(parser, xml, fsize, 1);
	XML_ParserFree(parser);

	if(fromFile) { free(xml); }

	return true;
}

bool XMLDocument::save(const std::string &file) {
	FileSaveBuffer(_FILE(file.c_str()), root->toXML().c_str(), -1);
	return true;
}
