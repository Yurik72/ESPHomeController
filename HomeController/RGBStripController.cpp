#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Utilities.h"
#include "BaseController.h"
#include "RGBStripController.h"

#include <Ticker.h>
#include "RGBStripEffects.h"
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


StripMatrix::StripMatrix(int w, int h, WS2812FX* p, StripWrapper* pw, uint8_t matrixType) :Adafruit_GFX(w, h) {

	//DBG_OUTPUT_PORT.println(String("Ctor") + String(matrixType));
	type = matrixType;
	matrixWidth = w;
	matrixHeight = h;
	pstrip = p;
	pwrapper = pw;
	rotation = 0;
	cursor_y = cursor_x = 0;
	remapFn = NULL;
	charcounter = 0;
	charbytecounter = 0;
	isCustomWrite_mode = true;
	_in_offset_x = 0;
	_in_offset_y = 0;
}
void  StripMatrix::set_inoffset_x(int16_t val) {
	_in_offset_x = val;
}
void  StripMatrix::set_inoffset_y(int16_t val) {
	_in_offset_y = val;
}
void StripMatrix::setPassThruColor(uint32_t c) {
	passThruColor = c;
	passThruFlag = true;
}

// Call without a value to reset (disable passthrough)
void StripMatrix::setPassThruColor(void) {
	passThruFlag = false;
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

void StripMatrix::startprint() {
	charcounter = 0;
}
void StripMatrix::endprint() {
}

size_t StripMatrix::write(uint8_t c) {
	charcounter++;
	if (charbytecounter == 0xFF)
		charbytecounter = 0;
	else
		charbytecounter++;
	if (pwrapper && pwrapper->getColorMatrixMode() == randomchar) {
		setPassThruColor(Adafruit_NeoPixel::Color(random(0, 255), random(0, 255), random(0, 255)));
		//setPassThruColor(RED) {
	}
	if (pwrapper && pwrapper->getColorMatrixMode() == color_wheel) {
		setPassThruColor(pwrapper->color_wheel(charbytecounter));
		//setPassThruColor(RED) {
	}
	//DBG_OUTPUT_PORT.println(String("char") + String(c) + String(" cursor_x:") + String(cursor_x)+ String(" cursor_y:") + String(cursor_y));
	size_t res= Adafruit_GFX::write(c);
	//DBG_OUTPUT_PORT.println(String("After") + String(" cursor_x:") + String(cursor_x) + String(" cursor_y:") + String(cursor_y));
	return res;
}
///End matrix
uint32_t StripWrapper::computeAdjustment(uint8_t scale) {

	uint8_t add_color[3];
	add_color[0] = 0;
	add_color[1] = 0;
	add_color[2] = 0;
	//DBG_OUTPUT_PORT.println("computeAdjustment");
	//DBG_OUTPUT_PORT.println(correction);
	if (scale > 0 && ((temperature!=0) || (correction!=0) )) {
		for (uint8_t i = 0; i < 3; i++) {
			uint8_t cc = (correction & (0xFF << (i * 8))) >> (i * 8);//colorCorrection.raw[i];
		
			uint8_t ct = (temperature & (0xFF << (i * 8)))>> (i * 8);//colorTemperature.raw[i];
			if (cc > 0 || ct > 0) {
				uint32_t work = (((uint32_t)cc) + 1) * (((uint32_t)ct) + 1) * scale;
				//DBG_OUTPUT_PORT.println(work);
				//work /= 0x10000L;
				 work /= 0x00010L;
				add_color[i] = work & 0xFF;
				//DBG_OUTPUT_PORT.println(String("add color:") + String(i) + String(":") + String(add_color[i]));

				//	DBG_OUTPUT_PORT.println(cc);
			}
		}
		//color = RGBCOLOR(REDVALUE(color) + add_color[0], GREENVALUE(color) + add_color[1], BLUEVALUE(color) + add_color[2]);
	}
	return RGBCOLOR(add_color[3], add_color[1], add_color[0]);
	
}
 uint32_t StripWrapper::applyAdjustment(uint8_t scale, uint32_t color) {
	 uint32_t adj = computeAdjustment(scale);
	return RGBCOLOR(REDVALUE(color) - REDVALUE(adj), GREENVALUE(color) - GREENVALUE(adj), BLUEVALUE(color) - BLUEVALUE(adj));
}
 uint32_t StripWrapper::reverseAdjustment(uint8_t scale, uint32_t color) {
	 uint32_t adj = computeAdjustment(scale);
	 return RGBCOLOR(REDVALUE(color) + REDVALUE(adj), GREENVALUE(color) + GREENVALUE(adj), BLUEVALUE(color) + BLUEVALUE(adj));
 
}

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
	pstrip->setColor(this->applyAdjustment(this->getBrightness(), color));
};
void WS2812Wrapper::setColor(uint8_t r, uint8_t g, uint8_t b) {
	//TraceColor("set color before", RGBCOLOR(r, g, b));
	pstrip->setColor(this->applyAdjustment(this->getBrightness(), RGBCOLOR(r,g,b)));
	//TraceColor("set color after", this->applyAdjustment(this->getBrightness(), RGBCOLOR(r, g, b)));
}

void WS2812Wrapper::setSpeed(uint16_t speed) {
	pstrip->setSpeed(speed);
}
void WS2812Wrapper::setPixelColor(uint16_t pix, uint32_t color) {
	if (temperature != 0 || correction != 0)
	pstrip->setPixelColor(pix, this->applyAdjustment(this->getBrightness(), color));
	else
		pstrip->setPixelColor(pix, color);
}
uint32_t WS2812Wrapper::getPixelColor(uint16_t pix) {
	if (temperature != 0 || correction != 0)
	return this->reverseAdjustment(this->getBrightness(), pstrip->getPixelColor(pix));
	else
		return pstrip->getPixelColor(pix);
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
	pMatrix = new StripMatrix(w, h, pstrip,this, matrixType);
	this->_matrixType = matrixType;
}
uint8_t WS2812Wrapper::setCustomMode(const __FlashStringHelper* name, uint16_t(*p)()) {
	return pstrip->setCustomMode(name,p);
}
void WS2812Wrapper::print(String text) {
	this->print_at(0, text);
};
void WS2812Wrapper::print_at(int16_t x, String text) {
	
	if (pMatrix) {
		//DBG_OUTPUT_PORT.println(String("Print_at")+text);

		pMatrix->startprint();
		pMatrix->setTextWrap(false);
		pMatrix->setPassThruColor();
		pMatrix->fillScreen(0);
		pMatrix->setCursor(x, 0);
		pMatrix->setPassThruColor(pstrip->getColor());
		pMatrix->print(text);
		pMatrix->endprint();
	}
}
void WS2812Wrapper::printfloat(String text) {
	
	resetfloatcycler();
	if (pMatrix) {
		pcyclerfloattext = new RGBStripFloatText(this, text, pMatrix->width(), pstrip->getSpeed() / 5000.0);
		pcyclerfloattext->start();
	}
}
void WS2812Wrapper::resetfloatcycler() {
	if (pcyclerfloattext) {
		pcyclerfloattext->stop();
		delete pcyclerfloattext;
		pcyclerfloattext = NULL;
	}
}
void WS2812Wrapper::setTextColor(uint16_t c) {
	if (pMatrix) {
		pMatrix->setTextColor(c);
	}
}
uint32_t WS2812Wrapper::color_wheel(uint8_t pos) {
	return pstrip->color_wheel(pos);
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
	this->textfloatmode = 0;
	this->text_timemode = 0;
	this-> firemode = 0;
	this -> snowmode = 0;
	this->matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
		NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG;
	//rgbModes = "";
	
	//this->coreMode = Both;
	//this->core = 1;
	//this->priority = 100;
	pEffect = NULL;
	this->malimit = 2000; //mamper limit
#ifdef	ENABLE_NATIVE_HAP
	this->ishap=true;
	this->hapservice=NULL;
	this->hap_on=NULL;
	this->hap_br=NULL;
	this->hap_hue=NULL;
	this->hap_saturation=NULL;
#endif
}
RGBStripController::~RGBStripController() {
	if (pStripWrapper)
		delete pStripWrapper;
	if (this->pSmooth)
		delete this->pSmooth;
	if (this->pCycle)
		delete this->pCycle;
	if (this->pEffect)
		delete this->pEffect;
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
	RGBState newState = this->get_state();
	newState.isOn = get_json_bool(root, FPSTR(szisOnText));// root[FPSTR(szisOnText)];
	newState.brightness = root[FPSTR(szbrightnessText)];
	newState.color = root["color"];
	newState.wxmode = root["wxmode"];
	newState.wxspeed = root["wxspeed"];
	newState.isLdr = root["isLdr"];
	newState.ldrValue = root["ldrValue"];
	//newState.ldrValue = root["ldrValue"];
//	if (src != srcRestore) {
		String txt = root["text"].as<String>();

		if (txt.length() > 0)
			strncpy(newState.text, txt.c_str(), RGB_TEXTLEN);
	//}
	//else {
//		if (newState.wxmode == textfloatmode)
//			newState.wxmode = 0;
//	}
	//DBG_OUTPUT_PORT.println(String("Deserilize color :") + String(newState.color) + String(" R") + String(REDVALUE(newState.color)) +String(" G") + String(GREENVALUE(newState.color)) + String(" B") + String(BLUEVALUE(newState.color)));
	//	DBG_OUTPUT_PORT.println("RGBStrip deserialize state");
	//	DBG_OUTPUT_PORT.println(newState.isOn);
	//	DBG_OUTPUT_PORT.println(newState.color);
	this->AddCommand(newState, SetRGB, src);
	
	return true;

}
void RGBStripController::loadconfig(JsonObject& json) {
	RGBStrip::loadconfig(json);
	pin = json[FPSTR(szPinText)];
	numleds = json[FPSTR(sznumleds)];
	//isEnableSmooth = json[FPSTR(szissmooth)];
//	if(json.containsKey("rgb_startled"))
//		rgb_startled= json["rgb_startled"];
	loadif(isEnableSmooth, json, FPSTR(szissmooth));
	loadif(rgb_startled, json, FPSTR(szrgb_startled));
	loadif(ismatrix, json, FPSTR(szismatrix));
	loadif(matrixWidth, json, FPSTR(szmatrixwidth));
	loadif(matrixType, json, FPSTR(szmatrixType));
	loadif(malimit, json,"malimit");
	loadif(temperature, json, "temperature");
	loadif(correction, json, "correction");
	//DBG_OUTPUT_PORT.println("matrixType");
	//DBG_OUTPUT_PORT.println(matrixType);
}


void RGBStripController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)]= pin;
	json[FPSTR(sznumleds)]= numleds;
	json[FPSTR(szservice)] = "RGBStripController";
	json[FPSTR(szname)] = "RGBStrip";
	json[FPSTR(szissmooth)] = false;
	json[FPSTR(szrgb_startled)] = -1;
	json[FPSTR(szismatrix)] = false;
	json[FPSTR(szmatrixwidth)] = 0;
	json["temperature"] = 0;
	json["correction"] = 0;
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
	pStripWrapper->setCorrection(correction);
	pStripWrapper->setTemperature(temperature);
	if (ismatrix) {
		//TO DO
		//uint8_t mtype = NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
			//NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);
		//	NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG;

		pStripWrapper->setupmatrix(matrixWidth, numleds / matrixWidth, matrixType);
		this->textmode = pStripWrapper->setCustomMode(FPSTR_PLATFORM("Show Text"), &RGBStripController::customemodetext);
		this->textfloatmode = pStripWrapper->setCustomMode(FPSTR_PLATFORM("Show Float Text"), &RGBStripController::customemodefloattext);
		this->text_timemode= pStripWrapper->setCustomMode(FPSTR_PLATFORM("Show Time"), &RGBStripController::customemodefloattext);
		
	}
	this->firemode = pStripWrapper->setCustomMode(FPSTR_PLATFORM("Fire Fire"), &RGBStripController::customeeffect);
	this->snowmode = pStripWrapper->setCustomMode(FPSTR_PLATFORM("Snow Snow"), &RGBStripController::customeeffect);

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
uint RGBStripController::get_custom_mode(rgb_custom_modes mode) {
	switch (mode) {
		case mode_text:
			return this->textmode;
			break;
		case mode_floattext:
			return this->textfloatmode;
			break;
		case mode_timetext:
			return this->text_timemode;
			break;
		default:
			return 0;
	}
}
bool RGBStripController::is_display_mode() {
	uint currentmode = this->get_state().wxmode;
	return (currentmode == this->textmode || currentmode == this->textfloatmode || currentmode == this->text_timemode);
}
void RGBStripController::runcore() {

	pStripWrapper->service();

}
void RGBStripController::run() {
	command cmd;

	pStripWrapper->service();
//	yield();
	bool isSet = true;
	if (this->isEnableSmooth && pSmooth->isActive())
		return;   ///ignore 
	if (commands.GetSize() == 0 && this->get_state().wxmode == text_timemode) {
		this->AddCommand(this->get_state(), SetTimeMode, srcSelf);
	}
	while (commands.Dequeue(&cmd)) {
		//DBG_OUTPUT_PORT.println("RGBStripController::process command");
		//DBG_OUTPUT_PORT.println(cmd.mode);
		if (this->baseprocesscommands(cmd))
			continue;
		RGBState newState = this->get_state();
		
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
			newState.cmode = cmd.state.cmode;
			if(strlen(cmd.state.text)>0)
				strncpy(newState.text, cmd.state.text, RGB_TEXTLEN);
			break;
		case SetFloatText:

			if (strlen(cmd.state.text)>0)
				strncpy(newState.text, cmd.state.text, RGB_TEXTLEN);
			//newState.isFloatText = true;
			newState.cmode = cmd.state.cmode;
			newState.wxmode = textfloatmode;
			
			break;
		case SetMatrixColorMode:
			newState.cmode = cmd.state.cmode;
			break;
		case SetTimeMode:

			strncpy(newState.text, getFormattedTime_HH_MM(this->get_current_time()).c_str(), RGB_TEXTLEN);
			newState.cmode = cmd.state.cmode;
			newState.wxmode = text_timemode;
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
void RGBStripController::set_power_on() {
	RGBStrip::set_power_on();
	this->run();
	if (pStripWrapper) {
		pStripWrapper->trigger();
	}
}
void RGBStripController::set_state(RGBState state) {
	//DBG_OUTPUT_PORT.println("RGBStripController::set_state");
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
				uint32_t duration = 1000;
				if (state.fadetm > 1) {
					duration = state.fadetm * 1000;
				}
				uint32_t count = duration / 50;
				DBG_OUTPUT_PORT.println(duration);
				DBG_OUTPUT_PORT.println(count);
				pSmooth->start(oldState.brightness,0,
					[self](int val) {
						self->pStripWrapper->setBrightness(val);
						self->pStripWrapper->trigger();
					},//self->setbrightness(val, srcSmooth);},
					[self, state]() {
						if (self->pStripWrapper->isRunning())
							self->pStripWrapper->stop();
						self->AddCommand(state, SetRGB, srcSmooth);
					},duration,count);
				//return;
				
			}else{
				pStripWrapper->setBrightness(0);
			    if (pStripWrapper->isRunning())pStripWrapper->stop();
		    }
			
		}

	}
	
	
	if (state.isOn) {
		if (oldState.wxmode != state.wxmode) {
		
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
			else if (state.wxmode == this->firemode ) {
				if (pEffect)
					delete pEffect;
				pStripWrapper->setMode(state.wxmode);
				pEffect = new RGBStripFireEffect(pStripWrapper, matrixWidth, numleds, 1.0 / state.wxspeed);
				pEffect->run();
			}
			else if (state.wxmode == this->snowmode) {
				if (pEffect)
					delete pEffect;
				pStripWrapper->setMode(state.wxmode);
				pEffect = new RGBStripSnowEffect(pStripWrapper, matrixWidth, numleds, 1.0 / state.wxspeed);
				pEffect->run();
			}
			else {
				if (state.wxmode >= 0) pStripWrapper->setMode(state.wxmode);
			}
			if (!pStripWrapper->isRunning()) pStripWrapper->start();
		}
		if (state.isLdr) {
			if ((oldState.brightness != state.brightness || oldState.ldrValue!= state.ldrValue) && !ignore_br)
				pStripWrapper->setBrightness(getLDRBrightness(state.brightness, state.ldrValue));
		}
		else {
			if (oldState.brightness != state.brightness && !ignore_br) {
				pStripWrapper->setBrightness(state.brightness);
				if((this->correction!=0 || this->temperature != 0) && oldState.color == state.color)
					pStripWrapper->setColor(REDVALUE(state.color), GREENVALUE(state.color), BLUEVALUE(state.color));
			}
		}
		if (oldState.color != state.color) 
			pStripWrapper->setColor(REDVALUE(state.color), GREENVALUE(state.color), BLUEVALUE(state.color));

		if (oldState.wxspeed != state.wxspeed) {
			pStripWrapper->setSpeed(state.wxspeed);
			if (pEffect)
				pEffect->set_delay(1.0 / state.wxspeed);
		}
		if (!state.isHsv && ((oldState.color != state.color) || (oldState.brightness != state.brightness))) {
			double intensity = 0.0;
			double hue = 0.0;
			double saturation = 0.0;
			//DBG_OUTPUT_PORT.print("old saturation");
			//DBG_OUTPUT_PORT.println(this->mqtt_saturation);
			ColorToHSI(state.color, state.brightness, hue, saturation, intensity);
			this->mqtt_hue = hue;

			this->mqtt_saturation = saturation*100.0/255.0;

			//DBG_OUTPUT_PORT.print("set color hue");
			///DBG_OUTPUT_PORT.println(this->mqtt_hue);
			//DBG_OUTPUT_PORT.print("set color saturation");
			//DBG_OUTPUT_PORT.println(this->mqtt_saturation);
		}
		if (oldState.cmode != state.cmode) {
			pStripWrapper->setColorMatrixMode(state.cmode);
		}
		if (strlen(state.text) > 0) {
			if (state.wxmode == textfloatmode) {
				pStripWrapper->printfloat(state.text);
			}
			else if (state.wxmode == textmode || state.wxmode== text_timemode) {
				pStripWrapper->print(state.text);
			}
		}
	}
	//for all cases
	if (oldState.wxmode == this->textfloatmode && oldState.wxmode != state.wxmode) {
		pStripWrapper->resetfloatcycler();
	}
	if (oldState.wxmode == this->firemode && oldState.wxmode != state.wxmode) {
		DBG_OUTPUT_PORT.println("fire mode off");

		if (pEffect) {
			pEffect->stop();
			delete pEffect;
			pEffect = NULL;
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
	//rgbModes = "";
	const size_t bufferSize = JSON_ARRAY_SIZE(pStripWrapper->getModeCount() + 3) +JSON_OBJECT_SIZE(520)+262;
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	//DBG_OUTPUT_PORT.println("string_modes");
	//DBG_OUTPUT_PORT.println(bufferSize);
	for (uint8_t i = 0; i < pStripWrapper->getModeCount(); i++) {
		const __FlashStringHelper* pmd= pStripWrapper->getModeName(i);
		if (!pmd)
			continue;
		JsonObject object = json.createNestedObject();
		
		object["mode"] = i;
		object[FPSTR(szname)] = pmd;// pStripWrapper->getModeName(i);
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
#ifdef	ENABLE_NATIVE_HAP

void RGBStripController::setup_hap_service(){


	DBG_OUTPUT_PORT.println("RGBStripController::setup_hap_service()");
	if(!ishap)
		return;


 //homekit_service_t* x= HOMEKIT_SERVICE(LIGHTBULB, .primary = true);
	//homekit_characteristic_t * ch= NEW_HOMEKIT_CHARACTERISTIC(NAME, "x");

	this->hapservice=hap_add_rgbstrip_service(this->get_name(),RGBStripController::hap_callback,this);
	this->hap_on=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_ON);;
	this->hap_br=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_BRIGHTNESS);;
	this->hap_hue=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_HUE);;
	this->hap_saturation=homekit_service_characteristic_by_type(this->hapservice, HOMEKIT_CHARACTERISTIC_SATURATION);;

}
void RGBStripController::notify_hap(){
	if(this->ishap && this->hapservice){
		//DBG_OUTPUT_PORT.println("RGBStripController::notify_hap");

		RGBState newState=this->get_state();
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
void RGBStripController::hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){
	//DBG_OUTPUT_PORT.println("RGBStripController::hap_callback");

	if(!context){
		return;
	};
		RGBStripController* ctl= (RGBStripController*)context;
		RGBState newState=ctl->get_state();
		RGBCMD cmd = On;
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
			DBG_OUTPUT_PORT.println("HUE");
			DBG_OUTPUT_PORT.println(ch->value.float_value);
		}
		if(ch==ctl->hap_saturation && ch->value.float_value!=ctl->mqtt_saturation){
			ctl->mqtt_saturation=ch->value.float_value;
			newState.color = HSVColor(ctl->mqtt_hue, ctl->mqtt_saturation/100.0, newState.brightness/255.0);
			cmd=SetColor;
			newState.isHsv = true;
			isSet=true;
			DBG_OUTPUT_PORT.println("Saturation");
			DBG_OUTPUT_PORT.println(ch->value.float_value);
		}
	//	newState.isOn=value.bool_value;
		if(isSet)
			ctl->AddCommand(newState, cmd, srcHAP);

}
#endif



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

RGBStripFloatText::RGBStripFloatText(StripWrapper* pStrip, String text, uint8_t matrixwidth ,double interval) {
	cycleIndex = 0;
	
	

	txt = text;
	//DBG_OUTPUT_PORT.println(txt);
	delay_interval = interval;
	mwidth = matrixwidth;
	
	if(delay_interval==0.0)
		delay_interval = 0.5;
	delay_interval = 0.1;
	pStripWrapper = pStrip;
	direction = right;
	cycleIndex = 0;
	minorcycleIndex = 0;
	charwidth = 6;
	textlen = text.length();
	cyclecount = textlen *charwidth+/*space*/1* charwidth;
 	//DBG_OUTPUT_PORT.println(String("RGBStripFloatText")+txt);
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
	
	uint16_t char_onscreen = mwidth / charwidth;
	uint8_t space = 1;
	String txtspace;
	for (int i = 0; i < space; i++)
		txtspace += " ";
	if (direction == right) {
		uint16_t char_offset = cycleIndex / charwidth;
		int16_t posx = minorcycleIndex;
		posx -= charwidth; 
		//draw piece of the first char 
		//char_offset++;
		//
		String txt_toprint = " "+txt;  //leading space always 
		if (char_offset >=space) {
			uint16_t char_to_add = char_offset - space;
			uint16_t from = textlen - char_to_add;
			uint16_t to = from+char_to_add;
			if (to > textlen) to = textlen ;
			//DBG_OUTPUT_PORT.println(String("from:") + String(from) + String(" to:") + String(to));
			//DBG_OUTPUT_PORT.println(txt_toprint.substring(from, to));
			txt_toprint = txt_toprint.substring(from, to) + txtspace + txt_toprint;
			//DBG_OUTPUT_PORT.println("Mode char_offset");
		}
		else {
			posx += char_offset * charwidth;
		}
		//DBG_OUTPUT_PORT.println(String(posx)+String(":")+String(txt_toprint)+String("---")+String(cycleIndex)+String("-")+String(cyclecount));
		//DBG_OUTPUT_PORT.println(txt_toprint.substring(0, char_onscreen));
		//DBG_OUTPUT_PORT.println(String("minor:") + String(minorcycleIndex) +String("posx:") + String(posx) + String(" charofset:") + String(char_offset) + String(" charonscreen:") + String(char_onscreen));
		//DBG_OUTPUT_PORT.println(String("delay:") + String(delay_interval));
		this->pStripWrapper->print_at(posx, txt_toprint.substring(0, char_onscreen+2));
		pStripWrapper->trigger();
		//this->pStripWrapper->print_at(posx, String("0"));// txt_toprint.substring(0, char_onscreen + 2));
		
	}
	else { //TODO
		this->pStripWrapper->print_at(cycleIndex, txt);
	}

	cycleTicker.once<RGBStripFloatText*>(delay_interval, RGBStripFloatText::callback, this);
	cycleIndex++;
	minorcycleIndex++;
	if (minorcycleIndex >= charwidth)
		minorcycleIndex = 0;
	if (cycleIndex >=cyclecount) {
		cycleIndex = 0;
		minorcycleIndex = 0;
	}
}
void RGBStripFloatText::callback(RGBStripFloatText* self) {
	self->oncallback();
}