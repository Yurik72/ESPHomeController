#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#include "ServoHVController.h"


//REGISTER_CONTROLLER(LDRController)
//REGISTER_CONTROLLER_FACTORY(ServoHVController)

//Factories::registerController(FPSTR_PLATFORM("ServoHVController"), NULL, NULL);

const size_t bufferSize = JSON_OBJECT_SIZE(20);


#define BUF_SIZE_LDR  JSON_OBJECT_SIZE(20)
ServoHVController::ServoHVController() {
	this->pin = 0;
}
String  ServoHVController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isOn"] = this->get_state().isOn;
	root["ldrValue"] = this->get_state().ldrValue;
	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  ServoHVController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	ServoHVState newState;
	newState.isOn = root["isOn"];
	newState.ldrValue = root["ldrValue"];

	this->AddCommand(newState, Measure, src);
	//this->set_state(newState);
	return true;

}
void ServoHVController::loadconfig(JsonObject& json) {
	pin = json["pin"];
}
void ServoHVController::getdefaultconfig(JsonObject& json) {
	json["pin"] = pin;
	json["service"] = "LDRController";
	json["name"] = "LDR";
	Servo::getdefaultconfig(json);
}
void  ServoHVController::setup() {
	pinMode(pin, INPUT);
	//digitalWrite(pin, LOW);
}

void ServoHVController::run() {
	if (this->commands.GetSize() == 0) {
		command newcmd;
		newcmd.mode = Measure;
		newcmd.state.ldrValue = analogRead(pin);
#ifdef  LDRCONTROLLER_DEBUG
		DBG_OUTPUT_PORT.print("LDR ctl run value->");
		DBG_OUTPUT_PORT.println(newcmd.state.ldrValue);
#endif //  LDRCONTROLLER_DEBUG

		//this->commands.Add(newcmd);
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
	command cmd;
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		ServoHVState newState = cmd.state;

		this->set_state(newState);
	}
	Servo::run();
}
void ServoHVController::set_state(ServoHVState state) {

	Servo::set_state(state);

}

