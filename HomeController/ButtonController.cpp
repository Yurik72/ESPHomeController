#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#include "ButtonController.h"

const size_t bufferSize = JSON_OBJECT_SIZE(5);

ButtonController::ButtonController() {
	this->pin = 0;
	this->btnhistory = 0;
	this->btnpresstime = 0;
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
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
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
	pin = json["pin"];
}

void ButtonController::getdefaultconfig(JsonObject& json) {
	json["pin"] = pin;
	json["service"] = "ButtonController";
	json["name"] = "Button";
	Button::getdefaultconfig(json);
}

void  ButtonController::setup() {
	pinMode(pin, INPUT);
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
	this->update();
	ButtonState st;
	if (this->is_down() != this->get_state().isDown) {
		st.isDown = !this->get_state().isDown;
		this->AddCommand(st, SetBtn, srcSelf);
	}
	if (this->is_pressed() != this->get_state().isPressed) {
		st.isPressed = !this->get_state().isPressed;
		this->AddCommand(st, SetBtn, srcSelf);
	}
}
void ButtonController::update() {
	this->addhistory(digitalRead(pin)==HIGH);
}
void ButtonController::addhistory(bool bit) {
	this->btnhistory = (this->btnhistory << 1) | bit;
}
bool  ButtonController::is_down() {
	return ((this->btnhistory & ADJUST_MASK) == DOWN_MASK);
}
bool  ButtonController::is_pressed() {
	return ((this->btnhistory & ADJUST_MASK) == PRESSED_MASK);
}