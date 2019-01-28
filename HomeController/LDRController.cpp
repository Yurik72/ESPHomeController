#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#include "LDRController.h"
const size_t bufferSize = JSON_OBJECT_SIZE(20);
String  LDRController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isOn"] = this->get_state().isOn;
	root["ldrValue"] = this->get_state().ldrValue;
	String json;
	serializeJson(root, json);

	return json;
}
bool  LDRController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	LDRState newState;
	newState.isOn = root["isOn"];
	newState.ldrValue = root["ldrValue"];
	
	this->AddCommand(newState, Measure, src);
	//this->set_state(newState);
	return true;

}
void LDRController::loadconfig(JsonObject& json) {
	pin = json["pin"];
}
void  LDRController::setup() {
	pinMode(pin, INPUT);
	//digitalWrite(pin, LOW);
}

void LDRController::run() {
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
		LDRState newState = cmd.state;

		this->set_state(newState);
	}
	CBaseController::run();
}
void LDRController::set_state(LDRState state) {

	CController::set_state(state);
	
}

bool LDRController::onpublishmqtt(String& endkey, String& payload) {
	endkey = "Status";
	payload = String(this->get_state().ldrValue);
	return true;
}