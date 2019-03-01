#include "config.h"
#include "Menu.h"



CMenu::CMenu():current(0) {

}
const CMenuItem* CMenu::AddItem(String& text) {
	CMenuItem* it = new CMenuItem(this, text);
	items.Add(it);

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
void CMenu::redraw() {
	if (!pAdapter)
		return;
	if (pAdapter->getcurrentpage() != pAdapter->getpage(current))
		pAdapter->setpage(current);
	for (size_t i = 0;(i < (items.GetSize() + current) && i < (pAdapter->getmaxlines()));i++)
		pAdapter->drawline(i, items.GetAt(i + current)->getname());
	

	
}
CMenuItem::CMenuItem(CMenu* pParent){
	this->pParent = pParent;
}
CMenuItem::CMenuItem(CMenu* pParent,String& text):CMenuItem(pParent){
	this->name = text;
}