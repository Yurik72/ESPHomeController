#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "DallasController.h"



//REGISTER_CONTROLLER(BME280Controller)
#ifndef DISABLE_DALLAS
REGISTER_CONTROLLER_FACTORY(DallasController)
#endif

//BME280ControllerFactory* ff = new BME280ControllerFactory();

const size_t bufferSize = JSON_OBJECT_SIZE(20);

DallasController::DallasController() {
	this->i2caddr = 0x77;
	
	this->psensors = NULL;
	
}
String  DallasController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	root[FPSTR(sztemp)] = this->get_state().temp;
	root[FPSTR(szhum)] = this->get_state().hum;
	root[FPSTR(szpres)] = this->get_state().pres;
	String json;

	serializeJson(root, json);

	return json;
}
bool  DallasController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
//		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}

	JsonObject root = jsonBuffer.as<JsonObject>(); // test decrease program size
	//JsonObject root = getRootObject(jsonBuffer);
	DallasState newState;
	newState.isOn = get_json_bool(root,FPSTR(szisOnText));// root[FPSTR(szisOnText)];
	newState.temp = get_json_double(root, FPSTR(sztemp));// root[FPSTR(sztemp)];

	this->AddCommand(newState, DlSet, src);
	//this->set_state(newState);
	return true;

}
void DallasController::loadconfig(JsonObject& json) {
	i2caddr = json[FPSTR(szi2caddr)];
	pin=json[FPSTR(szPinText)] ;
	
}
void DallasController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szi2caddr)] = i2caddr;
	json[FPSTR(szPinText)] = pin;
	json[FPSTR(szservice)] = "DallasController";
	json[FPSTR(szname)] = "BME";

	Dallas::getdefaultconfig(json);
}
void  DallasController::setup() {
	poneWire = new OneWire(this->pin);
	psensors = new DallasTemperature(poneWire);
}

void DallasController::run() {
	if (this->commands.GetSize() == 0) {
		command newcmd;
		newcmd.mode = DlMeasure;
		this->meassure(newcmd.state);
#ifdef  DALLASCONTROLLER_DEBUG
		DBG_OUTPUT_PORT.print("DallasController->");
		DBG_OUTPUT_PORT.print("Temperature in Celsius : ");
		DBG_OUTPUT_PORT.print(newcmd.state.temp);

#endif //  LDRCONTROLLER_DEBUG

		//this->commands.Add(newcmd);
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
	command cmd;

	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		DallasState newState = cmd.state;

		this->set_state(newState);
	}
	Dallas::run();
}


bool DallasController::onpublishmqtt(String& endkey, String& payload) {
	endkey = szStatusText;
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
void DallasController::onmqqtmessage(String topic, String payload) {

}

void DallasController::meassure(DallasState& state) {
	psensors->requestTemperatures(); // Send the command to get temperatures

	// After we got the temperatures, we can print them here.
	// We use the function ByIndex, and as an example get the temperature from the first sensor only.
	state.temp = psensors->getTempCByIndex(0);


	
}

