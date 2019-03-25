#ifndef menu_h
#define menu_h

#include <Arduino.h>
#include "config.h"
#include <functional>
#include "Array.h"
#include <SSD1306.h>
#include "BaseController.h"

class CMenuItem;
class CMenu;
class CTopLevelMenu;
class ButtonController;
struct ButtonState;
struct MenuState
{
	int current = 0;
};
enum MenuCMD :uint { Next,Prev,Select, LDRSaveState = 4096 };

class MenuController;

typedef CController<MenuController, MenuState, MenuCMD> MenuCtl;

class MenuController : public MenuCtl {
public:
	MenuController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	virtual void run();
	void onButtonEvent(ButtonState bs);
protected:
	CTopLevelMenu* pmenu;
	ButtonController* pBtnController;
};

DEFINE_CONTROLLER_FACTORY(MenuController)

class CMenuDisplayAdapter;
typedef  CSimpleArray<CMenuItem*> MenuItems;
typedef std::function<void()> func_onselect;
class CMenuItem {
public:
	CMenuItem(CMenu* pParent);
	CMenuItem(CMenu* pParent,String& text);


	virtual void drawContent(CMenuDisplayAdapter& adapter);
	virtual void drawPreview(CMenuDisplayAdapter& adapter);
	virtual void doAction() {};
	virtual void onPreload() {};
	const String& getname() { return name; };
	CMenu* getsubmenu() { return pSubMenu; };
	
	bool isSubMenu() { return pSubMenu!=NULL; };
	CMenu* getParent() { return pParent; };
protected:
	void setsubmenu(CMenu* p) { pSubMenu=p; };
private:
	String name;
	CMenu* pParent;
	CMenu* pSubMenu;
};
enum nav {next,prev,first,last,action,back};
enum MenuVisualState {Main,SubMenu,Content};

class CMenu {
public:
	CMenu();
	void AddItem(String& text);
	void AddItem(CMenuItem* pItem);
	MenuItems& getitems() { return  items; };
	CMenuItem* getcurrent();

	void redraw(CMenuDisplayAdapter* pAdapter);
	size_t getcurrentidx() { return currentidx; };
	void setcurrentidx(size_t val) { currentidx = val; };
	CMenuItem* getParentItem(CMenu* pMenu);
protected :
	void drawItems(const MenuItems& its, CMenuDisplayAdapter* pAdapter);
	const MenuItems& getActiveItems();
	virtual bool isTopLevel() { return false; }
	MenuVisualState getVisualState() { return vs_state; };
	void setVisualState(MenuVisualState vs) { vs_state=vs; };
	MenuVisualState vs_state;

private:
	MenuItems items;
	size_t currentidx;
	//CMenuItem* pCurrent;
	
	
	
};
class CTopLevelMenu :public CMenu {
public :
	CTopLevelMenu();
	const CMenuDisplayAdapter* get_adapter() { return pAdapter; };
	void redraw();
	void set_adapter(CMenuDisplayAdapter* p) { pAdapter = p; };
	CMenuItem*  donavigation(nav cmd);
	bool isRedrawRequired() { return bredrawRequired; };
	void invalidate() { bredrawRequired = true; }
	void setActiveSubMenu(CMenu* pVal) {  pActiveSubMenu=pVal; };
	CMenu* getActiveSubMenu() { return pActiveSubMenu; };
protected:
	virtual bool isTopLevel() { return true; };
private:
	CMenuDisplayAdapter* pAdapter;
	CMenu* pActiveSubMenu;
	bool bredrawRequired;
};

class CTopRFMenuItem :public CMenuItem {
public:
	CTopRFMenuItem(CMenu* pParent);
	CTopRFMenuItem(CMenu* pParent, String& text);
	virtual void onPreload();
};
class CActionRFMenuItem :public CMenuItem {
public:
	CActionRFMenuItem(CMenu* pParent) :CMenuItem(pParent) {};
	CActionRFMenuItem(CMenu* pParent, String& text) :CMenuItem(pParent, text) {};
	virtual void doAction() ;
};
class CMenuDisplayAdapter {
public:
	CMenuDisplayAdapter() {
		lineheight = 8;
		firtsindex = 0;
		preview_offset = 0;
	};
	virtual void setup(int adr= 0x3c, int pinSda =5, int pinScl =4) {};
	virtual int getmaxlines() { return get_maxy() / lineheight; };

	virtual int get_maxy() { return 0; };
	virtual int get_maxx() { return 0; };
	virtual void clear() {};
	virtual void update() {};
	virtual int drawline(size_t idx, const String& text) {};
	virtual void drawtext(size_t x, size_t y, const String& text) {};
	virtual size_t getlineheight() { return lineheight; };
	virtual void setlineheight(size_t val) { lineheight = val; };

	virtual void setfirstindex(size_t val) { firtsindex = val; };
	const size_t getfirstindex() { return firtsindex; };
	virtual size_t getcurrentpage() { return getpage(firtsindex); };
	virtual size_t getpage(size_t lineidx) { return lineidx / getmaxlines(); };
	virtual void setpage(size_t lineidx) { setfirstindex(getpage(lineidx)*getmaxlines()); };
protected:
	size_t lineheight;
	size_t firtsindex;
	size_t preview_offset;
};

//class SSD1306;

class CMenuDisplayAdapterSSD1306 :public  CMenuDisplayAdapter {
public:
	CMenuDisplayAdapterSSD1306();
	~CMenuDisplayAdapterSSD1306();
	virtual int get_maxy() { return 32; };
	virtual int get_maxx() { return 128; };
	virtual void setup(int adr = 0x3c, int pinSda = 1, int pinScl = 2);
	virtual int drawline(size_t idx, const String& text) ;
	virtual void drawtext(size_t x, size_t y, const String& text) ;
	virtual void clear() ;
	virtual void update() ;
protected:
	SSD1306* pdisplay; 
};
#endif