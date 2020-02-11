#ifndef RGBStripController_h
#define RGBStripController_h


#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
#include "Adafruit_GFX.h"
#include <WS2812FX.h>
#include <Print.h>
#include <Ticker.h>
#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif

#define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
#define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
#define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
#define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
#define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
#define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
#define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
#define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
#define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
#define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
#define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order

//current limit
#define ONE_AMPER_PER    60  //one amper per 60 leds
#define MAX_MA(count) ((count*100)/ONE_AMPER_PER)  //max ma per given led count
#define MAX_MA_BR(count,br) ((MAX_MA(count)*255)/br) //ma per given count and brigtness
#define CONSTRAINT_BR (br,count,limit) (MAX_MA_BR(count,br)>limit)? (br*limit/(MAX_MA_BR(count,br))):br



class WS2812FX;
class Ticker;
class RGBStripCycler;
class RGBStripFloatText;
class StripWrapper;
class RGBStripEffect;
enum CURSOR_CHARMODE {
	PERCHAR = 0,
	PERPIXEL=1
};

class StripMatrix: public  Adafruit_GFX {
public:
	StripMatrix(int w, int h, WS2812FX* p, StripWrapper* pw,
		uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
		//NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);
		NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);
		//NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS);
	virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
	void setPassThruColor(uint32_t c);
	void setPassThruColor();
	void startprint();
	void endprint();
	virtual size_t write(uint8_t c);
	bool setCustomMode(bool val) { isCustomWrite_mode = val;return val; };
	bool setCursorMode(CURSOR_CHARMODE val) { cursormode = val; return true; };
	void  set_inoffset_x(int16_t val);
	void  set_inoffset_y(int16_t val);
protected:
	WS2812FX* pstrip;
	StripWrapper* pwrapper;
	uint8_t type;
	uint8_t	matrixWidth, matrixHeight;
	
	
	uint16_t
	(*remapFn)(uint16_t x, uint16_t y);
	uint32_t passThruColor;
	boolean  passThruFlag = false;
private:
	uint16_t charcounter;
	uint8_t charbytecounter;
	bool isCustomWrite_mode;
	CURSOR_CHARMODE cursormode;
	int16_t _in_offset_x;
	int16_t _in_offset_y;
};
enum COLOR_MODE : uint8_t {

	current = 0,
	randomchar = 2,
	color_wheel=3
};
class StripWrapper {
public:
	StripWrapper() { pcyclerfloattext = NULL; cmode = current; }
	virtual ~StripWrapper() {
		deinit();
	}

	virtual void setup(int pin,int numleds) = 0;
	virtual void init(void)=0;
	virtual void deinit(void) {};
	virtual void start(void) {};
	virtual void stop(void) {};
	virtual void trigger(void) {};
	virtual void setBrightness(uint8_t br) {};
	virtual void setMode(uint8_t mode) {};
	virtual void setColor(uint32_t color) {};
	virtual void setColor(uint8_t r, uint8_t g, uint8_t b) {};
	virtual void setSpeed(uint16_t speed) {};
	virtual void service() {};
	virtual int getModeCount() { return 0; }
	virtual  const __FlashStringHelper* getModeName(int i) { return NULL; };
	virtual bool isRunning() { return false; }
	virtual uint8_t getBrightness(void) { return 0; }
	void set_rgb_startled(int val) { rgb_startled = val; };
	int get_rgb_startled() { return rgb_startled; };
	virtual void setupmatrix(int w, int h, uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS) {};
	virtual void print(String text) {};
	virtual void print_at(int16_t x, String text) {};
	virtual void printfloat(String text) {};
	virtual void resetfloatcycler() {};
	virtual uint8_t setCustomMode(const __FlashStringHelper* name, uint16_t(*p)()) {return 0;};
	virtual void setTextColor(uint16_t c) {};
	void setColorMatrixMode(COLOR_MODE m) { cmode = m; };
	virtual uint32_t color_wheel(uint8_t pos) { return 0;};
	COLOR_MODE getColorMatrixMode() { return cmode; };
	virtual void setPixelColor(uint16_t pix, uint32_t color) {};
	virtual uint32_t getPixelColor(uint16_t pix) { return 0; };
	virtual uint8_t get_matrixtype() { return _matrixType; };
protected:
	int rgb_startled;
	RGBStripFloatText* pcyclerfloattext;
	uint8_t _matrixType;
private:
	COLOR_MODE cmode;
	
};

class WS2812Wrapper :public StripWrapper {
public:
	WS2812Wrapper();
	WS2812Wrapper(bool useinternaldriver);
	virtual ~WS2812Wrapper();
	virtual void init(void);
	virtual void deinit(void);
	virtual void setup(int pin, int numleds);
	virtual void start(void);
	virtual void stop(void);
	virtual void setBrightness(uint8_t br) ;
	virtual void setMode(uint8_t mode);
	virtual void setColor(uint32_t color);
	virtual void setColor(uint8_t r, uint8_t g, uint8_t b);
	virtual void setSpeed(uint16_t speed);
	virtual void service() ;
	virtual int getModeCount();
	virtual  const __FlashStringHelper* getModeName(int i);
	virtual bool isRunning();
	virtual void trigger(void) ;
	virtual uint8_t getBrightness(void);
	virtual void setupmatrix(int w, int h, uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS) ;
	virtual void print(String text);
	virtual void print_at(int16_t x, String text);
	virtual void printfloat(String text);
	virtual void resetfloatcycler();
	virtual uint8_t setCustomMode(const __FlashStringHelper* name, uint16_t(*p)()) ;
	virtual void setTextColor(uint16_t c) ;
	virtual uint32_t color_wheel(uint8_t pos);
	virtual void setPixelColor(uint16_t pix, uint32_t color) ;
	virtual uint32_t getPixelColor(uint16_t pix);
private:
	WS2812FX* pstrip;
	StripMatrix* pMatrix;
	bool useinternaldriver;

};
#define RGB_TEXTLEN 32
struct RGBState
{
	RGBState() {
		//text[0] = '\0';
		memset(text, 0, RGB_TEXTLEN + 1);
	}
	bool isOn=false;
	int brightness = 0;
	int wxmode = 0;
	uint32_t color = 0;
	int wxspeed = 100;
	int ldrValue = 0;
	bool isLdr = false;
	char text[RGB_TEXTLEN+1];
	COLOR_MODE cmode = current;
	//map(value, fromLow, fromHigh, toLow, toHigh)
	int get_br_100(){
		return map(brightness,0,0xFF,0,100);
	}
	void set_br_100(int val){
		brightness= map(val,0,100,0,0xFF);
	};
	//bool isFloatText = false;
};
enum RGBCMD :uint {
	On = BaseOn,
	Off = BaseOff,
	SetBrigthness = 4,
	SetSpeed = 8,
	SetColor = 16,
	SetLdrVal = 32,
	SetMode   =64,
	SetIsLdr  =128,
	SetRGB	  =1024,
	SetRestore = 2048,   //should have the same name
	RgbSaveState = 4096,
	SetText=8192,
	SetFloatText =16384,
	SetMatrixColorMode=32768
};
class RGBStripController;
typedef CManualStateController<RGBStripController, RGBState, RGBCMD> RGBStrip;
class RGBStripController : public RGBStrip
{
public:
	RGBStripController();
	~RGBStripController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void runcore();
	virtual void set_state(RGBState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	int getLDRBrightness(int brigtness, int ldrval);
	virtual void onmqqtmessage(String topic, String payload);
	virtual bool onpublishmqttex(String& endkey, String& payload, int topicnr);
	virtual bool ispersiststate() { return true; }
	virtual void set_power_on();
	static  uint16_t customemodetext() { return (uint16_t)1000; };
	static  uint16_t customemodefloattext() { return (uint16_t)1000; };
	static  uint16_t customeeffect() { return (uint16_t)1000; };
	
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
	virtual void setuphandlers(ESP8266WebServer& server);
#else
	virtual void setuphandlers(WebServer& server);

#endif
#endif
#if defined ASYNC_WEBSERVER
	virtual void setuphandlers(AsyncWebServer& server);
#endif
	void setbrightness(int br, CmdSource src = srcState);
#ifdef	ENABLE_NATIVE_HAP
	virtual void setup_hap_service();
	static void hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);

	virtual void notify_hap();

#endif
protected:
	uint pin;
	uint numleds;
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice;
	homekit_characteristic_t * hap_on;
	homekit_characteristic_t * hap_br;
	homekit_characteristic_t * hap_hue;
	homekit_characteristic_t * hap_saturation;
#endif
private:
	String string_modes(void);
	StripWrapper* pStripWrapper;
	float mqtt_saturation;
	float mqtt_hue;
	CSmoothVal* pSmooth;
	RGBStripCycler* pCycle;
	bool isEnableSmooth;
	uint cyclemode;
	uint textmode;
	uint textfloatmode;
	uint firemode;
	uint snowmode;
	int rgb_startled;   /// indicates from which led rgb  sequence started instead grb
	bool ismatrix;
	uint8_t matrixWidth;
	uint8_t matrixType;
	RGBStripEffect* pEffect;
	uint8_t malimit;

};
DEFINE_CONTROLLER_FACTORY(RGBStripController)

//// auto cycler

#define GET_CYCLE_PARAM(parr,row,idx) parr[row*4+idx]
class RGBStripCycler
{
public:
	RGBStripCycler(StripWrapper* pStrip );
	void start();
	void stop();
	void reset();
	void oncallback();
	static void callback(RGBStripCycler* self);
	
protected:
	Ticker cycleTicker;
	uint8_t cycleIndex ;
	uint8_t cyclecount;
	uint32_t *pcycleParams;

	StripWrapper* pStripWrapper;
};
enum FLOAT_DIRECTION :boolean{
	left=0,
	right=1
};
class RGBStripFloatText
{
	
public:
	RGBStripFloatText(StripWrapper* pStrip,String text,uint8_t matrixwidth=32,double interval=1.0);
	void start();
	void stop();
	void reset();
	void oncallback();
	static void callback(RGBStripFloatText* self);

protected:
	Ticker cycleTicker;
	uint16_t cycleIndex;

	uint16_t cyclecount;
	uint16_t minorcycleIndex;
	String txt;
	float delay_interval;
	uint8_t mwidth;
	StripWrapper* pStripWrapper;
	FLOAT_DIRECTION direction;
	uint8_t charwidth;
	uint16_t textlen;
};

#endif
