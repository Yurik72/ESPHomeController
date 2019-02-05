#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Utilities.h"
#include <time.h>
#if defined(ESP8266)
#include "NTPClient.h"

#endif
#include "TimeController.h"

//const char* ntpServer = "pool.ntp.org";
//const long  gmtOffset_sec = 7200;
//const int   daylightOffset_sec = -3600;
#if defined(ESP8266)
//NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
#endif
const size_t bufferSize = JSON_OBJECT_SIZE(20);

TimeController::TimeController() {

}
String  TimeController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["time"] = this->get_state().time;

	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  TimeController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	TimeState newState;
	newState.time = root["time"];
	//this->set_state(newState);
	this->AddCommand(newState, SET, src);
	return true;

}
void TimeController::loadconfig(JsonObject& json) {
	//pin = json["pin"];
	gmtOffset_sec = json["timeoffs"];
	daylightOffset_sec = json["dayloffs"];
	ntpServer = json["server"].as<String>();
}
void  TimeController::setup() {
#if defined TIMECONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("TimeController::setup");
	DBG_OUTPUT_PORT.print("gmtOffset");
	DBG_OUTPUT_PORT.println(gmtOffset_sec);
	DBG_OUTPUT_PORT.print("daylightOffset");
	DBG_OUTPUT_PORT.println(daylightOffset_sec);
#endif
#if defined(ESP8266)
	ptimeClient = new NTPClient(ntpUDP, ntpServer.c_str(), gmtOffset_sec);
	ptimeClient->begin();
	ptimeClient->update();
#else
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer.c_str());
#endif
}

void TimeController::run() {
	
#if defined TIMECONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println(" TimeController::run");
#endif
	if (this->commands.GetSize() == 0) {
		command newcmd;

#if defined(ESP8266)
		ptimeClient->update();
		newcmd.state.time = ptimeClient->getEpochTime();

#else		
		struct tm timeinfo;
		if (getLocalTime(&timeinfo)) {
			newcmd.state.time = mklocaltime(&timeinfo,this->get_gmtoffset());
#if defined TIMECONTROLLER_DEBUG
			DBG_OUTPUT_PORT.print("time by getLocalTime");
			DBG_OUTPUT_PORT.println(asctime(&timeinfo));

#endif
		}

#endif
		//this->commands.Add(newcmd);
		newcmd.mode = SET;
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);

#if defined TIMECONTROLLER_DEBUG
		DBG_OUTPUT_PORT.print("Time ctl run ");
		DBG_OUTPUT_PORT.println(newcmd.state.time);
		DBG_OUTPUT_PORT.println(getFormattedTime(newcmd.state.time));
#endif
	}
	command cmd;
	while (commands.Dequeue(&cmd)) {
		TimeState newState = cmd.state;
		switch (cmd.mode) {
		case SET:
			
			//newState.time = cmd.time;
			break;
		default:break;
		}
		//DBG_OUTPUT_PORT.print("Time processed : ");
		//DBG_OUTPUT_PORT.println(getFormattedTime(newState.time));
		this->set_state(newState);
	}
	CBaseController::run();
}
void TimeController::set_state(TimeState state) {

	CController::set_state(state);
	
}