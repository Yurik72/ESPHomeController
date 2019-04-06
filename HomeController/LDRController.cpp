#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#include "LDRController.h"


//REGISTER_CONTROLLER(LDRController)
REGISTER_CONTROLLER_FACTORY(LDRController)

const size_t bufferSize = JSON_OBJECT_SIZE(20);


#define BUF_SIZE_LDR  JSON_OBJECT_SIZE(20)
LDRController::LDRController() {
	this->pin = 0;
	this->cvalmin = 0.0;
	this->cvalmax = MAX_LDRVAL;
}
String  LDRController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	root["ldrValue"] = this->get_state().ldrValue;

	if (cvalmin != cvalmax) {
		float cval = map_i_f(this->get_state().ldrValue, 0, MAX_LDRVAL, cvalmin, cvalmax);
		root["cValue"] = cval;
		if (cfmt.length()) {
			char buff[30];
			snprintf(buff, sizeof(buff), cfmt.c_str(), cval);
			root["csValue"] = buff;
		}
	}
	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  LDRController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	LDRState newState;
	newState.isOn = root[FPSTR(szisOnText)];
	newState.ldrValue = root["ldrValue"];
	
	this->AddCommand(newState, Measure, srcSelf);
	//this->set_state(newState);
	return true;

}
void LDRController::loadconfig(JsonObject& json) {

	pin = json[szPinText];
	cvalmin= json["cvalmin"].as<float>();
	cvalmax = json["cvalmax"].as<float>();
	cfmt = json["cfmt"].as<String>();

}
void LDRController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)] = pin;
	json["service"] = "LDRController";
	json["name"] = "LDR";
	json["cvalmin"]= cvalmin;
	json["cvalmax"]= cvalmax;
	json["cfmt"] = cfmt;
	LDR::getdefaultconfig(json);
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
	LDR::run();
}
void LDRController::set_state(LDRState state) {

	LDR::set_state(state);
	
}

bool LDRController::onpublishmqtt(String& endkey, String& payload) {
	endkey = szStatusText;
	payload = String(this->get_state().ldrValue);
	return true;
}