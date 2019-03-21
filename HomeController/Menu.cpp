#include "config.h"
#include "Menu.h"
#include "Controllers.h"
#include "ButtonController.h"
REGISTER_CONTROLLER_FACTORY(MenuController)

const size_t bufferSize = JSON_OBJECT_SIZE(20);


#define BUF_SIZE_LDR  JSON_OBJECT_SIZE(20)
MenuController::MenuController() {
	pmenu = new CTopLevelMenu();
	pBtnController = NULL;
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
void  MenuController::setup() {
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println("MenuController setup");
	
#endif 
	CMenuDisplayAdapter* p = new CMenuDisplayAdapterSSD1306();
	p->setup();
	pmenu->set_adapter(p);
///test
	String s = "Item 1";
	pmenu->AddItem(s);
	s = "Item 2";
	pmenu->AddItem(s);
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println(F("Setup button handlers"));
#endif
	CBaseController* pBCtl = Controllers::getInstance()->GetByName("Button");
	if (pBCtl) {
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.println(F("can't find button controller"));
#endif
		pBtnController=static_cast<ButtonController*>(pBCtl);
		MenuController* self = this;
		pBtnController->add_eventshandler_statechange(
			[self](CBaseController* p,ButtonState bs) {
			self->onButtonEvent(bs);
		}
		);
	}
	else{

	}
}
void MenuController::onButtonEvent(ButtonState bs) {
#ifdef  MENU_DEBUG
	DBG_OUTPUT_PORT.println(F("Button event"));
	DBG_OUTPUT_PORT.println(bs.idx);
#endif
}
void MenuController::run() {
	if (!pmenu)
		return;
	if (pmenu->isRedrawRequired()) {

		pmenu->redraw();
	}
}
CTopLevelMenu::CTopLevelMenu() {
	vs_state = Main;
	pActiveSubMenu = NULL;
}
CMenu::CMenu():current(0) {
	bredrawRequired = true;
	vs_state = Main;
}
const CMenuItem* CMenu::AddItem(String& text) {
	CMenuItem* it = new CMenuItem(this, text);
	AddItem(it);

}
const CMenuItem* CMenu::AddItem(CMenuItem* pItem) {
	items.Add(pItem);
}
const CMenuItem* CMenu::getcurrent() {
	if (getitems().GetSize() == 0 || current<0 || current>= getitems().GetSize())
	      return  NULL; 
	return getitems().GetAt(current);
};
const CMenuItem* CMenu::donavigation(nav cmd) {
	if (getitems().GetSize() == 0)
		return NULL;
	switch (cmd) {
	case first:
		current = 0;
		break;
	case last:
		current = getitems().GetSize()-1;
		break;
	case next:
		current++;
		if (current >= getitems().GetSize())
			current--;
		break;
	case prev:
		current--;
		if (current<0)
			current=0;
		break;
	default:
		break;
	}
	return this->getcurrent();
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
	DBG_OUTPUT_PORT.print("Menu current:");
	DBG_OUTPUT_PORT.println(current);

	DBG_OUTPUT_PORT.println(pAdapter->getcurrentpage());
#endif 
	pAdapter->clear();
	
	drawItems(getActiveItems(), pAdapter);

	pAdapter->update();
	this->bredrawRequired = false;
	
}
void  CMenu::drawItems(const MenuItems& its, CMenuDisplayAdapter* pAdapter) {
	if (pAdapter->getcurrentpage() != pAdapter->getpage(current))
		pAdapter->setpage(current);
	for (size_t i = pAdapter->getfirstindex();(i < items.GetSize() && i < pAdapter->getmaxlines());i++) {
#ifdef  MENU_DEBUG
		DBG_OUTPUT_PORT.print("Menu draw item:");
		DBG_OUTPUT_PORT.println(i);
#endif 
		String text;
		if (i == current) {
			text = ">>" + items.GetAt(i)->getname();
		}
		else {
			text = "  " + items.GetAt(i)->getname();
		}
		pAdapter->drawline(i, text);
		if (i == current) {
			items.GetAt(i)->drawPreview(*pAdapter);
		}
	}
}
CMenuItem::CMenuItem(CMenu* pParent){
	this->pParent = pParent;
}
CMenuItem::CMenuItem(CMenu* pParent,String& text):CMenuItem(pParent){
	this->name = text;
	
}
void CMenuItem::drawContent(CMenuDisplayAdapter& adapter) {

 }
void CMenuItem::drawPreview(CMenuDisplayAdapter& adapter) {

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