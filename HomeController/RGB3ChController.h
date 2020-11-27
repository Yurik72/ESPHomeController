#ifndef RGB3ChController_h
#define RGB3ChController_h


#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
#include <Print.h>
#include <Ticker.h>
#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif


//current limit
#define CONSTRAINT_BR(br,count,limit) (MAX_MA_BR(count,br)>limit)? (br*limit/(MAX_MA_BR(count,br))):br

#define CALC_COLOR(color,br) (((uint32_t)color)*br / 100)

class Ticker;


struct RGB3ChState
{

	bool isOn=false;
	int brightness = 0;
	uint32_t color = 0;
	int fadetm = 0;
	bool isHsv = false;
	//map(value, fromLow, fromHigh, toLow, toHigh)
	int get_br_100(){
		return map(brightness, DIM_MIN_VAL, DIM_MAX_VAL,0,100);
	}
	void set_br_100(int val){
		brightness= map(val, DIM_MIN_VAL,100,0, DIM_MAX_VAL);
	};
	//bool isFloatText = false;
};
enum RGB3ChCMD :uint {
	On = BaseOn,
	Off = BaseOff,
	SetBrigthness = 4,
	SetColor = 16,
	SetLdrVal = 32,
	SetIsLdr  =128,
	SetRGB	  =1024,
	SetRestore = 2048,   //should have the same name
	RgbSaveState = 4096,

};
class RGB3ChController;
typedef CManualStateController<RGB3ChController, RGB3ChState, RGB3ChCMD> RGB3ChStrip;
class RGB3ChController : public RGB3ChStrip
{
public:
	RGB3ChController();
	~RGB3ChController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void runcore();
	virtual void set_state(RGB3ChState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	
	virtual void onmqqtmessage(String topic, String payload);
	virtual bool onpublishmqttex(String& endkey, String& payload, int topicnr);
	virtual bool ispersiststate() { return true; }
	virtual void set_power_on();
	void setColor(uint8_t r, uint8_t g, uint8_t b);
	void setBrightness(uint8_t br);
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
	float mqtt_saturation;
	float mqtt_hue;
	uint32_t temperature = 0;
	uint32_t correction = 0;

	CSmoothVal* pSmooth;

	bool isEnableSmooth;
	uint8_t malimit;
	uint8_t rpin;
	uint8_t gpin;
	uint8_t bpin;
#ifdef ESP32
	int rchannel;
	int gchannel;
	int bchannel;
#endif

};
DEFINE_CONTROLLER_FACTORY(RGB3ChController)

//// auto cycler




#endif
