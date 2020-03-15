#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "RelayBlinkController.h"

#ifndef DISABLE_RELAYBLINK
REGISTER_CONTROLLER_FACTORY(RelayBlinkController)
const size_t bufferSize = JSON_OBJECT_SIZE(20);
#endif
RelayBlinkController::RelayBlinkController() {

	
	this->numchannels = 1;
	memset(mask, 0, sizeof(long)*MAX_RELAY_CHANNELS);
	memset(masklen, 0, sizeof(uint)*MAX_RELAY_CHANNELS);
	memset(duration, 0, sizeof(uint)*MAX_RELAY_CHANNELS);
	memset(nextms, 0, sizeof(long)*MAX_RELAY_CHANNELS);
	memset(position, 0, sizeof(int)*MAX_RELAY_CHANNELS);
	memset(pins, 0, sizeof(uint)*MAX_RELAY_CHANNELS);

}

void RelayBlinkController::loadconfig(JsonObject& json) {
	//RelayController::loadconfig(json);
	RelayBlink::loadconfig(json);
	isinvert = json["isinvert"];
	//mask = json["mask"];
	//masklen = json["masklen"];
	//duration = json["duration"];
	JsonArray arr = json["pins"].as<JsonArray>();
	this->numchannels = constrain(arr.size(), 0, MAX_RELAY_CHANNELS);
	for (int i = 0; i < this->numchannels; i++) {
			this->pins[i] = arr[i];
		}
	JsonArray arr_mask = json["mask"].as<JsonArray>();
	JsonArray arr_masklen = json["masklen"].as<JsonArray>();
	JsonArray arr_duration = json["duration"].as<JsonArray>();
	for (int i = 0; i < arr_mask.size(); i++) {
		this->mask[i] = arr_mask[i];
	}
	for (int i = 0; i < arr_masklen.size(); i++) {
		this->masklen[i] = arr_masklen[i];
	}
	for (int i = 0; i < arr_duration.size(); i++) {
		this->duration[i] = arr_duration[i];
	}
}
void RelayBlinkController::getdefaultconfig(JsonObject& json) {
	//RelayController::getdefaultconfig(json);
	RelayBlink::getdefaultconfig(json);
	json["mask"] = mask;
	json["masklen"] = masklen;
	json[FPSTR(szservice)] = "RelayBlinkController";
	json["duration"] = "duration";

}
String  RelayBlinkController::serializestate() {
#ifndef DISABLE_RELAYBLINK
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
#endif
	String json;
	json.reserve(128);
#ifndef DISABLE_RELAYBLINK
	serializeJson(root, json);
#endif
	return json;
}
bool  RelayBlinkController::deserializestate(String jsonstate, CmdSource src) {
#ifndef DISABLE_RELAYBLINK
	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	RelayBlinkState newState;
	//newState.isOn = root[FPSTR(szisOnText)];
	this->AddCommand(newState, RelayBlinkSet, src);
	//this->set_state(newState);
#endif
	return true;

}
void  RelayBlinkController::setup() {
	for (int i = 0; i < this->numchannels; i++) {
		pinMode(pins[i], OUTPUT);
		digitalWrite(pins[i], this->isinvert ? HIGH : LOW);
		
	}
	RelayBlink::setup();
}
void RelayBlinkController::set_monitor_state(uint channel, bool isOn, long mask, uint masklen, uint duration) {
	command cmd;
	//DBG_OUTPUT_PORT.println("set_monitor_state");
	DBG_OUTPUT_PORT.println(channel);
	DBG_OUTPUT_PORT.println(isOn);
	DBG_OUTPUT_PORT.println(this->numchannels);
	
	uint idx = constrain(channel, 0, MAX_RELAY_CHANNELS - 1);
	cmd.state.channelnum = idx;
	cmd.state.data.mask = mask;
	cmd.state.data.masklen = masklen;
	cmd.state.data.duration = duration;
	cmd.state.isOn[idx] = isOn;
	cmd.mode = RelayBlinkSetData;

	this->AddCommand(cmd.state, cmd.mode, srcSelf);
}
void RelayBlinkController::set_state(RelayBlinkState state) {


	RelayBlink::set_state(state);
	
	//uint idx = constrain(state.channelnum, 0, MAX_RELAY_CHANNELS - 1);
	//digitalWrite(pins[idx], (state.isOn[idx] ^ this->isinvert) ? HIGH : LOW);

//	digitalWrite(pin, (state.isOn ^ this->isinvert) ? HIGH : LOW);

}
void RelayBlinkController::run() {

	command cmd;
	while (commands.Dequeue(&cmd)) {
		//DBG_OUTPUT_PORT.println("dequeue command");
		RelayBlinkState newState = this->get_state();
		uint ch = constrain(cmd.state.channelnum, 0, MAX_RELAY_CHANNELS - 1);
		switch (cmd.mode) {
		case RelayBlinkSwitch:
			newState.isOn[ch] = !cmd.state.isOn[ch];
			this->position[ch] = 0;
			break;
		case RelayBlinkSet:
		case RelayBlinkRestore:
			newState.isOn[ch] = cmd.state.isOn[ch];
			this->position[ch] = 0;
			break;
		case RelayBlinkOn:
			newState.isOn[ch] = true;
			this->position[ch] = 0;
			break;
		case RelayBlinkOff:
			newState.isOn[ch] = false;
			break;
		case RelayBlinkSetData:
			newState.isOn[ch] = cmd.state.isOn[ch];
			this->mask[ch] = cmd.state.data.mask;
			this->masklen[ch] = cmd.state.data.masklen;
			this->duration[ch] = cmd.state.data.duration;
			this->position[ch] = 0;
			break;
		default:break;
		}
		this->set_state(newState);
	}
	for (int i = 0; i < this->numchannels; i++) {
		if (this->nextms[i] > millis() )
			continue;
		if (!this->get_state().isOn[i]) {
			//DBG_OUTPUT_PORT.println("switch blink off");
			digitalWrite(this->pins[i], (this->isinvert ? HIGH : LOW));
			continue;
		}

		bool val =  BIT_FROM_POS(this->position[i], this->mask[i]);//((0x1 << this->position[i]) & this->mask[i]);
		if (this->isinvert)
			val = !val;

		digitalWrite(this->pins[i], (val ? HIGH : LOW));
		this->position[i]++;
		if (this->position[i] >= this->masklen[i])
			this->position[i] = 0;
		this->nextms[i] = millis() + this->duration[i];
	}
	RelayBlink::run();
}