#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "ThingSpeakClient.h"



//REGISTER_CONTROLLER(BME280Controller)
#ifndef DISABLE_THINGSPEAK
REGISTER_CONTROLLER_FACTORY(ThingSpeakController)
#endif
const char* thingspeakserver = "api.thingspeak.com";
//BME280ControllerFactory* ff = new BME280ControllerFactory();

const size_t bufferSize = JSON_OBJECT_SIZE(20);

ThingSpeakController::ThingSpeakController() {

	
}
String  ThingSpeakController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	for (int i = 0; i < MAX_CHANNELS; i++) {
		root[String(i)] = this->get_state().data[i];
	}

	String json;

	serializeJson(root, json);

	return json;
}
bool  ThingSpeakController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.println(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	ThingSpeakState newState;
	newState.isOn = root[FPSTR(szisOnText)];
	

	
	//this->set_state(newState);
	return true;

}
void ThingSpeakController::loadconfig(JsonObject& json) {
	ThingSpeak::loadconfig(json);
	apiKey = json["apiKey"].as<String>();
	JsonArray arr = json["value"].as<JsonArray>();
	for (int i = 0; i < arr.size() && i< MAX_CHANNELS; i++) {
		chanelusage[i] = arr[i].as<bool>();
	}
}
void ThingSpeakController::getdefaultconfig(JsonObject& json) {

	json[FPSTR(szservice)] = "ThingSpeakController";
	json[FPSTR(szname)] = "ThingSpeak";

	ThingSpeak::getdefaultconfig(json);
}
void  ThingSpeakController::setup() {
	ThingSpeak::setup();
}

void ThingSpeakController::run() {
	DBG_OUTPUT_PORT.println("ThingSpeakController::run");
	if (this->commands.GetSize() == 0) {
		command newcmd;
		newcmd.mode = TsSend;
		this->real_send();


		//this->commands.Add(newcmd);
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
	command cmd;

	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		ThingSpeakState newState = cmd.state;

		this->set_state(newState);
	}
	ThingSpeak::run();
}
void ThingSpeakController::set_state(ThingSpeakState state) {


	ThingSpeak::set_state(state);

	//	digitalWrite(pin, (state.isOn ^ this->isinvert) ? HIGH : LOW);

}
void ThingSpeakController::real_send() {
	
	WiFiClient client;
	if (WiFi.status() == WL_CONNECTED && client.connect(thingspeakserver, 80))  //   "184.106.153.149" or api.thingspeak.com
	{
		String postStr = apiKey;
		for (int i = 0; i < MAX_CHANNELS; i++) {
			if (chanelusage[i]) {
				postStr += "&field" + String(i + 1) + "=";
				postStr += String(this->get_state().data[i]);
			}
		}

		postStr += "\r\n\r\n";
		DBG_OUTPUT_PORT.println(String("Thing speak send:") + postStr);

		client.print("POST /update HTTP/1.1\n");
		client.print("Host: api.thingspeak.com\n");
		client.print("Connection: close\n");
		client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
		client.print("Content-Type: application/x-www-form-urlencoded\n");
		client.print("Content-Length: ");
		client.print(postStr.length());
		client.print("\n\n");
		client.print(postStr);
		int startWaitForResponseAt = millis();

		while (client.available() == 0 && millis() - startWaitForResponseAt < 5000)  //new thingspeak rules
		{
			delay(100);
		}
		if (client.available() != 0) {

		}
		else {
			DBG_OUTPUT_PORT.println("Data is send but without response");

		}


		
	}
	client.stop();
}