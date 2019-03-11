#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "RelayBlinkController.h"

//REGISTER_CONTROLLER(RelayBlinkController)
const size_t bufferSize = JSON_OBJECT_SIZE(20);

RelayBlinkController::RelayBlinkController() {
	this->mask = 0b10;
	this->masklen = 2;
	this->duration = 100;
	this->nextms = 0;
	this->position = 0;

}

void RelayBlinkController::loadconfig(JsonObject& json) {
	RelayController::loadconfig(json);
	mask = json["mask"];
	masklen = json["masklen"];
	duration = json["duration"];
}
void RelayBlinkController::getdefaultconfig(JsonObject& json) {
	RelayController::getdefaultconfig(json);
	json["mask"] = mask;
	json["masklen"] = masklen;
	json["service"] = "RelayBlinkController";
	json["duration"] = "duration";

}
void RelayBlinkController::set_state(RelayState state) {


	Relay::set_state(state);

//	digitalWrite(pin, (state.isOn ^ this->isinvert) ? HIGH : LOW);

}
void RelayBlinkController::run() {
	if (nextms > millis() || !this->get_state().isOn)
		return;
	bool val = ((0x1 << position) & mask);
	digitalWrite(pin, (this->isinvert ^ (val ? HIGH : LOW)));
	position++;
	if (position > masklen)
		position = 0;
	nextms = millis() + duration;
}