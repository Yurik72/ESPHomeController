#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "RCSwitch.h"
#include "RFController.h"

REGISTER_CONTROLLER(RFController)
REGISTER_CONTROLLER_FACTORY(RFController)

const size_t bufferSize = JSON_OBJECT_SIZE(5);



RFController::RFController() {
	this->pin = 0;
	this->pSwitch = NULL;

}

String  RFController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isReceive"] = this->get_state().isReceive;
	root["isSend"] = this->get_state().isSend;
	root["rftoken"] = this->get_state().rftoken;
	root["timetick"] = this->get_state().timetick;
	String json;
	json.reserve(64);
	serializeJson(root, json);

	return json;
}

bool  RFController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	RFState newState;
	//newState.isPressed = root["isPressed"];


	//	this->AddCommand(newState, Measure, src);
		//this->set_state(newState);
	return true;

}
void RFController::loadconfig(JsonObject& json) {
	pin = json["pin"];
}
void RFController::getdefaultconfig(JsonObject& json) {
	json["pin"] = pin;
	json["service"] = "RFController";
	json["name"] = "RF";
	RF::getdefaultconfig(json);
}

void  RFController::setup() {
	RF::setup();
	this->pSwitch = new RCSwitch();
	this->pSwitch->enableReceive(this->pin);
	//pinMode(pin, INPUT);
	//digitalWrite(pin, LOW);
}

void RFController::run() {
	if (this->pSwitch->available()) {
		command newcmd;
		newcmd.mode = OnReceive;
		newcmd.state.rftoken = this->pSwitch->getReceivedValue();
		newcmd.state.timetick = millis();
		DBG_OUTPUT_PORT.print("RFController receive:");
		DBG_OUTPUT_PORT.println(newcmd.state.rftoken);
		//Serial.println("received");
		//output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
		this->pSwitch->resetAvailable();
	}
	command cmd;
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		this->set_state(cmd.state);
	}
}