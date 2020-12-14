#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Utilities.h"
#include "BaseController.h"
#include "RGB3ChController.h"

#include <Ticker.h>


#ifndef DISABLE_RGB
REGISTER_CONTROLLER_FACTORY(RGB3ChController)
#endif

const size_t bufferSize = JSON_OBJECT_SIZE(40);
static String rgbModes;

/// Matrix







///// Controller
RGB3ChController::RGB3ChController() {
	
	this->mqtt_hue = 0.0;
	this->mqtt_saturation = 0.0;
	this->pSmooth = new CSmoothVal();

	this->isEnableSmooth = true;
	//rgbModes = "";
	
	//this->coreMode = Both;
	//this->core = 1;
	//this->priority = 100;
	
	this->malimit = 2000; //mamper limit
	this->rpin=0;
	this->gpin=0;
	this->bpin=0;
#ifdef ESP32
	//.this->channel= get_next_available_channel ();
	this->rchannel = get_next_espchannel();
	this->gchannel = get_next_espchannel();
	this->bchannel = get_next_espchannel();
#endif
#ifdef	ENABLE_NATIVE_HAP
	this->ishap=true;
	this->hapservice=NULL;
	this->hap_on=NULL;
	this->hap_br=NULL;
	this->hap_hue=NULL;
	this->hap_saturation=NULL;
#endif
}
RGB3ChController::~RGB3ChController() {
	if (this->pSmooth)
		delete this->pSmooth;
}
String  RGB3ChController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	root[FPSTR(szbrightnessText)] = this->get_state().brightness;
	root["color"] = this->get_state().color;

	String json;
	json.reserve(256);
	serializeJson(root, json);

	return json;
}
bool  RGB3ChController::deserializestate(String jsonstate, CmdSource src) {
	//DBG_OUTPUT_PORT.println("RGBStripController::deserializestate");
	if (jsonstate.length() == 0) {
		DBG_OUTPUT_PORT.println("State is empty");
		return false;
	}
	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	RGB3ChState newState = this->get_state();
	newState.isOn = get_json_bool(root, FPSTR(szisOnText));// root[FPSTR(szisOnText)];
	newState.brightness = root[FPSTR(szbrightnessText)];
	newState.color = root["color"];


	this->AddCommand(newState, SetRGB, src);
	
	return true;

}
void RGB3ChController::loadconfig(JsonObject& json) {
	RGB3ChStrip::loadconfig(json);
	pin = json[FPSTR(szPinText)];
	numleds = json[FPSTR(sznumleds)];
	//isEnableSmooth = json[FPSTR(szissmooth)];
//	if(json.containsKey("rgb_startled"))
//		rgb_startled= json["rgb_startled"];
	loadif(isEnableSmooth, json, FPSTR(szissmooth));
	loadif(malimit, json,"malimit");
	loadif(temperature, json, "temperature");
	loadif(correction, json, "correction");
	loadif(rpin, json, "rpin");
	loadif(gpin, json, "gpin");
	loadif(bpin, json, "bpin");
}


void RGB3ChController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)]= pin;
	json[FPSTR(sznumleds)]= numleds;
	json[FPSTR(szservice)] = "RGB3ChController";
	json[FPSTR(szname)] = "RGB3Ch";
	json[FPSTR(szissmooth)] = false;
	json[FPSTR(szrgb_startled)] = -1;
	json[FPSTR(szismatrix)] = false;
	json[FPSTR(szmatrixwidth)] = 0;
	json["temperature"] = 0;
	json["correction"] = 0;
	json["rpin"] = 1;
	json["gpin"] = 2;
	json["bpin"] = 3;
	RGB3ChStrip::getdefaultconfig(json);
}
void  RGB3ChController::setup() {


	RGB3ChStrip::setup();
#ifdef ESP8266
	if(rpin>0)
		pinMode(rpin, OUTPUT);
	if (gpin > 0)
		pinMode(gpin, OUTPUT);
	if (bpin > 0)
		pinMode(bpin, OUTPUT);


	//digitalWrite(pin, this->isinvert ? HIGH : LOW);
#endif
#ifdef ESP32
	//	if (gpio_hold_en((gpio_num_t)pin) != ESP_OK) {
	//		DBG_OUTPUT_PORT.println("rtc_gpio_hold_en error");
	//	}
	if (rpin > 0) {
		ledcSetup(rchannel, DIM_FREQ, DIM_RESOLUTION);
		ledcAttachPin(rpin, rchannel);
		pinMode(rpin, OUTPUT);
	}
	if (gpin > 0) {
		ledcSetup(gchannel, DIM_FREQ, DIM_RESOLUTION);
		ledcAttachPin(gpin, gchannel);
		pinMode(gpin, OUTPUT);
	}
	if (bpin > 0) {
		ledcSetup(bchannel, DIM_FREQ, DIM_RESOLUTION);
		ledcAttachPin(bpin, bchannel);
		pinMode(bpin, OUTPUT);
	}
	setBrightness(100);
	//	if (gpio_hold_en((gpio_num_t)pin) != ESP_OK) {
	//		DBG_OUTPUT_PORT.println("rtc_gpio_hold_en error");
	//	}
#endif
}

void RGB3ChController::runcore() {

	

}
void RGB3ChController::setColor(uint8_t r, uint8_t g, uint8_t b) {
	RGB3ChState state = this->get_state();
	if (rpin > 0) {
#ifdef ESP8266
		analogWrite(rpin, DIMCALC_VAL(br, isinvert));
#endif
#ifdef ESP32
		ledcWrite(rchannel, DIMCALC_VAL(CALC_COLOR(r, state.get_br_100()), false));
#endif
	}
	if (gpin > 0) {
#ifdef ESP8266
		analogWrite(gpin, DIMCALC_VAL(br, isinvert));
#endif
#ifdef ESP32
		ledcWrite(gchannel, DIMCALC_VAL(CALC_COLOR(g, state.get_br_100()), false));
#endif
	}
	if (bpin > 0) {
#ifdef ESP8266
		analogWrite(bpin, DIMCALC_VAL(br, isinvert));
#endif
#ifdef ESP32
		ledcWrite(bchannel, DIMCALC_VAL(CALC_COLOR(b, state.get_br_100()), false));
#endif
	}
	/*
	DBG_OUTPUT_PORT.print("RED::");
	DBG_OUTPUT_PORT.println(CALC_COLOR(r, state.get_br_100()));
	DBG_OUTPUT_PORT.print("GREEN::");
	DBG_OUTPUT_PORT.println(CALC_COLOR(g, state.get_br_100()));
	DBG_OUTPUT_PORT.print("BLUE::");
	DBG_OUTPUT_PORT.println(CALC_COLOR(b, state.get_br_100()));
	*/
}
void RGB3ChController::setBrightness(uint8_t br) {
	RGB3ChState state = this->get_state();
	this->setColor(REDVALUE(state.color), GREENVALUE(state.color), BLUEVALUE(state.color));
}


void RGB3ChController::run() {
	command cmd;

	bool isSet = true;
	if (this->isEnableSmooth && pSmooth->isActive())
		return;   ///ignore 
	while (commands.Dequeue(&cmd)) {
		//DBG_OUTPUT_PORT.println("RGBStripController::process command");
		//DBG_OUTPUT_PORT.println(cmd.mode);
		if (this->baseprocesscommands(cmd))
			continue;
		RGB3ChState newState = this->get_state();
		
		switch (cmd.mode) {
		case On:

			newState.isOn = true;
			newState.fadetm = cmd.state.fadetm;
			break;
		case Off:
			newState.isOn = false;
			newState.fadetm = cmd.state.fadetm;
			break;
		case SetBrigthness:
			newState.brightness = cmd.state.brightness;
			break;
		case SetColor:
			newState.color = cmd.state.color;
			newState.isHsv = cmd.state.isHsv;
			break;
		case SetRGB:
		case SetRestore:
			newState = cmd.state;
			break;

		default:
			isSet = false;
			break;
		}
		if(isSet)
			this->set_state(newState);
	}
	RGB3ChStrip::run();
}
void RGB3ChController::set_power_on() {
	RGB3ChStrip::set_power_on();
	this->run();
}
void RGB3ChController::set_state(RGB3ChState state) {
	//DBG_OUTPUT_PORT.println("RGBStripController::set_state");
	RGB3ChState oldState = this->get_state();
	RGB3ChController* self = this;
	bool ignore_br = false;
	if (oldState.isOn != state.isOn) {  // on/off
		if (state.isOn) {
			if (this->isEnableSmooth && !pSmooth->isActive()) {
			//	DBG_OUTPUT_PORT.println("CMD On Smooth");
				pSmooth->stop();
				
				int brval = state.brightness;
				pSmooth->start(0, brval,
					[self](int val) {
						self->setBrightness(val);
						
					},   //self->setbrightness(val, srcSmooth);},
					[self, state]() {self->AddCommand(state, SetRGB, srcSmooth);});
				ignore_br = true;
				//return;
			}
			else {
				this->setBrightness(state.brightness);
			}

			
			
		}
		else {
			//DBG_OUTPUT_PORT.println("Switching OFF");
			if (this->isEnableSmooth && !pSmooth->isActive()) {
				pSmooth->stop();
				uint32_t duration = 1000;
				if (state.fadetm > 1) {
					duration = state.fadetm * 1000;
				}
				uint32_t count = duration / 50;
				//DBG_OUTPUT_PORT.println(duration);
				//DBG_OUTPUT_PORT.println(count);
				pSmooth->start(oldState.brightness,0,
					[self](int val) {
						self->setBrightness(val);
						//self->pStripWrapper->trigger();
					},//self->setbrightness(val, srcSmooth);},
					[self, state]() {
						//if (self->pStripWrapper->isRunning())
						//	self->pStripWrapper->stop();
						self->AddCommand(state, SetRGB, srcSmooth);
					},duration,count);
				//return;
				
			}else{
				this->setBrightness(0);
		    }
			
		}

	}
	
	
	if (state.isOn) {
		if (oldState.color != state.color) 
			this->setColor(REDVALUE(state.color), GREENVALUE(state.color), BLUEVALUE(state.color));

		if (!state.isHsv && ((oldState.color != state.color) || (oldState.brightness != state.brightness))) {
			double intensity = 0.0;
			double hue = 0.0;
			double saturation = 0.0;
			ColorToHSI(state.color, state.brightness, hue, saturation, intensity);
			this->mqtt_hue = hue;

			this->mqtt_saturation = saturation*100.0/255.0;

		}
		if (oldState.brightness != state.brightness && !ignore_br) {
			this->setBrightness(state.brightness);
		}
	}

	RGB3ChStrip::set_state(state);
	
}

bool RGB3ChController::onpublishmqtt(String& endkey, String& payload) {
	endkey = szStatusText;
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
bool RGB3ChController::onpublishmqttex(String& endkey, String& payload, int topicnr){
	switch (topicnr) {
		case 0:
			endkey = szStatusText;
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
void RGB3ChController::onmqqtmessage(String topic, String payload) {
	//RGBState oldState = this->get_state();
	command setcmd;
	setcmd.state= this->get_state();
	if (topic.endsWith("Set")) {
		if (payload.toInt() > 0) {
			setcmd.mode = On;
			setcmd.state.isOn = true;
		}
		else {
			setcmd.mode = Off;
			setcmd.state.isOn = false;
		}

	}
	if (topic.endsWith("Brightness")) {
		setcmd.mode = SetBrigthness;
		setcmd.state.brightness = payload.toInt();
	}else if(topic.endsWith("Hue")) {
		this->mqtt_hue= payload.toFloat();
		setcmd.state.color = HSVColor(this->mqtt_hue, this->mqtt_saturation, setcmd.state.brightness);
		setcmd.mode = SetColor;
#ifdef MQTT_DEBUG
		DBG_OUTPUT_PORT.print("Mqtt: Hue,hue = color = ");
		DBG_OUTPUT_PORT.println(this->mqtt_hue);
		DBG_OUTPUT_PORT.println(setcmd.state.color);
#endif
	}
	else if (topic.endsWith("Saturation")) {
		this->mqtt_saturation = payload.toFloat();
		setcmd.state.color = HSVColor(this->mqtt_hue, this->mqtt_saturation, setcmd.state.brightness);
		setcmd.mode = SetColor;
#ifdef MQTT_DEBUG
		DBG_OUTPUT_PORT.print("Mqtt: Saturation,sat- color = ");
		DBG_OUTPUT_PORT.println(this->mqtt_saturation);
		DBG_OUTPUT_PORT.println(setcmd.state.color);
#endif
	}
	this->AddCommand(setcmd.state, setcmd.mode, srcMQTT);
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
void RGB3ChController::setuphandlers(AsyncWebServer& server) {


}
#endif
void RGB3ChController::setbrightness(int br, CmdSource src) {
	RGB3ChState st = this->get_state();
	if (st.brightness == 0 && br!=0) 
		this->AddCommand(st, On, src);

	st.brightness = br;
	this->AddCommand(st, SetBrigthness, src);
	if (br == 0) 
		this->AddCommand(st, Off, src);
	

}
#ifdef	ENABLE_NATIVE_HAP

void RGB3ChController::setup_hap_service(){


	DBG_OUTPUT_PORT.println("RGB3ChController::setup_hap_service()");
	if(!ishap)
		return;

	if (this->accessory_type > 1) {
		this->hapservice = hap_add_rgbstrip_service_as_accessory(this->accessory_type, this->get_name(), RGB3ChController::hap_callback, this);
	}
	else
	{
		this->hapservice = hap_add_rgbstrip_service(this->get_name(), RGB3ChController::hap_callback, this);
	}
 //homekit_service_t* x= HOMEKIT_SERVICE(LIGHTBULB, .primary = true);
	//homekit_characteristic_t * ch= NEW_HOMEKIT_CHARACTERISTIC(NAME, "x");

	//this->hapservice=hap_add_rgbstrip_service(this->get_name(), RGB3ChController::hap_callback,this);
	this->hap_on=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_ON);;
	this->hap_br=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_BRIGHTNESS);;
	this->hap_hue=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_HUE);;
	this->hap_saturation=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_SATURATION);;

}
void RGB3ChController::notify_hap(){
	if(this->ishap && this->hapservice){
		//DBG_OUTPUT_PORT.println("RGBStripController::notify_hap");

		RGB3ChState newState=this->get_state();
		if(this->hap_on && this->hap_on->value.bool_value!=newState.isOn){
			this->hap_on->value.bool_value=newState.isOn;
		  homekit_characteristic_notify(this->hap_on,this->hap_on->value);
		}
		if(this->hap_br && this->hap_br->value.int_value !=newState.get_br_100()){
			this->hap_br->value.int_value=newState.get_br_100();
		  homekit_characteristic_notify(this->hap_br,this->hap_br->value);
		}
		if(this->hap_hue && this->hap_hue->value.float_value !=this->mqtt_hue){
			this->hap_hue->value.float_value=this->mqtt_hue;
		  homekit_characteristic_notify(this->hap_hue,this->hap_hue->value);
		}
		if(this->hap_saturation && this->hap_saturation->value.float_value !=this->mqtt_saturation){
			this->hap_saturation->value.float_value=this->mqtt_saturation;
		  homekit_characteristic_notify(this->hap_saturation,this->hap_saturation->value);
		}
	}
}
void RGB3ChController::hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){
	//DBG_OUTPUT_PORT.println("RGBStripController::hap_callback");

	if(!context){
		return;
	};
		RGB3ChController* ctl= (RGB3ChController*)context;
		RGB3ChState newState=ctl->get_state();
		RGB3ChCMD cmd = On;
		bool isSet=false;
		if(ch==ctl->hap_on && ch->value.bool_value!=newState.isOn){
			newState.isOn=ch->value.bool_value;
			cmd =newState.isOn?On:Off;
			isSet=true;
		}
		if(ch==ctl->hap_br && ch->value.int_value!=newState.get_br_100()){
			newState.set_br_100(ch->value.int_value);
			cmd=SetBrigthness;
			isSet=true;
		}
		if(ch==ctl->hap_hue && ch->value.float_value!=ctl->mqtt_hue){
			ctl->mqtt_hue=ch->value.float_value;
			newState.isHsv = true;
			newState.color = HSVColor(ctl->mqtt_hue, ctl->mqtt_saturation/100.0, newState.brightness / 255.0);
			cmd=SetColor;
			//isSet=true;
			//DBG_OUTPUT_PORT.println("HUE");
			//DBG_OUTPUT_PORT.println(ch->value.float_value);
		}
		if(ch==ctl->hap_saturation && ch->value.float_value!=ctl->mqtt_saturation){
			ctl->mqtt_saturation=ch->value.float_value;
			newState.color = HSVColor(ctl->mqtt_hue, ctl->mqtt_saturation/100.0, newState.brightness/255.0);
			cmd=SetColor;
			newState.isHsv = true;
			isSet=true;
			//DBG_OUTPUT_PORT.println("Saturation");
			//DBG_OUTPUT_PORT.println(ch->value.float_value);
		}
	//	newState.isOn=value.bool_value;
		if(isSet)
			ctl->AddCommand(newState, cmd, srcHAP);

}
#endif




