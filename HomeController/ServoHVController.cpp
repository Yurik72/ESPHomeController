#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#include "Utilities.h"
#include "BaseController.h"
#include "ServoHVController.h"

#ifdef ESP32
#include "esp32-hal-ledc.h"
//#include "WS2812Driver.h"
#endif



//REGISTER_CONTROLLER(LDRController)
#ifndef ESP8266 
REGISTER_CONTROLLER_FACTORY(ServoHVController)
#endif
//Factories::registerController(FPSTR_PLATFORM("ServoHVController"), NULL, NULL);

const size_t bufferSize = JSON_OBJECT_SIZE(20);


#define BUF_SIZE_LDR  JSON_OBJECT_SIZE(20)
ServoHVController::ServoHVController() {
	this->pin = 0;
	this->pinV = 0;
	this->pinH = 0;
	this->channelV = 0;
	this->channelH = 0;
	this->delayOnms = 0;
	this->nextOff = 0;
	this->minH=20;
	this->maxH=160;
	this->minV=20;
	this->maxV=160;
}
String  ServoHVController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	root["posV"] = this->get_state().posV;
	root["posH"] = this->get_state().posH;
	String json;
	json.reserve(128);
	serializeJson(root, json);
	
	return json;
}
bool  ServoHVController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	ServoHVState newState;
	newState.isOn = root[FPSTR(szisOnText)];
	newState.posV = root["posV"];
	newState.posH = root["posH"];
	this->AddCommand(newState, ServoSet, srcSelf);
	//this->set_state(newState);
	return true;

}
void ServoHVController::loadconfig(JsonObject& json) {
	pin = json[FPSTR(szPinText)];
	pinV = json["pinV"];
	pinH = json["pinH"];
	delayOnms = json["delayOnms"];
	minH=  json["minH"];
	maxH = json["maxH"];
	minV = json["minV"];
	maxH = json["maxH"];
	
	
}
void ServoHVController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)] = pin;
	json[FPSTR(szservice)] = "ServoHVController";
	json[FPSTR(szname)] = "Servo";
	json["pinV"] = pinV;
	json["pinH"] = pinH;
	json["minH"]=minH;
	json["maxH"]=maxH;
	json["minV"]=minV;
	json["maxV"]=maxV;
	json["delayOnms"] = delayOnms;

	Servo::getdefaultconfig(json);
}
void  ServoHVController::setup() {
	pinMode(pin, OUTPUT);
	//digitalWrite(pin, LOW);
	if (this->pinV > 0) {  //setup vertical
		
#ifdef ESP32
		
		//this->channelV = get_next_available_channel();
		this->channelV = get_next_espchannel();
		ledcSetup(this->channelV, SERVO_FREQ, SERVO_BITS); // channel , 50 Hz, 
		ledcAttachPin(this->pinV, this->channelV);   // GPIO  assigned to channel 
#endif
		pinMode(this->pinV, OUTPUT);
	}
	if (this->pinH > 0) {  //setup horizontal
	
#ifdef ESP32
	
		//this->channelH = get_next_available_channel();
		this->channelH = get_next_espchannel();
		ledcSetup(this->channelH, SERVO_FREQ, SERVO_BITS); // channel , 50 Hz, 
		ledcAttachPin(this->pinH, this->channelH);   // GPIO  assigned to channel 
		pinMode(this->pinH, OUTPUT);
#endif
#ifdef SERVO_DEBUG
		DBG_OUTPUT_PORT.println(F("RelayDimController::setup()"));
		DBG_OUTPUT_PORT.print(F("Channel H:"));
		DBG_OUTPUT_PORT.println(this->channelH);
		DBG_OUTPUT_PORT.print(F("Channel V:"));
		DBG_OUTPUT_PORT.println(this->channelV);
#endif
	}
}

void ServoHVController::run() {

	command cmd;
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		ServoHVState newState = cmd.state;
		switch (cmd.mode) {
			case ServoSet:
			case ServoSetH:
			case ServoSetV:
				newState.isOn = true;
				break;
			case ServoOff:
				newState.isOn = false;
			default:break;
		}
		this->set_state(newState);
	}
	if(this->get_state().isOn && this->nextOff < millis() ) { ///  time to off
		command cmdoff;
		cmdoff.state = this->get_state();
		cmdoff.state.isOn = false;
		this->AddCommand(cmdoff.state, ServoOff, srcSelf);
	}

	Servo::run();
}
void ServoHVController::set_state(ServoHVState state) {
	ServoHVState oldState = this->get_state();
	if (oldState.isOn != state.isOn) {  // on/off
		if (state.isOn) {
			if (pin > 0)
				digitalWrite(pin, HIGH);
			this->nextOff=millis()+ this-> delayOnms;
		}
		else {
			if (pin > 0)
				digitalWrite(pin, LOW);
			
		}
		 
	}
	if (state.isOn) {
		if (oldState.posH != state.posH) {
			state.posH = constrain(state.posH, this->minH, this->maxH);
#ifdef ESP32
			
			ledcWrite(this->channelH, CALC_SERVO_PULSE(state.posH));       // sweep servo H
#else
			analogWrite(this->pinV, CALC_SERVO_PULSE(state.posV));  //TO DO , will not work on esp8266
#endif
		}
		if (oldState.posV != state.posV) {
			state.posV = constrain(state.posV, this->minV, this->maxV);
#ifdef ESP32
			ledcWrite(this->channelV, CALC_SERVO_PULSE(state.posV));       // sweep servo H
#else
			analogWrite(this->pinV, CALC_SERVO_PULSE(state.posV));  //TO DO , will not work on esp8266

#endif
		}
	}

	Servo::set_state(state);

}

