#include "config.h"
#include "Utilities.h"
#include "OledController.h"
#include "Controllers.h"



#ifndef DISABLE_OLED
REGISTER_CONTROLLER_FACTORY(OledControllerSSD1306)
#endif
const size_t bufferSize = JSON_OBJECT_SIZE(100);
String  OledController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	root[FPSTR(szbrightnessText)] = this->get_state().brightness;


	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  OledController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(F("deserialize failed: "));
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	OledState newState;
	newState.isOn = root[FPSTR(szisOnText)];
	newState.brightness = root[FPSTR(szbrightnessText)];

	this->AddCommand(newState, OledDrawText, src);
	//this->set_state(newState);
	return true;

}
void OledController::loadconfig(JsonObject& json) {
	Oled::loadconfig(json);
	
	i2caddr = json["i2caddr"];
	if (i2caddr == 0)
		i2caddr = 0x3c;
}

void OledController::setup() {

}
OledControllerSSD1306::OledControllerSSD1306() {
	pdisplay = NULL;
	
}
OledControllerSSD1306::~OledControllerSSD1306() {
	if (pdisplay)
		delete pdisplay;
}
void OledControllerSSD1306::getdefaultconfig(JsonObject& json) {
	json["i2caddr"] = i2caddr;

	json["service"] = "OledControllerSSD1306";
	json["name"] = "OLED";
	Oled::getdefaultconfig(json);
}
void OledControllerSSD1306::setup() {
	this->setup(this->i2caddr, 21, 22);
}
void OledControllerSSD1306::setup(int adr, int pinSda, int pinScl) {
#ifdef  OLED_DEBUG
	DBG_OUTPUT_PORT.println("1306 setup");
	DBG_OUTPUT_PORT.println(adr);
	DBG_OUTPUT_PORT.println(pinSda);
	DBG_OUTPUT_PORT.println(pinScl);
#endif
	pdisplay = new SSD1306(adr, pinSda, pinScl);
	pdisplay->init();
	pdisplay->clear();
	pdisplay->drawString(5, 5, "Loading...");
	pdisplay->display();

}
int  OledControllerSSD1306::drawline(size_t idx, const String& text) {
	
	//pdisplay->drawString(0, drawindex*getlineheight(), text);
};
void  OledControllerSSD1306::drawtext(size_t x, size_t y, const String& text) {
	pdisplay->drawString(x, y, text);
};
void OledControllerSSD1306::clear() {
	pdisplay->clear();
}
void OledControllerSSD1306::update() {
	pdisplay->display();
}
void OledControllerSSD1306::run() {
	command cmd;
	while (commands.Dequeue(&cmd)) {
		//DBG_OUTPUT_PORT.print("Process Command ");
		OledState newState = cmd.state;
		switch (cmd.mode) {
		case OledDrawText:
			newState.isOn = !newState.isOn;
			newState.text = cmd.state.text;
			newState.x = cmd.state.x;
			newState.y = cmd.state.y;
			newState.y = cmd.state.y;
			//DBG_OUTPUT_PORT.println("DimSwitch");
			break;
		case OledClear:
			newState.isClear = true;
			break;
		case OledSetBrigthness:
			newState.brightness = cmd.state.brightness;
			break;
		
		default:break;
		}
		this->set_state(newState);
	}
	Oled::run();
}
void OledControllerSSD1306::set_state(OledState state) {
	OledState oldState = this->get_state();
	if (state.brightness != oldState.brightness) {
		pdisplay->setBrightness(state.brightness);
	}
	
		pdisplay->setFont(ArialMT_Plain_10);
		switch (state.fontsize) {
		
		case 2:
			pdisplay->setFont(ArialMT_Plain_16);
			break;
		case 3:
			pdisplay->setFont(ArialMT_Plain_24);
			break;

		}
	
	if (state.isClear) {
		pdisplay->clear();
	}
	else {
		pdisplay->drawString(state.x, state.y, state.text);
		pdisplay->display();
	}
}