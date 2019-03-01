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
	this->pinsend = 0;
	this->pSwitch = NULL;
	this->store_recdata = true;
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
	pinsend = json["sendpin"];
}
void RFController::getdefaultconfig(JsonObject& json) {
	json["pin"] = pin;
	json["pinsend"] = pinsend;
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
	bool savepersist = this->store_recdata;

	if (this->pSwitch->available()) {
		command newcmd;
		newcmd.mode = OnReceive;
		newcmd.state.rftoken = this->pSwitch->getReceivedValue();
		newcmd.state.rfprotocol = this->pSwitch->getReceivedProtocol();
		newcmd.state.rfdatalen = this->pSwitch->getReceivedBitlength();
		newcmd.state.rfdelay = this->pSwitch->getReceivedDelay();
		newcmd.state.timetick = millis();
		DBG_OUTPUT_PORT.print("RFController receive:");
		DBG_OUTPUT_PORT.println(newcmd.state.rftoken);
		
		//Serial.println("received");
		//output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
		this->pSwitch->resetAvailable();
		savepersist &= true;
	}
	command cmd;
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		if (cmd.mode == Send) {
			this->rfsend(cmd.state);
		}
		if (cmd.mode == SaveReceive) {
			this->savepersist(cmd.state);
			continue; //state not changed
		}
		this->set_state(cmd.state);
	}
	if (savepersist) {  ///will proceed next cycle
		this->AddCommand(cmd.state, SaveReceive, srcSelf);
	}

}
void RFController::savepersist(RFState psstate) {
	bool exist = false;
	for (int i = 0;i < this->persistdata.GetSize();i++) {
		if (this->persistdata.GetAt(i).token == psstate.rftoken) {
			exist = true;
			break;
		}
	}
	if (!exist) {
		RFData dt(psstate);
		this->persistdata.Add(dt);
		this->saveperisttofile();
	}

	
}
void RFController::saveperisttofile() {

}

void RFController::rfsend(RFState sendstate) {
	this->pSwitch->enableTransmit(this->pinsend);

	this->pSwitch->setPulseLength(sendstate.rfdatalen);

	// Optional set protocol (default is 1, will work for most outlets)
	this->pSwitch->setProtocol(sendstate.rfprotocol);
	this->pSwitch->send(sendstate.rftoken, sendstate.rfdatalen);
	delay(100);
	this->pSwitch->disableTransmit();
}
