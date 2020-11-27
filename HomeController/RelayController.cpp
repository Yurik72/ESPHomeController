#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "RelayController.h"

//REGISTER_CONTROLLER(RelayController)
#ifndef DISABLE_RELAY
REGISTER_CONTROLLER_FACTORY(RelayController)
#endif

const size_t bufferSize = JSON_OBJECT_SIZE(20);

RelayController::RelayController() {
	this->isinvert = false;
	this->pin = 0;
	this->ispower_on = true;
#ifdef	ENABLE_NATIVE_HAP
	this->ishap=true;
	this->hapservice=NULL;
#endif
}
String  RelayController::serializestate() {
	
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;

	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  RelayController::deserializestate(String jsonstate, CmdSource src) {
	
	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	RelayState newState;
	newState.isOn= root[FPSTR(szisOnText)];
	this->AddCommand(newState, Set, src);
	//this->set_state(newState);
	return true;
	
}
void RelayController::loadconfig(JsonObject& json) {
	Relay::loadconfig(json);
	pin= json[FPSTR(szPinText)];
	isinvert= json["isinvert"];
	loadif(ispower_on, json, "poweron");
	
}
void RelayController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)]= pin;
	json["isinvert"]= isinvert;
	json[FPSTR(szservice)] = "RelayController";
	json[FPSTR(szname)] = "Relay";
	json["ishap"] = ishap;
	Relay::getdefaultconfig(json);
}
void  RelayController::setup() {
	pinMode(pin, OUTPUT);
	digitalWrite(pin, this->isinvert?HIGH:LOW);
}
void RelayController::set_power_on() {
	if (ispower_on)
	{
		Relay::set_power_on();
		this->run();
	}

}
void RelayController::run() {
	command cmd;
	while (commands.Dequeue(&cmd)) {
		//DBG_OUTPUT_PORT.print("Process Command ");
		RelayState newState = cmd.state;
		switch (cmd.mode) {
			case Switch:
				newState.isOn = !newState.isOn;
				//DBG_OUTPUT_PORT.println("Switch");
				break;
			case Set:
			case RelayRestore:
				newState.isOn = cmd.state.isOn;
				//DBG_OUTPUT_PORT.println("RelayRestore/Set");
				break;
			case RelayOn:
				newState.isOn = true;
				//DBG_OUTPUT_PORT.println("RelayOn");
				break;
			case RelayOff:
				newState.isOn = false;
				//DBG_OUTPUT_PORT.println("RelayOff");
			default:break;
	   }
		this->set_state(newState);
	}
	Relay::run();
}
void RelayController::set_state(RelayState state) {

	//DBG_OUTPUT_PORT.print("RelayController state:");
	//DBG_OUTPUT_PORT.println(state.isOn?"ON":"OFF");
	Relay::set_state(state);

	digitalWrite(pin, (state.isOn ^ this->isinvert)?HIGH:LOW);
	if (this->repch >=0) {
		//DBG_OUTPUT_PORT.println("Report to channel");
		if (state.isOn )
			this->report_monitor_on(this->repch);
		else
			this->report_monitor_off(this->repch);
	}
}

bool RelayController::onpublishmqtt(String& endkey, String& payload) {
	endkey = szStatusText;
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
void RelayController::onmqqtmessage(String topic, String payload) {
#ifdef MQTT_DEBUG
	DBG_OUTPUT_PORT.println("RelayController MQT");
	DBG_OUTPUT_PORT.print("topic :");
	DBG_OUTPUT_PORT.println(topic);
	DBG_OUTPUT_PORT.print("payload :");
	DBG_OUTPUT_PORT.println(payload);
#endif
	command setcmd;
	if (topic == "Set") {
		command setcmd;
		setcmd.mode = Set;
		setcmd.state.isOn = payload == "1";
		this->AddCommand(setcmd.state, setcmd.mode, srcMQTT);
	}
	//this->AddCommand(setcmd.state, setcmd.mode);
}
#ifdef	ENABLE_NATIVE_HAP

void RelayController::setup_hap_service(){
	DBG_OUTPUT_PORT.println("RelayController::setup_hap_service()");
	if(!ishap)
		return;
	if(this->accessory_type>1){
		DBG_OUTPUT_PORT.println("adding as new accessory");
		this->hapservice=hap_add_lightbulb_service_as_accessory(this->accessory_type,this->get_name(),RelayController::hap_callback,this);
		DBG_OUTPUT_PORT.println((int)this->hapservice);
	}
	else{

		this->hapservice=hap_add_lightbulb_service(this->get_name(),RelayController::hap_callback,this);
	}

}
void RelayController::notify_hap(){
	if(this->ishap && this->hapservice){
		//DBG_OUTPUT_PORT.println("RelayController::notify_hap");
		homekit_characteristic_t * ch= homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_ON);
		if(ch){

			if(ch->value.bool_value!=this->get_state().isOn){
				ch->value.bool_value=this->get_state().isOn;
			  homekit_characteristic_notify(ch,ch->value);
			}
		}
	}
}
void RelayController::hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){
	//DBG_OUTPUT_PORT.println("RelayController::hap_callback");
	//DBG_OUTPUT_PORT.println(value.bool_value);
	if(context){
		RelayController* ctl= (RelayController*)context;
		RelayState newState=ctl->get_state();
		RelayCMD cmd = Set;
		newState.isOn=value.bool_value;
		ctl->AddCommand(newState, cmd, srcHAP);
	}
}
#endif

