#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "RelayController.h"

REGISTER_CONTROLLER(RelayController)
REGISTER_CONTROLLER_FACTORY(RelayController)

const size_t bufferSize = JSON_OBJECT_SIZE(20);

RelayController::RelayController() {
	this->isinvert = false;
	this->pin = 0;
}
String  RelayController::serializestate() {
	
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isOn"] = this->get_state().isOn;

	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  RelayController::deserializestate(String jsonstate, CmdSource src) {
	
	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	RelayState newState;
	newState.isOn= root["isOn"];
	this->AddCommand(newState, Set, src);
	//this->set_state(newState);
	return true;
	
}
void RelayController::loadconfig(JsonObject& json) {
	Relay::loadconfig(json);
	pin= json["pin"];
	isinvert= json["isinvert"];
}
void RelayController::getdefaultconfig(JsonObject& json) {
	json["pin"]= pin;
	json["isinvert"]= isinvert;
	json["service"] = "RelayController";
	json["name"] = "Relay";
	
	Relay::getdefaultconfig(json);
}
void  RelayController::setup() {
	pinMode(pin, OUTPUT);
	digitalWrite(pin, this->isinvert?HIGH:LOW);
}

void RelayController::run() {
	command cmd;
	while (commands.Dequeue(&cmd)) {
		DBG_OUTPUT_PORT.print("Process Command ");
		RelayState newState = cmd.state;
		switch (cmd.mode) {
			case Switch:
				newState.isOn = !newState.isOn;
				DBG_OUTPUT_PORT.println("Switch");
				break;
			case Set:
			case RelayRestore:
				newState.isOn = cmd.state.isOn;
				DBG_OUTPUT_PORT.println("RelayRestore/Set");
				break;
			case RelayOn:
				newState.isOn = true;
				DBG_OUTPUT_PORT.println("RelayOn");
				break;
			case RelayOff:
				newState.isOn = false;
				DBG_OUTPUT_PORT.println("RelayOff");
			default:break;
	   }
		this->set_state(newState);
	}
	Relay::run();
}
void RelayController::set_state(RelayState state) {

	DBG_OUTPUT_PORT.print("RelayController state:");
	DBG_OUTPUT_PORT.println(state.isOn?"ON":"OFF");
	Relay::set_state(state);

	digitalWrite(pin, (state.isOn ^ this->isinvert)?HIGH:LOW);

}

bool RelayController::onpublishmqtt(String& endkey, String& payload) {
	endkey = "Status";
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
void RelayController::onmqqtmessage(String topic, String payload) {
	DBG_OUTPUT_PORT.println("RelayController MQT");
	DBG_OUTPUT_PORT.print("topic :");
	DBG_OUTPUT_PORT.println(topic);
	DBG_OUTPUT_PORT.print("payload :");
	DBG_OUTPUT_PORT.println(payload);
	command setcmd;
	if (topic == "Set") {
		command setcmd;
		setcmd.mode = Set;
		setcmd.state.isOn = payload == "1";
		this->AddCommand(setcmd.state, setcmd.mode, srcMQTT);
	}
	//this->AddCommand(setcmd.state, setcmd.mode);
}