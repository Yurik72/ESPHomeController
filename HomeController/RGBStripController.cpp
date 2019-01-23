#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Utilities.h"
#include "BaseController.h"
#include "RGBStripController.h"



const size_t bufferSize = JSON_OBJECT_SIZE(20);

RGBStripController::RGBStripController() {
	this->pStrip = NULL;
	this->mqtt_hue = 0.0;
	this->mqtt_saturation = 0.0;

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
	//this->set_state(newState);
	this->AddCommand(newState, SetRGB, srcState);
	return true;

}
void RGBStripController::loadconfig(JsonObject& json) {
	CManualStateController<RGBStripController, RGBState, RGBCMD>::loadconfig(json);
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
		case SetMode:
			newState.wxmode = cmd.state.wxmode;
			break;
		case SetRGB:
		case SetRestore:
			newState = cmd.state;
			break;
		default:break;
		}
		this->set_state(newState);
	}
	CManualStateController<RGBStripController, RGBState, RGBCMD>::run();
}
void RGBStripController::set_state(RGBState state) {
	RGBState oldState = this->get_state();
	
	if (oldState.isOn != state.isOn) {  // on/off
		if (state.isOn) {
			DBG_OUTPUT_PORT.println("Switching On");
			DBG_OUTPUT_PORT.println(state.brightness);
			pStrip->setBrightness(state.brightness);
			
			if (!pStrip->isRunning()) pStrip->start();
			
		}
		else {
			DBG_OUTPUT_PORT.println("Switching OFF");
			pStrip->setBrightness(0);
			if(pStrip->isRunning())pStrip->stop();
			
		}

	}
	if (state.isOn) {
		if (oldState.wxmode != state.wxmode) {
			DBG_OUTPUT_PORT.println("oldState.wxmode != state.wxmod");
			if (pStrip->isRunning())pStrip->stop();
			if (state.wxmode > 0) pStrip->setMode(state.wxmode);
			if (!pStrip->isRunning()) pStrip->start();
		}
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
	//CController::set_state(state);
	//CManualStateController<RGBStripController, RGBState, RGBCMD>::set_state(state);
	RGBStrip::set_state(state);
	
}
int RGBStripController::getLDRBrightness(int brigtness,int ldrval) {
	if (ldrval < 100)
		return 196;
	return ((double)(MAX_LDRVAL - ldrval) / MAX_LDRVAL)*brigtness;
}
bool RGBStripController::onpublishmqtt(String& endkey, String& payload) {
	endkey = "Status";
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
bool RGBStripController::onpublishmqttex(String& endkey, String& payload, int topicnr){
	switch (topicnr) {
		case 0:
			endkey = "Status";
			payload = String(this->get_state().isOn ? 1 : 0);
			return true;
		case 1:
			if (!this->get_state().isOn) return false;
			endkey = "Brightness";
			payload = String(this->get_state().brightness);
			return true;
		case 2:
			if (!this->get_state().isOn) return false;
			endkey = "Hue";
			payload = String(this->mqtt_hue);
			return true;
		case 3:
			if (!this->get_state().isOn) return false;
			endkey = "Saturation";
			payload = String(this->mqtt_saturation);
			return true;
	default:
		return false;
	}
	
}
void RGBStripController::onmqqtmessage(String topic, String payload) {
	//RGBState oldState = this->get_state();
	command setcmd;
	setcmd.state= this->get_state();
	if (topic.endsWith("Brightness")) {
		setcmd.mode = SetBrigthness;
		setcmd.state.brightness = payload.toInt();
	}else if(topic.endsWith("Hue")) {
		this->mqtt_hue= payload.toFloat();
		setcmd.state.color = HSVColor(this->mqtt_hue, this->mqtt_saturation, setcmd.state.brightness);
	}
	else if (topic.endsWith("Saturation")) {
		this->mqtt_saturation = payload.toFloat();
		setcmd.state.color = HSVColor(this->mqtt_hue, this->mqtt_saturation, setcmd.state.brightness);
	}
	this->AddCommand(setcmd.state, setcmd.mode, srcMQTT);
}
String RGBStripController::string_modes(void) {
	const size_t bufferSize = JSON_ARRAY_SIZE(pStrip->getModeCount() + 1) + pStrip->getModeCount()*JSON_OBJECT_SIZE(2);
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	for (uint8_t i = 0; i < pStrip->getModeCount(); i++) {
		JsonObject object = json.createNestedObject();
		object["mode"] = i;
		object["name"] = pStrip->getModeName(i);
	}
	JsonObject object = json.createNestedObject();

	String json_str;
	serializeJson(json, json_str);
	return json_str;
}
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
void RGBStripController::setuphandlers(ESP8266WebServer& server) {
	ESP8266WebServer* _server = &server;
#else
void RGBStripController::setuphandlers(WebServer& server) {
	WebServer* _server = &server;
#endif
	String path = "/";
	path += this->get_name();
	path+=  String("/get_modes");
	RGBStripController* self=this;
	_server->on(path, HTTP_GET, [=]() {
		DBG_OUTPUT_PORT.println("get modes request");

		_server->sendHeader("Access-Control-Allow-Origin", "*");
		_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		_server->send_P(200, PSTR("text/html"), self->string_modes().c_str());
		DBG_OUTPUT_PORT.println("Processed");
	});
}
#endif
#if defined ASYNC_WEBSERVER
void RGBStripController::setuphandlers(AsyncWebServer& server) {


	String path = "/";
	path += this->get_name();
	path += String("/get_modes");
	RGBStripController* self = this;
	server.on(path.c_str(), HTTP_GET, [self](AsyncWebServerRequest *request) {
		DBG_OUTPUT_PORT.println("get modes request");
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", 
			self->string_modes().c_str());
		response->addHeader("Access-Control-Allow-Origin", "*");
		response->addHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		
		DBG_OUTPUT_PORT.println("Processed");
	});


}
#endif