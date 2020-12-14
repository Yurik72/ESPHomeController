#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Utilities.h"
#include <time.h>
#if defined(ESP8266)
#include "NTPClient.h"

#endif
#include "TimeController.h"
#include "Controllers.h"
#ifdef ESP32
#include "esp_wifi.h"
#include "esp_bt_main.h"
//#include "esp_bt.h"
#endif
#ifdef	ENABLE_NATIVE_HAP
extern "C" {
#include "homeintegration.h"
}
#endif
//const char* ntpServer = "pool.ntp.org";
//const long  gmtOffset_sec = 7200;
//const int   daylightOffset_sec = -3600;
#if defined(ESP8266)
//NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
#endif


//REGISTER_CONTROLLER(TimeController)
REGISTER_CONTROLLER_FACTORY(TimeController)

const size_t bufferSize = JSON_OBJECT_SIZE(20);

TimeController::TimeController() {
	this->ntpServer = "pool.ntp.org";
	this->gmtOffset_sec = 0;
	this->daylightOffset_sec = 0;
	this->enablesleep = false;
	this->sleepinterval = 300000;
	this->sleeptype = 1;
	this->btnwakeup = 0;
	this->is_sleepstarted = false;
	this->restartinterval = 0;
	this->nextrestart = 0;
	this->numshortsleeps = 0;
	this->sleepnumber = 0;
	this->offsetshortwakeup = 0;
	this->offsetwakeup = 0;
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
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
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
	gmtOffset_sec = json[FPSTR(sztimeoffs)];
	daylightOffset_sec = json[FPSTR(szdayloffs)];
	ntpServer = json[FPSTR(szserver)].as<String>();
	loadif(enablesleep, json, FPSTR(szenablesleep));
	loadif(sleepinterval, json, FPSTR(szsleepinterval));
	loadif(btnwakeup, json, FPSTR(szbtnwakeup));
	loadif(sleeptype, json, FPSTR(szsleeptype));
	loadif(restartinterval, json, FPSTR(szrestartinterval));
	loadif(numshortsleeps, json, FPSTR(sznumshortsleeps));
	
	
	offsetwakeup = (sleepinterval / 1000 - 20) * 1000000;
	offsetshortwakeup= (sleepinterval / 1000 - 3) * 1000000;
	Controllers::getInstance()->setTimeCtl(this);
	
}
void TimeController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(sztimeoffs)]= gmtOffset_sec;
	json[FPSTR(szdayloffs)]= daylightOffset_sec;
	
	json[FPSTR(szenablesleep)] = enablesleep;
	json[FPSTR(szsleepinterval)] = sleepinterval;
	json[FPSTR(szbtnwakeup)] = btnwakeup;
	json[FPSTR(szsleeptype)] = sleeptype;
	json[FPSTR(szrestartinterval)] = restartinterval;
	json[FPSTR(sznumshortsleeps)] = numshortsleeps;
	
	json[FPSTR(szserver)]= ntpServer.c_str();


	json[FPSTR(szservice)] = "TimeController";
	json[FPSTR(szname)] = "Time";
	TimeCtl::getdefaultconfig(json);
}
void  TimeController::setup() {
	TimeCtl::setup();
}
void TimeController::setup_after_wifi() {
#if defined TIMECONTROLLER_FULL_DEBUG
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
	this->nextsleep = millis() + 300000; //5 min to give a chanse to update//this->sleepinterval;

	TimeCtl::setup_after_wifi();
}
#ifdef	ENABLE_NATIVE_HAP
void TimeController::setup_after_hap() {
	if (enablesleep) {
		DBG_OUTPUT_PORT.println("We are in sleep and will save power");
		set_wifi_save_power_middle();
	}
}
#endif
void TimeController::run() {
	
#if defined TIMECONTROLLER_FULL_DEBUG
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
#if defined TIMECONTROLLER_FULL_DEBUG
			DBG_OUTPUT_PORT.print("time by getLocalTime");
			DBG_OUTPUT_PORT.println(asctime(&timeinfo));

#endif
		}

#endif
		newcmd.state.time_withoffs = newcmd.state.time;
#ifdef ESP32

		//newcmd.state.time_withoffs+= gmtOffset_sec;
		if(timeinfo.tm_isdst)
			newcmd.state.time_withoffs += daylightOffset_sec;
#endif
		//this->commands.Add(newcmd);
		newcmd.mode = SET;
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);

#if defined TIMECONTROLLER_FULL_DEBUG
		DBG_OUTPUT_PORT.print("Time ctl run ");
		DBG_OUTPUT_PORT.println(newcmd.state.time);
		DBG_OUTPUT_PORT.print("offs flag:");
		DBG_OUTPUT_PORT.print(timeinfo.tm_isdst);
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
	check_restart();
	if (enablesleep) {
		if (this->nextsleep <= millis()) {
			this->nextsleep = millis() + this->sleepinterval;
#if defined TIMECONTROLLER_DEBUG
			 DBG_OUTPUT_PORT.println("Startint  sleep");
#endif
#ifdef ESP32
			 if (this->sleeptype == 0) {
				 esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
			 }
			 else if (this->sleeptype == 1) {
				 if (this->is_sleepstarted && !is_shortwakeup() && !WiFi.isConnected()) { //give more chanse to other services to reach internet
					 this->is_sleepstarted = false;
					 this->nextsleep = millis() + 3000;
#if defined TIMECONTROLLER_DEBUG
					 DBG_OUTPUT_PORT.println("continue wake up due to wifi disconnect");
#endif
				 }
				 else {
					 
					 esp_sleep_enable_timer_wakeup(is_shortwakeup_next()? offsetshortwakeup: offsetwakeup);// 1000000 * 30);  //30s
					 Controllers::getInstance()->set_isneedreconnectwifi(!is_shortwakeup_next());
#if defined TIMECONTROLLER_DEBUG
					 if(is_shortwakeup())
						DBG_OUTPUT_PORT.println("short wake up");
#endif
					 if (this->btnwakeup > 0)
						 esp_sleep_enable_ext0_wakeup((gpio_num_t)btnwakeup, 0);
					 this->sleepnumber++;
					 this->is_sleepstarted = true;
					 this->raise_event(SleepStart, this->sleeptype);
					 delay(20);
					 esp_light_sleep_start();
					 delay(500);
					 this->raise_event(SleepUp, this->sleeptype);
				 }
			 }
			 else if (this->sleeptype == 2) {
				 DBG_OUTPUT_PORT.println("Starting modem sleep ");
				 esp_bluedroid_disable();
				// esp_bt_controller_disable();
				 esp_wifi_stop();
			 }
#endif
			
		}
	}
	//esp_light_sleep_start();
	//esp_sleep_enable_timer_wakeup(SleepSecs * uS_TO_S_FACTOR);
	TimeCtl::run();
}
bool TimeController::is_shortwakeup() {
	if (this->numshortsleeps == 0)
		return false;
	return (this->sleepnumber % this->numshortsleeps) > 0;
}
bool TimeController::is_shortwakeup_next() {
	if (this->numshortsleeps == 0)
		return false;
	return ((this->sleepnumber+1) % this->numshortsleeps) > 0;
}
void TimeController::check_restart() {
	if (this->restartinterval > 0) {
		if (this->nextrestart == 0)
			this->nextrestart = millis() + this->restartinterval;
		if (this->nextrestart <= millis())
			isReboot = true;
	}
}
void TimeController::set_state(TimeState state) {

	TimeCtl::set_state(state);
	
}