#ifndef _SCREEN_H_
#define _SCREEN_H_


#include <corela.h>
#include "ScriptObject.h"
#include "Rect.h"
#include <list>
using namespace std;


#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a<b?b:a)



class Element : public ScriptObject
{
public:

	Element() : ScriptObject()
	{
		id = "Element";
		rect = 0;

		BindFunction("draw", (SCRIPT_FUNCTION)&Element::_draw);
		BindFunction("hitTest", (SCRIPT_FUNCTION)&Element::_hitTest);

		BindMember("rect", &rect, TYPE_OBJECT);
		SetMemberObject("rect", new Rect(), 0);

		//BindMember("rect", &rect, TYPE_UNKNOWN);
		//SetMember<void*>("rect", &rect);
		
		/*gmVariable var;
		var.SetUser(machine, (void*)&rect, GM_TYPE_OBJECT);
		table->Set(machine, "rect", var);*/
	}

	~Element() { }

	void addChild(Element* e)
	{
		if(e->parent) { e->parent->removeChild(e); }
		childs.push_back(e);
		e->parent = this;
	}

	void removeChild(Element* e)
	{
		if(e->parent != this) return;
		childs.remove(e);
		e->parent = 0;
	}

	void draw()
	{
		// TODO: draw "this"
		//RendDraw2D(rend, texture, area.left, area.top, area.right-area.left, area.bottom-area.top);
		printf("Element->draw()\n");

		list<Element*>::iterator i;
		for(i = childs.begin(); i != childs.end(); i++)
		{
			(*i)->draw();
		}
	}

	int _draw(gmThread* a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		draw();
		return GM_OK;
	}

	Element* hitTest(double x, double y)
	{
		if(!rect->hitTest(x,y)) return 0;

		Element* hit = this;
		
		list<Element*>::iterator i;
		for(i = childs.begin(); i != childs.end(); i++)
		{
			hit = (*i)->hitTest(x,y);
		}

		return hit;
	}

	int _hitTest(gmThread* a_thread)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_FLOAT_OR_INT_PARAM(x, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(y, 1);

		Element* hit = hitTest(x, y);
		if(!hit) { a_thread->PushNull(); return GM_OK; }
		
		gmUserObject* user = machine->AllocUserObject(hit, GM_TYPE_OBJECT);
		a_thread->PushUser(user);

		return GM_OK;
	}

private:
	Rect* rect;
	TEXTURE* texture;
	Element* parent;
	list<Element*> childs;
};


class Screen : public ScriptObject
{
public:

	Screen() : ScriptObject()
	{
		BindFunction("draw", (SCRIPT_FUNCTION)&Screen::gm_draw);
		BindMember("root", &root, TYPE_OBJECT);
	}
	~Screen() { }
	
	int gm_draw(gmThread* a_thread)
	{
		if(!root) return GM_OK;
		printf("\nScreen->draw()... [%d]\n", root);
		if(root) root->draw();
		return GM_OK;
	}

	Element* hitTest(double x, double y)
	{
		if(!root) return 0;
		return root->hitTest(x,y);
	}

private:
	Element* root;
};


#endif
