#ifndef RGBStripController_h
#define RGBStripController_h

#include <WS2812FX.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
#if defined(ESP8266)
#define MAX_LDRVAL 1024
#else
#define MAX_LDRVAL 4095

#endif
class WS2812FX;
struct RGBState
{
	bool isOn;
	int brightness = 0;
	int wxmode = 0;
	uint32_t color = 0;
	int wxspeed = 0;
	int ldrValue = 0;
	bool isLdr = false;
};
enum RGBCMD :uint {
	On = 1,
	Off = 2,
	SetBrigthness = 4,
	SetSpeed = 8,
	SetColor = 16,
	SetLdrVal = 32,
	SetMode   =64,
	SetIsLdr  =128,
	SetRGB	  =1024,
	SetRestore = 2048,   //should have the same name
	RgbSaveState = 4096
};
class RGBStripController;
typedef CManualStateController<RGBStripController, RGBState, RGBCMD> RGBStrip;
class RGBStripController : public RGBStrip
{
public:
	RGBStripController();
	~RGBStripController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(RGBState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	int getLDRBrightness(int brigtness, int ldrval);
	virtual void onmqqtmessage(String topic, String payload);
	virtual bool onpublishmqttex(String& endkey, String& payload, int topicnr);
	virtual bool ispersiststate() { return true; }
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

protected:
	uint pin;
	uint numleds;
private:
	String string_modes(void);
	WS2812FX* pStrip;
	float mqtt_saturation;
	float mqtt_hue;

};

#endif
