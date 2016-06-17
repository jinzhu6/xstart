#ifndef _MAP_H_
#define _MAP_H_

#include "ScriptObject.h"
#include <algorithm>
#include <vector>
#include <string>


class Map : public ScriptObject {
public:

	Map() : ScriptObject() {
		id = "Map";
		help = "Map class for mapping a list of values and/or object to strings indices. Supports foreach iteration, the order of the insertions/changes is preserved. For convenience the dot . and index [] operators can be used to modify and add to the map.";

		BindFunction("get", (SCRIPT_FUNCTION)&Map::gm_get, "[Object] get({string} key)", "Gets the mapped object for the given key. If the key does not exist, the value null is returned.");
		BindFunction("set", (SCRIPT_FUNCTION)&Map::gm_set, "[this] set({string} key, {object} value)", "Sets the value for the key or creates a new key with the given value. If <i>value</i> is <i>null</i> the given <i>key</i> is removed.");
		BindFunction("length", (SCRIPT_FUNCTION)&Map::gm_length, "{int} length()", "Returns the number of entries in the map.");
	}

	gmVariable iteratorGet(int n) {
		while(n < 0) { n += order.size(); }
		if(n >= order.size())  {
			gmVariable v;
			v.Nullify();
			return v;
		}
		// NOTE: We can either return the key/name of the entry or the object to that name. In the latter case the names/keys are not iteratable - so we use former method.
//		return table->Get(machine, order[n].c_str());
		gmVariable var;
		var.SetString(machine, order[n].c_str());
		return var;
	}

	std::string get(std::string key) {
		gmVariable var = table->Get(machine, key.c_str());
		if(var.IsNull()) { return ""; }
		return std::string(var.GetCStringSafe());
	}

	bool has(std::string key) {
		gmVariable var = table->Get(machine, key.c_str());
		if(var.IsNull()) { return false; }
		return true;
	}

	void set(std::string key, std::string val) {
		gmVariable var;
		var.SetString(machine, val.c_str());
		table->Set(machine, key.c_str(), var);

		if(val == "") { _eraseExisting(key); return; }

		std::vector<std::string>::iterator it = std::find(order.begin(), order.end(), key);
		if(it == order.end()) {
			order.push_back(key);
		} else {
			int pos = std::distance(order.begin(), it);
			order.at(pos) = key;
		}
	}

	void set(std::string key, gmVariable& val) {
		table->Set(machine, key.c_str(), val);
		if(val.IsNull()) { _eraseExisting(key); return; }

		std::vector<std::string>::iterator it = std::find(order.begin(), order.end(), key);
		if(it == order.end()) {
			order.push_back(key);
		} else {
			int pos = std::distance(order.begin(), it);
			order.at(pos) = key;
		}
	}

	int gm_set(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_STRING_PARAM(key, 0);
		set(key, a_thread->Param(1));
		return ReturnThis(a_thread);
	}

	int gm_get(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(key, 0);
		gmVariable var = table->Get(machine, key);
		a_thread->Push(var);
		return GM_OK;
	}

	void _eraseExisting(std::string key) {
		std::vector<std::string>::iterator i;
		for(i = order.begin(); i != order.end(); i++) {
			if(*i == key) {
				order.erase(i);
				return;
			}
		}
	}

	void SetDot(const char* key, gmVariable &var) {
		_eraseExisting(key);
		if(!var.IsNull()) {
			order.push_back(key);
		}
	}

	int length() {
		return this->order.size();
	}
	int gm_length(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(0);
		a_thread->PushInt(length());
		return GM_OK;
	}

	std::string toXML(int depth=0) {
		std::string begin = "";
		std::string out = "";

		for(std::vector<std::string>::iterator it = order.begin(); it != order.end(); it++) {
			std::string key = *it;
			std::string val = get(key);

			if(key == "id" || key == "name") {
				begin += " " + key + "=\"" + val + "\"";
				continue;
			}
			if(depth > 0) {
				out += "\n";
				for(int k=0; k<depth; k++) { out += std::string("    "); }
			}

			if(depth > 0) {
				std::string key_fixed;
				key_fixed = key;
				int trail_space = 10 - key_fixed.length();
				if(trail_space > 0) for(int j=0; j<trail_space; j++) { key_fixed += " "; }
				key = key_fixed;
			}

			out += " " + key + "=\"" + val + "\"";
		}

		if(depth > 0 && order.size() > 0) {
			out += "\n";
			for(int k=0; k<depth; k++) { out += std::string("    "); }
		}

		return begin + out;
	}

public:
	std::vector<std::string> order;
};


#endif
