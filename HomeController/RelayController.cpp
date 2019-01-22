#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "RelayController.h"
const size_t bufferSize = JSON_OBJECT_SIZE(20);
String  RelayController::serializestate() {
	
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isOn"] = this->get_state().isOn;

	String json;
	serializeJson(root, json);

	return json;
}
bool  RelayController::deserializestate(String jsonstate) {
	
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
	this->AddCommand(newState, Set, srcState);
	//this->set_state(newState);
	return true;
	
}
void RelayController::loadconfig(JsonObject& json) {
	pin= json["pin"];
}
void  RelayController::setup() {
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

void RelayController::run() {
	command cmd;
	while (commands.Dequeue(&cmd)) {
		RelayState newState = cmd.state;
		switch (cmd.mode) {
			case Switch:
				newState.isOn = !newState.isOn;
				break;
			case Set:
				newState.isOn = cmd.state.isOn;
				break;
			default:break;
	   }
		this->set_state(newState);
	}
	CBaseController::run();
}
void RelayController::set_state(RelayState state) {

	DBG_OUTPUT_PORT.print("RelayController state:");
	DBG_OUTPUT_PORT.println(state.isOn?"ON":"OFF");
	CController::set_state(state);

	digitalWrite(pin, state.isOn?HIGH:LOW);
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