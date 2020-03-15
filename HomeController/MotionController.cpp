#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "MotionController.h"


#ifndef DISABLE_MOTION
REGISTER_CONTROLLER_FACTORY(MotionController)
#endif

const size_t bufferSize = JSON_OBJECT_SIZE(20);

MotionController::MotionController() {
	
	this->pin = 0;
	this->autoreset = 0;
	this->nextreset = 0;
#ifdef	ENABLE_NATIVE_HAP
	this->ishap=true;
	this->hapservice=NULL;
#endif
}
String  MotionController::serializestate() {
	
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;

	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  MotionController::deserializestate(String jsonstate, CmdSource src) {
	
	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	MotionState newState;
	newState.isOn= root[FPSTR(szisOnText)];
	this->AddCommand(newState, MotionSet, src);
	//this->set_state(newState);
	return true;
	
}
void MotionController::loadconfig(JsonObject& json) {
	Motion::loadconfig(json);
	pin= json[FPSTR(szPinText)];
	loadif(autoreset,json,"autoreset");
}
void MotionController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)]= pin;
	
	json[FPSTR(szservice)] = "MotionController";
	json[FPSTR(szname)] = "Motion";
	json["ishap"] = ishap;
	json["autoreset"] = 0;
	Motion::getdefaultconfig(json);
}
void  MotionController::setup() {
	if(pin > 0)
		pinMode(pin, INPUT);
	
}

void MotionController::run() {
	command cmd;
	if (this->commands.GetSize() == 0 && this->get_state().isOn && pin>0) {
		command newcmd;
		bool motion = digitalRead(pin);
		newcmd.state = this->get_state();
		newcmd.mode = MotionSet;
		if (motion != newcmd.state.isTriggered) {
			newcmd.state.tmTrigger = millis();
			newcmd.state.isTriggered = motion;// analogRead(pin);
			this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
			DBG_OUTPUT_PORT.println(String("MotionController detected ")+ String((motion?"ON":"OFF")));
		}
	}
	if (this->commands.GetSize() && autoreset > 0 && nextreset < millis() && this->get_state().isOn  && this->get_state().isTriggered)
	{
		command newcmd;
		newcmd.state = this->get_state();
		newcmd.mode = MotionSet;
		newcmd.state.isTriggered = false;
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
		DBG_OUTPUT_PORT.println("MotionController autoreset ");
	}
	while (commands.Dequeue(&cmd)) {
		MotionState newState = cmd.state;
		switch (cmd.mode) {
			case MotionSwitch:
				newState.isOn = !newState.isOn;
				break;
			case MotionSet:
				newState.isTriggered = cmd.state.isTriggered;
				newState.tmTrigger = cmd.state.tmTrigger;
				if (autoreset > 0)
					nextreset = millis() + autoreset;
			case MotionRestore:
				newState.isOn = cmd.state.isOn;
				break;
			case MotionOn:
				newState.isOn = true;
				break;
			case MotionOff:
				newState.isOn = false;
			default:break;
	   }
		this->set_state(newState);
	}
	Motion::run();
}
void MotionController::set_state(MotionState state) {

	Motion::set_state(state);

	

}

bool MotionController::onpublishmqtt(String& endkey, String& payload) {
	endkey = szStatusText;
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
void MotionController::onmqqtmessage(String topic, String payload) {

	command setcmd;
	if (topic == "Set") {
		command setcmd;
		setcmd.mode = MotionSet;
		setcmd.state.isOn = payload == "1";
		this->AddCommand(setcmd.state, setcmd.mode, srcMQTT);
	}
	//this->AddCommand(setcmd.state, setcmd.mode);
}
#ifdef	ENABLE_NATIVE_HAP

void MotionController::setup_hap_service(){
	DBG_OUTPUT_PORT.println("MotionController::setup_hap_service()");
	if(!ishap)
		return;
	if(this->accessory_type>1){
		DBG_OUTPUT_PORT.println("MotionController adding as new accessory");
		this->hapservice= hap_add_motion_service_as_accessory(this->accessory_type,this->get_name(), MotionController::hap_callback,this);
		
	}
	else{

		this->hapservice= hap_add_motion_service(this->get_name(),MotionController::hap_callback,this);
	}

}
void MotionController::notify_hap(){
	if(this->ishap && this->hapservice){
		DBG_OUTPUT_PORT.println("MotionController::notify_hap");
		homekit_characteristic_t * ch= homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_MOTION_DETECTED);
		if(ch){

			if(ch->value.bool_value!=this->get_state().isTriggered){
				ch->value.bool_value=this->get_state().isTriggered;
			  homekit_characteristic_notify(ch,ch->value);
			}
		}
	}
}
void MotionController::hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){
	DBG_OUTPUT_PORT.println("RelayController::hap_callback");
	DBG_OUTPUT_PORT.println(value.bool_value);
	if(context){
		MotionController* ctl= (MotionController*)context;
		MotionState newState=ctl->get_state();
		MotionCMD cmd = MotionSet;
		newState.isOn=value.bool_value;
		ctl->AddCommand(newState, cmd, srcHAP);
	}
}
#endif

