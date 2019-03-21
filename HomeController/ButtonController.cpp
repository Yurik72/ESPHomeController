#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#include "ButtonController.h"

//#ifndef ESP8266   /// further solving factory
//REGISTER_CONTROLLER(ButtonController)
//#endif // !1
REGISTER_CONTROLLER_FACTORY(ButtonController)

const size_t bufferSize = JSON_OBJECT_SIZE(10);

ButtonController::ButtonController() {
	memset(pin, 0, sizeof(uint8_t)*MAX_BUTTONS); 
	memset(btnhistory, 0, sizeof(uint16_t)*MAX_BUTTONS);
	memset(btnstate, 0, sizeof(enumstate)*MAX_BUTTONS);
	
	this->btncount = 0;
	Ticker* pTicker;
	this->coreMode = Both;
}

String  ButtonController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isPressed"] = this->get_state().isPressed;

	String json;
	json.reserve(64);
	serializeJson(root, json);

	return json;
}
bool  ButtonController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	ButtonState newState;
	newState.isPressed = root["isPressed"];


//	this->AddCommand(newState, Measure, src);
	//this->set_state(newState);
	return true;

}
void ButtonController::loadconfig(JsonObject& json) {
	uint8_t mpin = json[FPSTR(szPinText)];
	JsonArray arr = json["pins"].as<JsonArray>();
	if (pin != 0) {
		this->btncount = 1;
		this->pin[0] = mpin;
	}
	else {
		this->btncount = constrain(arr.size(), 0, MAX_BUTTONS);
		for (int i = 0; i < this->btncount; i++) {
			this->pin[i] = arr[i];
		}
	}
}

void ButtonController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)] = pin;
	json["service"] = "ButtonController";
	json["name"] = "Button";
	

	Button::getdefaultconfig(json);
}

void  ButtonController::setup() {
	Button::setup();
	for(uint8_t i=0;i<btncount;i++)
		pinMode(pin[i], INPUT);
	//digitalWrite(pin, LOW);
}
void ButtonController::run() {
	command cmd;
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		this->set_state(cmd.state);
	}
}
void ButtonController::runcore() {
	for (uint8_t i = 0;i < btncount;i++) {
		this->update(i);
		ButtonState st;
		st.idx = i;
		if (this->is_down(i) && btnstate[i]!= isdown){  //this->get_state().isDown) {
			st.isDown = !this->get_state().isDown;
		    btnstate[i] = isdown;
			this->AddCommand(st, SetBtn, srcSelf);
		}
		if (this->is_pressed(i) && btnstate[i] != ispressed){//!= this->get_state(i).isPressed) {
			st.isPressed = true;
			btnstate[i] = ispressed;
			this->AddCommand(st, SetBtn, srcSelf);
		}
	}
}
void ButtonController::update(uint8_t idx) {
	this->addhistory(digitalRead(pin[idx])==HIGH,idx);
}
void ButtonController::addhistory(bool bit, uint8_t idx) {
	this->btnhistory[idx] = (this->btnhistory[idx] << 1) | bit;
}
bool  ButtonController::is_down(uint8_t idx) {
	return ((this->btnhistory[idx] & ADJUST_MASK) == DOWN_MASK);
}
bool  ButtonController::is_pressed(uint8_t idx) {
	return ((this->btnhistory[idx] & ADJUST_MASK) == PRESSED_MASK);
}