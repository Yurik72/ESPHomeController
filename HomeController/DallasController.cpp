#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "DallasController.h"



//REGISTER_CONTROLLER(BME280Controller)

REGISTER_CONTROLLER_FACTORY(DallasController)


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
	root["temp"] = this->get_state().temp;
	root["hum"] = this->get_state().hum;
	root["pres"] = this->get_state().pres;
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
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	DallasState newState;
	newState.isOn = root[FPSTR(szisOnText)];
	newState.temp = root["temp"];

	this->AddCommand(newState, BMESet, src);
	//this->set_state(newState);
	return true;

}
void DallasController::loadconfig(JsonObject& json) {
	i2caddr = json["i2caddr"];
	pin=json[FPSTR(szPinText)] ;
	
}
void DallasController::getdefaultconfig(JsonObject& json) {
	json["i2caddr"] = i2caddr;
	json[FPSTR(szPinText)] = pin;
	json["service"] = "DallasController";
	json["name"] = "BME";

	Dallas::getdefaultconfig(json);
}
void  DallasController::setup() {
	poneWire = new OneWire(this->pin);
	psensors = new DallasTemperature(poneWire);
}

void DallasController::run() {
	if (this->commands.GetSize() == 0) {
		command newcmd;
		newcmd.mode = BMEMeasure;
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

