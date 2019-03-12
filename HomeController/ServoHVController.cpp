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


#define TIMER_WIDTH 8  
//REGISTER_CONTROLLER(LDRController)
REGISTER_CONTROLLER_FACTORY(ServoHVController)

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
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
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
}
void ServoHVController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)] = pin;
	json["service"] = "ServoHVController";
	json["name"] = "Servo";
	json["pinV"] = pinV;
	json["pinH"] = pinH;
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
		ledcSetup(this->channelV, 50, TIMER_WIDTH); // channel , 50 Hz, 8-bit width
		ledcAttachPin(this->pinV, this->channelV);   // GPIO  assigned to channel 
#endif
		pinMode(this->pinV, OUTPUT);
	}
	if (this->pinH > 0) {  //setup horizontal
	
#ifdef ESP32
	
		//this->channelH = get_next_available_channel();
		this->channelH = get_next_espchannel();
		ledcSetup(this->channelH, 50, TIMER_WIDTH); // channel , 50 Hz, 8-bit width
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
#ifdef ESP32
			ledcWrite(this->channelH, state.posH);       // sweep servo H
#endif
		}
		if (oldState.posV != state.posV) {
#ifdef ESP32
			ledcWrite(this->channelV, state.posV);       // sweep servo H
#endif
		}
	}

	Servo::set_state(state);

}

