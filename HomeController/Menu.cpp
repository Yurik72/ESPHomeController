#include "config.h"
#include "Utilities.h"
#include "Menu.h"
#include "Controllers.h"
#include "ButtonController.h"
#include "RFController.h"


#ifndef DISABLE_MENU
REGISTER_CONTROLLER_FACTORY(MenuController)
#endif

const size_t bufferSize = JSON_OBJECT_SIZE(20);
static MenuController* _instance = NULL;

#define BUF_SIZE_LDR  JSON_OBJECT_SIZE(20)
MenuController::MenuController() {
	pmenu = new CTopLevelMenu();
	pBtnController = NULL;
	pinsda = 5;
	pinslc = 4;
	i2caddr = 0x3c;
	rfController="RF";
	buttonController="Button";
	_instance = this;
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println("MenuController CTOR");
	
#endif //  LDRCONTROLLER_DEBUG
}
String  MenuController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();

	root["current"] = this->get_state().current;
	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  MenuController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	MenuState newState;
	newState.current = root["current"];
	
	//this->AddCommand(newState, Measure, srcSelf);
	//this->set_state(newState);
	return true;

}
void MenuController::loadconfig(JsonObject& json) {
	uint val = 0;

	loadif(pinsda, json, "pinsda");
	loadif(pinslc, json, "pinslc");
	loadif(i2caddr, json, "i2caddr");
	/*
	val = json["pinsda"];
	if(val)	pinsda = val;
	val = json["pinslc"];
	if (val)	pinslc = val;
	val = json["i2caddr"];
	if (val)	i2caddr = val;
	*/
	loadif(rfController, json, "RFController");
	loadif(buttonController, json, "ButtonController");
	/*
	String s;
	s = json["RFController"].as<String>();
	if (s.length())
		rfController = s;
	s = json["ButtonController"].as<String>();
	if (s.length())
		buttonController = s;
	*/
}
void MenuController::getdefaultconfig(JsonObject& json) {
	json["pinsda"] = pinsda;
	json["pinslc"] = pinsda;
	json["i2caddr"]= i2caddr;
	json["service"] = "MenuController";
	json["name"] = "Menu";
	json["RFController"] = rfController;
	json["ButtonController"] = buttonController;

	MenuCtl::getdefaultconfig(json);
}
void  MenuController::setup() {
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println("MenuController setup");
	
#endif 
	CMenuDisplayAdapter* p = new CMenuDisplayAdapterSSD1306();
	p->setup(i2caddr, pinsda, pinslc);
	pmenu->set_adapter(p);
///test
	this->loadservices();
//	String s = "Item 1";
//	pmenu->AddItem(s);
//	s = "Item 2";
//	pmenu->AddItem(s);
//	s = "RF Sender";
//	CTopRFMenuItem* prfTop = new CTopRFMenuItem(static_cast<CMenu*>(pmenu),s);
	
	//pmenu->AddItem(prfTop);
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println(F("Setup button handlers "));
#endif
	CBaseController* pBCtl = Controllers::getInstance()->GetByName(this->getbtnname());
	if (pBCtl) {

		pBtnController=static_cast<ButtonController*>(pBCtl);
		MenuController* self = this;
		pBtnController->add_eventshandler_statechange(
			[self](CBaseController* p,ButtonState bs) {
			self->onButtonEvent(bs);
		}
		);
	}
	else{
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.println(F("can't find button controller"));
#endif
	}
}
void MenuController::onButtonEvent(ButtonState bs) {
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println(F("Button event"));
	DBG_OUTPUT_PORT.println(bs.idx);
#endif
	if (bs.isPressed) {
		switch (bs.idx) {
		case 1:
			pmenu->donavigation(next);
			break;
		case 2:
			pmenu->donavigation(prev);
			break;
		case 0:
			pmenu->donavigation(action);
			break;

		}
	}
}
void MenuController::run() {
	if (!pmenu)
		return;
	if (pmenu->isRedrawRequired()) {

		pmenu->redrawall();
	}
}

void MenuController::loadservices() {

	String s;
	s = "RF Sender";
	CTopRFMenuItem* prfTop = new CTopRFMenuItem(pmenu, s);
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println("loadservices");
	DBG_OUTPUT_PORT.println((long)prfTop);
#endif
	pmenu->AddItem(prfTop);
	for (int i = 0;i < Controllers::getInstance()->GetSize();i++) {
		CBaseController* p = Controllers::getInstance()->GetAt(i);
		s = p->get_name();
		CMenuItem* pitem=pmenu->AddItem(s);
		pitem->setcontroller(p);
	}


}

CTopLevelMenu::CTopLevelMenu() {
	vs_state = Main;
	pActiveSubMenu = NULL;
	bredrawRequired = true;
}
void CTopLevelMenu::redrawall() {
	pAdapter->clear();
	CMenuItem* pCurrent = this->getcurrent();
	if (this->getActiveSubMenu())
		pCurrent = this->getActiveSubMenu()->getcurrent();
	if (this->getVisualState() == SubMenu && this->getActiveSubMenu()) {
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.println(F("Redraw sub menu"));

#endif
		this->getActiveSubMenu()->redraw(pAdapter);
	}
	else if (this->getVisualState()==Content && pCurrent){
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.println(F("Draw content"));

#endif
		pCurrent->drawContent(*pAdapter);
	}
	else {
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.println(F("redraw top level"));
#endif
		redraw(pAdapter);
	}
	pAdapter->update();
	bredrawRequired = false;
};
CMenuItem* CTopLevelMenu::donavigation(nav cmd) {
	if (getitems().GetSize() == 0)
		return NULL;
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println(F("donavigation"));
	
#endif
	CMenu* pActiveMenu = static_cast<CMenu*>(this);
	if (this->getVisualState() == SubMenu && this->getActiveSubMenu()) {
		pActiveMenu = this->getActiveSubMenu();
	}
	size_t idx = pActiveMenu->getcurrentidx();
	CMenuItem* pCurrent = pActiveMenu->getcurrent();

	if (pCurrent)
		pCurrent->onPreload();
	if (cmd == prev && idx == 0)
		cmd = back;
	if (cmd == prev && this->getVisualState()==Content)
		cmd = back;
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println(F("switch"));
	DBG_OUTPUT_PORT.println(cmd);
#endif
	switch (cmd) {
			case first:
				idx = 0;
				pActiveMenu->setcurrentidx(idx);
				break;
			case last:
				idx = pActiveMenu->getitems().GetSize() - 1;
				pActiveMenu->setcurrentidx(idx);
				break;
			case next:
				{
#ifdef  MENU_DEBUG
				DBG_OUTPUT_PORT.println(F("action next"));
#endif
					idx++;
					if (idx >= pActiveMenu->getitems().GetSize())
						idx--;
					pActiveMenu->setcurrentidx(idx);
				}
				break;
			case prev:
				{
				
				if (idx > 0)
					idx--;
				pActiveMenu->setcurrentidx(idx);
				}
				break;
			case action:
				{
	#ifdef  MENU_DEBUG
					DBG_OUTPUT_PORT.println(F("action action"));
	#endif				
					if (pCurrent && pCurrent->isSubMenu()) {
						setActiveSubMenu(pCurrent->getsubmenu());
						setVisualState(SubMenu);
					}
					else {
						if (pCurrent) {
							this->setVisualState(Content);
							pCurrent->doAction();
						}
					}
					
				}
				break;
			case back:
				{
					if (this->getVisualState() == Content) {
						setActiveSubMenu(NULL);
						setVisualState(Main);
					}
					else if (this->getVisualState() == SubMenu && pCurrent->getParent() != this) {
	#ifdef  MENU_DEBUG
						DBG_OUTPUT_PORT.println(F("action back"));
	#endif
						CMenuItem* pParent = this->getParentItem(pCurrent->getParent());

						if (pParent && pParent->getParent() == this) {
							setActiveSubMenu(NULL);
							setVisualState(Main);
	#ifdef  MENU_DEBUG
							DBG_OUTPUT_PORT.println(F("main"));
	#endif
						}
						else if (pParent && pParent->getParent()) {
							setActiveSubMenu(pParent->getParent());
							setVisualState(SubMenu);

						}

					}
				}
				break;
			default:
				break;
	}
	this->bredrawRequired = true;
	return this->getcurrent();
}
CMenu::CMenu():currentidx(0) {
	
	vs_state = Main;

}
CMenuItem*  CMenu::AddItem(String& text) {
	CMenuItem* it = new CMenuItem(this, text);
	return AddItem(it);
	 

}
CMenuItem* CMenu::AddItem(CMenuItem* pItem) {
	 items.Add(pItem);
	 return pItem;
}
CMenuItem* CMenu::getcurrent() {
	if ((getitems().GetSize() == 0 )|| (getcurrentidx() <0) || (getcurrentidx() >= getitems().GetSize()))
	      return  NULL; 

	return (CMenuItem * )getitems().GetAt(getcurrentidx());
};
CMenuItem* CMenu::getParentItem(CMenu* pMenu) {

	for (int i = 0;i < this->getitems().GetSize();i++) {
		CMenuItem* p = this->getitems().GetAt(i);
		if (p->getsubmenu() == pMenu) {

			return p;
		}
		if (p->isSubMenu())
			p = p->getsubmenu()->getParentItem(pMenu);
		if (p)
			return p;
		
	}
	return NULL;
}
const MenuItems& CMenu::getActiveItems() {
	//to do sub menu
	return items;
}
void CMenu::redraw(CMenuDisplayAdapter* pAdapter) {
	if (!pAdapter)
		return;
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println("Menu redraw");
	//DBG_OUTPUT_PORT.println((long)this);
	DBG_OUTPUT_PORT.print("Menu current:");
	DBG_OUTPUT_PORT.println(getcurrentidx());

	DBG_OUTPUT_PORT.println(pAdapter->getcurrentpage());
#endif 
	
	
	drawItems(getActiveItems(), pAdapter);

	
	
	
}
void  CMenu::drawItems(const MenuItems& its, CMenuDisplayAdapter* pAdapter) {
	if (pAdapter->getcurrentpage() != pAdapter->getpage(getcurrentidx()))
		pAdapter->setpage(getcurrentidx());
	for (size_t i = pAdapter->getfirstindex();(i < items.GetSize() && i < pAdapter->getmaxlines());i++) {
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.print("Menu draw item:");
		DBG_OUTPUT_PORT.println(i);
#endif 
		String text;
		if (i == getcurrentidx()) {
			text = ">>" + items.GetAt(i)->getname();
		}
		else {
			text = "   " + items.GetAt(i)->getname();
		}
		pAdapter->drawline(i, text);
		if (i == getcurrentidx()) {
			items.GetAt(i)->drawPreview(*pAdapter);
		}
	}
}
CMenuItem::CMenuItem(CMenu* pParent){
	this->pParent = pParent;
	this->pSubMenu = NULL;
	this->pCtl = NULL;
}
CMenuItem::CMenuItem(CMenu* pParent,String& text):CMenuItem(pParent){
	this->name = text;
	
}
void CMenuItem::clearsubmenu() {
	if (this->getsubmenu()) {
		for (int i = 0;i < this->getsubmenu()->getitems().GetSize();i++) {
			delete this->getsubmenu()->getitems().GetAt(i);
		}
		this->getsubmenu()->getitems().SetSize(0);
	}
}
void CMenuItem::doAction() {

}
void CMenuItem::drawContent(CMenuDisplayAdapter& adapter) {
	String s = "NONE";
	if (this->getcontroller()) {
		s=this->getcontroller()->serializestate();
	}
	adapter.drawtext(10, 10, s);
 }
void CMenuItem::drawPreview(CMenuDisplayAdapter& adapter) {

 }

////rf
CTopRFMenuItem::CTopRFMenuItem(CMenu* pParent) :CMenuItem(pParent) {

};
CTopRFMenuItem::CTopRFMenuItem(CMenu* pParent, String& text) :CMenuItem(pParent, text) {
};

void CTopRFMenuItem::onPreload() {
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.print("onPreload");
#endif 
	if (!this->getsubmenu()) {
		this->setsubmenu(new CMenu());
	}
	this->clearsubmenu();

	CBaseController* pBCtl = Controllers::getInstance()->GetByName(_instance->getrfname());
	if (pBCtl) {
		RFController* pRF = static_cast<RFController*>(pBCtl);
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.print("getpersistdata");
#endif 
		CSimpleArray< RFData>& dt = pRF->getpersistdata();
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.print("after getpersistdata");
#endif 
		for (int i = 0;i < dt.GetSize();i++) {

			String s = dt.GetAt(i).name;
			CActionRFMenuItem* prf = new CActionRFMenuItem(this->getsubmenu(), s);
			prf->setRF(dt.GetAt(i));
#ifdef  MENU_DEBUG
			DBG_OUTPUT_PORT.print(s);
#endif 
			this->getsubmenu()->AddItem(prf);
		}
	}
	else {
		DBG_OUTPUT_PORT.print("can't find RF controller");
	}

}
void CActionRFMenuItem::drawContent(CMenuDisplayAdapter& adapter) {
	String txt="Sending";
	adapter.drawtext(20, 20, txt);
}
void CActionRFMenuItem::doAction() {
	CBaseController* pBCtl = Controllers::getInstance()->GetByName(_instance->getrfname());
	if (pBCtl) {
		RFController* pRF = static_cast<RFController*>(pBCtl);
		pRF->send(this->getRF());
	}
	else {
		
	}
}


CMenuDisplayAdapterSSD1306::CMenuDisplayAdapterSSD1306() {
	pdisplay = NULL;
	setlineheight( 8);
}
CMenuDisplayAdapterSSD1306::~CMenuDisplayAdapterSSD1306() {
	if (pdisplay)
		delete pdisplay;
}
void CMenuDisplayAdapterSSD1306::setup(int adr, int pinSda, int pinScl) {
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println("1306 setup");
	DBG_OUTPUT_PORT.println(adr);
	DBG_OUTPUT_PORT.println(pinSda);
	DBG_OUTPUT_PORT.println(pinScl);
#endif
	pdisplay = new SSD1306(adr, pinSda, pinScl);
	pdisplay->init();
}
int  CMenuDisplayAdapterSSD1306::drawline(size_t idx, const String& text) {
	if (idx<firtsindex || idx>(firtsindex + getmaxlines()))
		return -1;
	size_t  drawindex = idx - firtsindex;
	pdisplay->drawString(0, drawindex*getlineheight(), text);
};
void  CMenuDisplayAdapterSSD1306::drawtext(size_t x, size_t y, const String& text) {
	pdisplay->drawString(x, y, text);
};
void CMenuDisplayAdapterSSD1306::clear() {
	pdisplay->clear();
}
void CMenuDisplayAdapterSSD1306::update() {
	pdisplay->display();
}