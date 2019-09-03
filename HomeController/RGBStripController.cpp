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

/// Matrix
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#else
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#endif

#ifdef __AVR
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#endif

static const uint8_t PROGMEM
gamma5[] = {
  0x00,0x01,0x02,0x03,0x05,0x07,0x09,0x0b,
  0x0e,0x11,0x14,0x18,0x1d,0x22,0x28,0x2e,
  0x36,0x3d,0x46,0x4f,0x59,0x64,0x6f,0x7c,
  0x89,0x97,0xa6,0xb6,0xc7,0xd9,0xeb,0xff },
  gamma6[] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x08,
	0x09,0x0a,0x0b,0x0d,0x0e,0x10,0x12,0x13,
	0x15,0x17,0x19,0x1b,0x1d,0x20,0x22,0x25,
	0x27,0x2a,0x2d,0x30,0x33,0x37,0x3a,0x3e,
	0x41,0x45,0x49,0x4d,0x52,0x56,0x5b,0x5f,
	0x64,0x69,0x6e,0x74,0x79,0x7f,0x85,0x8b,
	0x91,0x97,0x9d,0xa4,0xab,0xb2,0xb9,0xc0,
	0xc7,0xcf,0xd6,0xde,0xe6,0xee,0xf7,0xff };


#ifndef _swap_uint16_t
#define _swap_uint16_t(a, b) { uint16_t t = a; a = b; b = t; }
#endif
static uint32_t expandColor(uint16_t color) {
	return ((uint32_t)pgm_read_byte(&gamma5[color >> 11]) << 16) |
		((uint32_t)pgm_read_byte(&gamma6[(color >> 5) & 0x3F]) << 8) |
		pgm_read_byte(&gamma5[color & 0x1F]);
}


StripMatrix::StripMatrix(int w, int h, WS2812FX* p, uint8_t matrixType) :Adafruit_GFX(w, h) {

	//DBG_OUTPUT_PORT.println(String("Ctor") + String(matrixType));
	type = matrixType;
	matrixWidth = w;
	matrixHeight = h;
	pstrip = p;

	rotation = 0;
	cursor_y = cursor_x = 0;
	remapFn = NULL;
	
}
void StripMatrix::drawPixel(int16_t x, int16_t y, uint16_t color) {
	
	if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return;

	int16_t t;
	switch (rotation) {
	case 1:
		t = x;
		x = matrixWidth - 1 - y;
		y = t;
		break;
	case 2:
		x = matrixWidth - 1 - x;
		y = matrixHeight - 1 - y;
		break;
	case 3:
		t = x;
		x = y;
		y = matrixHeight - 1 - t;
		break;
	}

	int tileOffset = 0, pixelOffset;

	if (remapFn) { // Custom X/Y remapping function
		pixelOffset = (*remapFn)(x, y);
	}
	else {      // Standard single matrix or tiled matrices

		uint8_t  corner = type & NEO_MATRIX_CORNER;
		uint16_t minor, major, majorScale;

		

		// Find pixel number within tile
		minor = x; // Presume row major to start (will swap later if needed)
		major = y;

		// Determine corner of entry, flip axes if needed
		if (corner & NEO_MATRIX_RIGHT)  minor = matrixWidth - 1 - minor;
		if (corner & NEO_MATRIX_BOTTOM) major = matrixHeight - 1 - major;

		// Determine actual major axis of matrix
		if ((type & NEO_MATRIX_AXIS) == NEO_MATRIX_ROWS) {
			majorScale = matrixWidth;
		}
		else {
			_swap_uint16_t(major, minor);
			majorScale = matrixHeight;
		}
		//YKTEST
		//DBG_OUTPUT_PORT.println(String("type")+String(type)+String(" width:")+String(matrixWidth)+String("height:")+String(matrixHeight)+ String("majorScale:") + String(majorScale) + String(" minor:") + String(minor) + String(" major:") + String(major));
		// Determine pixel number within tile/matrix
		if ((type & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_PROGRESSIVE) {
			// All lines in same order
			pixelOffset = major * majorScale + minor;
		}
		else {
			// Zigzag; alternate rows change direction.
			if (major & 1) pixelOffset = (major + 1) * majorScale - 1 - minor;
			else          pixelOffset = major * majorScale + minor;
		}
	}
	//YKTEST
	//DBG_OUTPUT_PORT.println(String("Draw x:")+String(x)+String(" y:")+String(y)+String(" offset:")+String(pixelOffset));
	pstrip->setPixelColor(/*tileOffset +*/ pixelOffset,
		passThruFlag ? passThruColor : expandColor(color));
}


///End matrix

WS2812Wrapper::WS2812Wrapper() {
	pstrip = NULL;
	pMatrix = NULL;
	this->useinternaldriver = false;
	this->rgb_startled = -1;
}
WS2812Wrapper::WS2812Wrapper(bool useinternaldriver):WS2812Wrapper(){
	this->useinternaldriver = useinternaldriver;
}
WS2812Wrapper::~WS2812Wrapper() {
	if (pstrip)
		delete pstrip;
}
void WS2812Wrapper::setup(int pin, int numleds) {
#ifdef  ESP8266	
	if (rgb_startled == 1) {
		pstrip = new  WS2812FX(numleds, pin, NEO_RGB + NEO_KHZ800);
	}
	else {
		pstrip = new  WS2812FX(numleds, pin, NEO_GRB + NEO_KHZ800);
	}

#else
	pstrip=new  WS2812FX(numleds, pin, NEO_GRB + NEO_KHZ800);
#endif
#ifdef  ESP32	
	if (useinternaldriver) {
		
		ws2812driver.init(pstrip);
		ws2812driver.set_rgb_startled(rgb_startled);
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
uint8_t WS2812Wrapper::getBrightness(void) {
	return pstrip->getBrightness();
}

void WS2812Wrapper::setupmatrix(int w, int h, uint8_t matrixType ) {
	pMatrix = new StripMatrix(w, h, pstrip, matrixType);
}
uint8_t WS2812Wrapper::setCustomMode(const __FlashStringHelper* name, uint16_t(*p)()) {
	return pstrip->setCustomMode(name,p);
}
void WS2812Wrapper::print(String text) {
	if (pMatrix) {
		
		pMatrix->fillScreen(0);
		pMatrix->setCursor(0, 0);
		//YKTEST
		pMatrix->print(text);
		//DBG_OUTPUT_PORT.println("Start Printing");
		//pMatrix->print("0");
	}
};
void WS2812Wrapper::print_at(int16_t x, String text) {
	if (pMatrix) {
		pMatrix->fillScreen(0);
		pMatrix->setCursor(x, 0);
		pMatrix->print(text);
	}
}
void StripWrapper::printfloat(String text) {
	if (pcyclerfloattext)
		delete pcyclerfloattext;
	pcyclerfloattext = new RGBStripFloatText(this, text);
	pcyclerfloattext->start();
}
///// Controller
RGBStripController::RGBStripController() {
	this->pStripWrapper = NULL;
	this->mqtt_hue = 0.0;
	this->mqtt_saturation = 0.0;
	this->pSmooth = new CSmoothVal();
	this->pCycle = NULL;
	this->isEnableSmooth = true;
	this->cyclemode = 0;
	this->rgb_startled = -1;
	this->ismatrix = false;
	this->matrixWidth = 0;
	this->textmode = 0;
	this->matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
		NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG;
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
	if (this->pCycle)
		delete this->pCycle;
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
	root["text"] = String(this->get_state().text);
	String json;
	json.reserve(256);
	serializeJson(root, json);

	return json;
}
bool  RGBStripController::deserializestate(String jsonstate, CmdSource src) {
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
//	if(json.containsKey("rgb_startled"))
//		rgb_startled= json["rgb_startled"];
	loadif(rgb_startled, json, "rgb_startled");
	loadif(ismatrix, json, "ismatrix");
	loadif(matrixWidth, json, "matrixwidth");
	loadif(matrixType, json, "matrixType");


}
void RGBStripController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)]= pin;
	json["numleds"]= numleds;
	json[FPSTR(szservice)] = "RGBStripController";
	json[FPSTR(szname)] = "RGBStrip";
	json["issmooth"] = false;
	json["rgb_startled"] = -1;
	json["ismatrix"] = false;
	json["matrixwidth"] = 0;
	RGBStrip::getdefaultconfig(json);
}
void  RGBStripController::setup() {
#ifdef  ESP32	
	pStripWrapper =new  WS2812Wrapper(true);
#else
	pStripWrapper = new  WS2812Wrapper();
#endif
	pStripWrapper->set_rgb_startled(rgb_startled);
	pStripWrapper->setup(pin, numleds);
	if (ismatrix) {
		//TO DO
		uint8_t mtype = NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
			//NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);
			NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG;

		pStripWrapper->setupmatrix(matrixWidth, numleds / matrixWidth, mtype);
		this->textmode = pStripWrapper->setCustomMode(FPSTR_PLATFORM("Show Text"), &RGBStripController::customemodetext);
	}
	pStripWrapper->init();

	pStripWrapper->setBrightness(30);
	pStripWrapper->setSpeed(1000);
	pStripWrapper->setColor(0x007BFF);
	pStripWrapper->setMode(FX_MODE_STATIC);
	
	
	this->pCycle = new RGBStripCycler(pStripWrapper);
	this->cyclemode = pStripWrapper->getModeCount();
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
		case SetText:
			//newState.text = cmd.state.text;
			newState.wxmode = textmode;
			strncpy(newState.text, cmd.state.text, RGB_TEXTLEN);
			break;
		case SetFloatText:
			strncpy(newState.text, cmd.state.text, RGB_TEXTLEN);
			newState.isFloatText = true;
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
			if (oldState.wxmode == this->cyclemode) {
#ifdef RGBSTRIP_DEBUG
				DBG_OUTPUT_PORT.println("cycler stop");
#endif
				this->pCycle->stop();
			}
			if (pStripWrapper->isRunning())pStripWrapper->stop();

			if (state.wxmode == this->cyclemode) {
#ifdef RGBSTRIP_DEBUG
				DBG_OUTPUT_PORT.println("cycler start");
#endif
				this->pCycle->start();
			}
			else {
				if (state.wxmode >= 0) pStripWrapper->setMode(state.wxmode);
			}
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
	if (strlen(state.text) > 0) {
		if (state.isFloatText) {
			pStripWrapper->printfloat(state.text);
		}
		else {
			pStripWrapper->print(state.text);
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
	const size_t bufferSize = JSON_ARRAY_SIZE(pStripWrapper->getModeCount() + 3) +JSON_OBJECT_SIZE(10);
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	
	for (uint8_t i = 0; i < pStripWrapper->getModeCount(); i++) {
		JsonObject object = json.createNestedObject();
		object["mode"] = i;
		object[FPSTR(szname)] = pStripWrapper->getModeName(i);
	}
	JsonObject cycleobj = json.createNestedObject();
	cycleobj["mode"] = this->cyclemode;
	cycleobj[FPSTR(szname)] = F("Cycle modes");
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
#endif
void RGBStripController::setbrightness(int br, CmdSource src) {
	RGBState st = this->get_state();
	if (st.brightness == 0 && br!=0) 
		this->AddCommand(st, On, src);

	st.brightness = br;
	this->AddCommand(st, SetBrigthness, src);
	if (br == 0) 
		this->AddCommand(st, Off, src);
	

}


///cycler
uint32_t defaultcycleParams[][4] = { // color, speed, mode, duration (seconds)  // to do load from json config
	  {0xff0000, 200,  1,  15.0}, // blink red for 15 seconds
	  {0x00ff00, 200,  3, 20.0}, // wipe green for 20 seconds
	  {0x0000ff, 200, 11,  25.0}, // dual scan blue for 25 seconds
	  {0xeeeeee, 200, 29,  25.0}, // Blink Rainbow blue for 25 seconds
	  {0xeeeeee, 200, 32,  25.0}, // Chase Random for 25 seconds
	  {0xeeeeee, 200, 33,  25.0}, // Chase Rainbow for 25 seconds
	  {0x0000ff, 200, 42, 25.0}  // fireworks for 25 seconds
};

RGBStripCycler::RGBStripCycler(StripWrapper* pStrip) {
	cycleIndex = 0;
	cyclecount = 7;
	
	pcycleParams = &defaultcycleParams[0][0];
	pStripWrapper = pStrip;
}

void RGBStripCycler::start() {
	this->reset();
#ifdef RGBSTRIP_DEBUG
	DBG_OUTPUT_PORT.println("RGBStripCycler  start");
#endif

	this->oncallback();
}
void RGBStripCycler::stop() {
	cycleTicker.detach();
}
void RGBStripCycler::reset() {
	cycleIndex = 0;
}
void RGBStripCycler::oncallback() {
	uint8_t br = this->pStripWrapper->getBrightness();
#ifdef RGBSTRIP_DEBUG
	DBG_OUTPUT_PORT.println("RGBStripCycler  oncallback");
	DBG_OUTPUT_PORT.print("color");
	DBG_OUTPUT_PORT.println(GET_CYCLE_PARAM(pcycleParams, cycleIndex, 0));

	DBG_OUTPUT_PORT.print("mode");
	DBG_OUTPUT_PORT.println(GET_CYCLE_PARAM(pcycleParams, cycleIndex, 2));

	DBG_OUTPUT_PORT.print("time");
	DBG_OUTPUT_PORT.println(GET_CYCLE_PARAM(pcycleParams, cycleIndex, 3));
	
	DBG_OUTPUT_PORT.print("brigthness");
	DBG_OUTPUT_PORT.println(br);

#endif
	this->pStripWrapper->setColor(GET_CYCLE_PARAM(pcycleParams, cycleIndex, 0));
	this->pStripWrapper->setMode(GET_CYCLE_PARAM(pcycleParams, cycleIndex, 2));
	this->pStripWrapper->setSpeed(GET_CYCLE_PARAM(pcycleParams, cycleIndex, 1));
	this->pStripWrapper->setBrightness(br);
	cycleTicker.once<RGBStripCycler*>(GET_CYCLE_PARAM(pcycleParams, cycleIndex, 3), RGBStripCycler::callback, this);
	cycleIndex++;
	if (cycleIndex >= cyclecount) cycleIndex = 0;
}
void RGBStripCycler::callback(RGBStripCycler* self) {
	self->oncallback();
}

RGBStripFloatText::RGBStripFloatText(StripWrapper* pStrip, String text) {
	cycleIndex = 0;
	cyclecount = text.length();

	txt = text;
	pStripWrapper = pStrip;
}

void RGBStripFloatText::start() {
	this->reset();

	this->oncallback();
}
void RGBStripFloatText::stop() {
	cycleTicker.detach();
}
void RGBStripFloatText::reset() {
	cycleIndex = 0;
}
void RGBStripFloatText::oncallback() {

	this->pStripWrapper->print_at(cycleIndex, txt);
	cycleTicker.once<RGBStripFloatText*>(1.0, RGBStripFloatText::callback, this);
	cycleIndex++;
	if (cycleIndex >= cyclecount) cycleIndex = 0;
}
void RGBStripFloatText::callback(RGBStripFloatText* self) {
	self->oncallback();
}