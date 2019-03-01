#ifndef menu_h
#define menu_h

#include <Arduino.h>
#include "config.h"
#include <functional>
#include "Array.h"

class CMenuItem;
class CMenu;
class CMenuDisplayAdapter;
typedef  CSimpleArray<CMenuItem*> MenuItems;
typedef std::function<void()> func_onselect;
class CMenuItem {
public:
	CMenuItem(CMenu* pParent);
	CMenuItem(CMenu* pParent,String& text);



	const String& getname() { return name; };
private:
	String name;
	CMenu* pParent;
};
enum nav {next,prev,first,last};
class CMenu {
public:
	CMenu();
	const CMenuItem* AddItem(String& text);
	const MenuItems getitems() { return  items; };
	const CMenuItem* getcurrent();
	const CMenuItem*  donavigation(nav cmd);
	const CMenuDisplayAdapter* get_adapter() { return pAdapter; };
	void set_adapter(CMenuDisplayAdapter* p) {  pAdapter=p; };
	void redraw();
private:
	MenuItems items;
	CMenuDisplayAdapter* pAdapter;
	//CMenuItem* pCurrent;
	size_t current;
};

class CMenuDisplayAdapter {
public:
	virtual int getmaxlines() { return get_maxy() / lineheight; };
	virtual int get_maxy() { return 0; };
	virtual int get_maxx() { return 0; };
	virtual int drawline(size_t idx, const String& text) {};
	virtual void setlineheight(size_t val) { lineheight = val; };
	virtual void setfirstindex(size_t val) { firtsindex = val; };
	const size_t getfirstindex() { return firtsindex; };
	virtual size_t getcurrentpage() { return getpage(firtsindex); };
	virtual size_t getpage(size_t lineidx) { return lineidx / getmaxlines(); };
	virtual void setpage(size_t lineidx) { setfirstindex(getpage(lineidx)*getmaxlines()); };
protected:
	size_t lineheight;
	size_t firtsindex;
};

#endif