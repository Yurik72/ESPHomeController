#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Utilities.h"
#include "RGBStripController.h"



const size_t bufferSize = JSON_OBJECT_SIZE(20);

RGBStripController::RGBStripController() {
	pStrip = NULL;
}
RGBStripController::~RGBStripController() {
	if (pStrip)
		delete pStrip;
}
String  RGBStripController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isOn"] = this->get_state().isOn;
	root["brightness"] = this->get_state().brightness;
	root["color"] = this->get_state().color;
	root["wxmode"] = this->get_state().wxmode;
	root["wxspeed"] = this->get_state().wxspeed;

	String json;
	serializeJson(root, json);

	return json;
}
bool  RGBStripController::deserializestate(String jsonstate) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	RGBState newState;
	newState.isOn = root["isOn"];
	newState.brightness = root["brightness"];
	newState.color = root["color"];
	newState.wxmode = root["wxmode"];
	newState.wxspeed = root["wxspeed"];
	this->set_state(newState);
	return true;

}
void RGBStripController::loadconfig(JsonObject& json) {
	pin = json["pin"];
	numleds = json["numleds"];
}
void  RGBStripController::setup() {
	pStrip =new  WS2812FX(numleds, pin, NEO_GRB + NEO_KHZ800);
	pStrip->init();

	pStrip->setBrightness(30);
	pStrip->setSpeed(1000);
	pStrip->setColor(0x007BFF);
	pStrip->setMode(FX_MODE_STATIC);
	pStrip->start();
}

void RGBStripController::run() {
	command cmd;
	pStrip->service();
	while (commands.Dequeue(&cmd)) {
		RGBState newState = this->get_state();
		switch (cmd.mode) {
		case On:
			newState.isOn = true;
			break;
		case Off:
			newState.isOn = false;
			break;
		case SetBrigthness:
			newState.brightness = cmd.state.brightness;
			break;
		case SetColor:
			newState.color = cmd.state.color;
			break;
		case SetLdrVal:
			newState.ldrValue = cmd.state.ldrValue;
			break;
		default:break;
		}
		this->set_state(newState);
	}
	CBaseController::run();
}
void RGBStripController::set_state(RGBState state) {
	RGBState oldState = this->get_state();

	if (oldState.isOn != state.isOn) {  // on/off
		if (state.isOn) {
			DBG_OUTPUT_PORT.println("Switching On");
			DBG_OUTPUT_PORT.println(state.brightness);
			pStrip->setBrightness(state.brightness);
			pStrip->start();
		}
		else {
			DBG_OUTPUT_PORT.println("Switching OFF");
			pStrip->setBrightness(0);
			pStrip->stop();
			
		}

	}
	if (state.isOn) {
		if (state.isLdr) {
			if (oldState.brightness != state.brightness || oldState.ldrValue!= state.ldrValue)
				pStrip->setBrightness(getLDRBrightness(state.brightness, state.ldrValue));
		}
		else {
			if (oldState.brightness != state.brightness)
				pStrip->setBrightness(state.brightness);
		}
		if (oldState.color != state.color)
			pStrip->setColor(REDVALUE(state.color), GREENVALUE(state.color), BLUEVALUE(state.color));

	}
	pStrip->trigger();
	CController::set_state(state);
	
}
int RGBStripController::getLDRBrightness(int brigtness,int ldrval) {
	if (ldrval < 100)
		return 196;
	return ((double)(1024 - ldrval) / 1024)*brigtness;
}
bool RGBStripController::onpublishmqtt(String& endkey, String& payload) {
	endkey = "Status";
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
void RGBStripController::onmqqtmessage(String topic, String payload) {
	//RGBState oldState = this->get_state();
	command setcmd;
	setcmd.state= this->get_state();
	if (topic.endsWith("Brightness")) {
		setcmd.mode = SetBrigthness;
		setcmd.state.brightness = payload.toInt();
	}else if(topic.endsWith("hue")) {
	}
	this->AddCommand(setcmd.state, setcmd.mode);
}