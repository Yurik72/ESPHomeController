#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Utilities.h"
#include "BaseController.h"
#include "RGBStripController.h"

#include <Ticker.h>

#ifdef  ESP32
#include "WS2812Driver.h"

LedInterruptDriver<NS(250), NS(625), NS(375)> ws2812driver;
#endif //  ESP32


#ifdef  ESP32
struct channelregister {
public:
	channelregister() {
		set_first_channel(get_driver_max_channel());
	}
};
static channelregister globchannelregister;
#endif //  ESP32
//REGISTER_CONTROLLER(RGBStripController)
#ifndef DISABLE_RGB
REGISTER_CONTROLLER_FACTORY(RGBStripController)
#endif

const size_t bufferSize = JSON_OBJECT_SIZE(40);
static String rgbModes;
WS2812Wrapper::WS2812Wrapper() {
	pstrip = NULL;
	this->useinternaldriver = false;
}
WS2812Wrapper::WS2812Wrapper(bool useinternaldriver):WS2812Wrapper(){
	this->useinternaldriver = useinternaldriver;
}
WS2812Wrapper::~WS2812Wrapper() {
	if (pstrip)
		delete pstrip;
}
void WS2812Wrapper::setup(int pin, int numleds) {
	pstrip=new  WS2812FX(numleds, pin, NEO_GRB + NEO_KHZ800);
#ifdef  ESP32	
	if (useinternaldriver) {
		
		ws2812driver.init(pstrip);
		pstrip->setCustomShow([] () {
			ws2812driver.customShow();
		
		});
	}
#endif
}
void WS2812Wrapper::init() {
	pstrip->init();
}
void WS2812Wrapper::deinit() {
	delete pstrip;
	pstrip = NULL;
}
void WS2812Wrapper::start() {
	pstrip->start();
}
void WS2812Wrapper::stop() {
	pstrip->stop();
}
void WS2812Wrapper::setBrightness(uint8_t br) {
	pstrip->setBrightness(br);
};
void WS2812Wrapper::setMode(uint8_t mode) {
	pstrip->setMode(mode);
};
void WS2812Wrapper::setColor(uint32_t color) {
	pstrip->setColor(color);
};
void WS2812Wrapper::setColor(uint8_t r, uint8_t g, uint8_t b) {
	pstrip->setColor(r, g, b);
}

void WS2812Wrapper::setSpeed(uint16_t speed) {
	pstrip->setSpeed(speed);
}
void WS2812Wrapper::service() {
	pstrip->service();
}
void WS2812Wrapper::trigger() {
	pstrip->trigger();
}
int WS2812Wrapper::getModeCount() {
	return pstrip->getModeCount();
}
const __FlashStringHelper* WS2812Wrapper::getModeName(int i) {
	return pstrip->getModeName(i);
}
bool WS2812Wrapper::isRunning() {
	return pstrip->isRunning();
}



///// Controller
RGBStripController::RGBStripController() {
	this->pStripWrapper = NULL;
	this->mqtt_hue = 0.0;
	this->mqtt_saturation = 0.0;
	this->pSmooth = new CSmoothVal();
	this->isEnableSmooth = true;
	//rgbModes = "";
	
	//this->coreMode = Both;
	//this->core = 1;
	//this->priority = 100;
}
RGBStripController::~RGBStripController() {
	if (pStripWrapper)
		delete pStripWrapper;
	if (this->pSmooth)
		delete this->pSmooth;
}
String  RGBStripController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	root[FPSTR(szbrightnessText)] = this->get_state().brightness;
	root["color"] = this->get_state().color;
	root["wxmode"] = this->get_state().wxmode;
	root["wxspeed"] = this->get_state().wxspeed;
	root["isLdr"] = this->get_state().isLdr;
	root["ldrValue"] = this->get_state().ldrValue;
	String json;
	json.reserve(256);
	serializeJson(root, json);

	return json;
}
bool  RGBStripController::deserializestate(String jsonstate, CmdSource src) {
	DBG_OUTPUT_PORT.println("RGBStripController::deserializestate");
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
	RGBState newState;
	newState.isOn = root[FPSTR(szisOnText)];
	newState.brightness = root[FPSTR(szbrightnessText)];
	newState.color = root["color"];
	newState.wxmode = root["wxmode"];
	newState.wxspeed = root["wxspeed"];
	newState.isLdr = root["isLdr"];
	newState.ldrValue = root["ldrValue"];
	//this->set_state(newState);
	this->AddCommand(newState, SetRGB, src);
	
	return true;

}
void RGBStripController::loadconfig(JsonObject& json) {
	RGBStrip::loadconfig(json);
	pin = json["pin"];
	numleds = json["numleds"];
	isEnableSmooth = json["issmooth"];
}
void RGBStripController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)]= pin;
	json["numleds"]= numleds;
	json["service"] = "RGBStripController";
	json["name"] = "RGBStrip";
	json["issmooth"] = false;
	RGBStrip::getdefaultconfig(json);
}
void  RGBStripController::setup() {
#ifdef  ESP32	
	pStripWrapper =new  WS2812Wrapper(true);
#else
	pStripWrapper = new  WS2812Wrapper();
#endif
	
	pStripWrapper->setup(pin, numleds);
	pStripWrapper->init();

	pStripWrapper->setBrightness(30);
	pStripWrapper->setSpeed(1000);
	pStripWrapper->setColor(0x007BFF);
	pStripWrapper->setMode(FX_MODE_STATIC);
	pStripWrapper->start();
	RGBStrip::setup();
	//String modes = this->string_modes();
}
void RGBStripController::runcore() {

	pStripWrapper->service();

}
void RGBStripController::run() {
	command cmd;

	pStripWrapper->service();
	yield();
	bool isSet = true;
	if (this->isEnableSmooth && pSmooth->isActive())
		return;   ///ignore 
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
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
		case SetIsLdr:
			newState.isLdr = cmd.state.isLdr;
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
	RGBStrip::run();
}
void RGBStripController::set_state(RGBState state) {
	
	RGBState oldState = this->get_state();
	RGBStripController* self = this;
	bool ignore_br = false;
	if (oldState.isOn != state.isOn) {  // on/off
		if (state.isOn) {
			//DBG_OUTPUT_PORT.println("Switching On");
			//DBG_OUTPUT_PORT.println(state.brightness);
			if (!pStripWrapper->isRunning()) pStripWrapper->start();
			if (this->isEnableSmooth && !pSmooth->isActive()) {
				DBG_OUTPUT_PORT.println("CMD On Smooth");
				pSmooth->stop();
				
				int brval = state.brightness;
				if (state.isLdr)
					brval = getLDRBrightness(state.brightness, state.ldrValue);
				pSmooth->start(0, brval,
					[self](int val) {
						self->pStripWrapper->setBrightness(val);
						self->pStripWrapper->trigger();
					},   //self->setbrightness(val, srcSmooth);},
					[self, state]() {self->AddCommand(state, SetRGB, srcSmooth);});
				ignore_br = true;
				//return;
			}
			else {
				pStripWrapper->setBrightness(state.brightness);
			}

			
			
		}
		else {
			DBG_OUTPUT_PORT.println("Switching OFF");
			if (this->isEnableSmooth && !pSmooth->isActive()) {
				pSmooth->stop();
				pSmooth->start(oldState.brightness,0,
					[self](int val) {
						self->pStripWrapper->setBrightness(val);
						self->pStripWrapper->trigger();
					},//self->setbrightness(val, srcSmooth);},
					[self, state]() {
						if (self->pStripWrapper->isRunning())
							self->pStripWrapper->stop();
						self->AddCommand(state, SetRGB, srcSmooth);
					});
				//return;
				
			}else{
				pStripWrapper->setBrightness(0);
			    if (pStripWrapper->isRunning())pStripWrapper->stop();
		    }
			
		}

	}
	
	
	if (state.isOn) {
		if (oldState.wxmode != state.wxmode) {
			//DBG_OUTPUT_PORT.println("oldState.wxmode != state.wxmod");
			if (pStripWrapper->isRunning())pStripWrapper->stop();
			if (state.wxmode >= 0) pStripWrapper->setMode(state.wxmode);
			if (!pStripWrapper->isRunning()) pStripWrapper->start();
		}
		if (state.isLdr) {
			if (oldState.brightness != state.brightness || oldState.ldrValue!= state.ldrValue && !ignore_br)
				pStripWrapper->setBrightness(getLDRBrightness(state.brightness, state.ldrValue));
		}
		else {
			if (oldState.brightness != state.brightness && !ignore_br)
				pStripWrapper->setBrightness(state.brightness);
		}
		if (oldState.color != state.color)
			pStripWrapper->setColor(REDVALUE(state.color), GREENVALUE(state.color), BLUEVALUE(state.color));

		if ((oldState.color != state.color) || (oldState.brightness != state.brightness)) {
			double intensity = 0.0;
			double hue = 0.0;
			double saturation = 0.0;
			ColorToHSI(state.color, state.brightness, hue, saturation, intensity);
			this->mqtt_hue = hue;
			this->mqtt_saturation = saturation;
		}
	}
	
	pStripWrapper->trigger();
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
	endkey = szStatusText;
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
bool RGBStripController::onpublishmqttex(String& endkey, String& payload, int topicnr){
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
void RGBStripController::onmqqtmessage(String topic, String payload) {
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
String RGBStripController::string_modes(void) {
	if (rgbModes.length() > 0)
		return rgbModes;
	const size_t bufferSize = JSON_ARRAY_SIZE(pStripWrapper->getModeCount() + 1) +JSON_OBJECT_SIZE(10);
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	
	for (uint8_t i = 0; i < pStripWrapper->getModeCount(); i++) {
		JsonObject object = json.createNestedObject();
		object["mode"] = i;
		object["name"] = pStripWrapper->getModeName(i);
	}

	//JsonObject object = json.createNestedObject();
	
	String json_str;
	//json_str.reserve(4096);
	//DBG_OUTPUT_PORT.println("load modes 3");
	serializeJson(json, rgbModes);
	//DBG_OUTPUT_PORT.println("load modes 4");
	//rgbModes = json_str;
	//DBG_OUTPUT_PORT.println(rgbModes);
	//return json_str;
	return rgbModes;
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
	   // DBG_OUTPUT_PORT.println("get modes request");
		//DBG_OUTPUT_PORT.println(ESP.getFreeHeap());
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json",
			self->string_modes().c_str());
		//AsyncWebServerResponse *response = request->beginResponse(200, "application/json","");
		//response->addHeader("Access-Control-Allow-Origin", "*");
		//response->addHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		request->send(response);
		//DBG_OUTPUT_PORT.println(ESP.getFreeHeap());
	//	DBG_OUTPUT_PORT.println("Processed");
	});


}

void RGBStripController::setbrightness(int br, CmdSource src) {
	RGBState st = this->get_state();
	if (st.brightness == 0 && br!=0) 
		this->AddCommand(st, On, src);

	st.brightness = br;
	this->AddCommand(st, SetBrigthness, src);
	if (br == 0) 
		this->AddCommand(st, Off, src);
	

}
#endif