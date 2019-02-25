#ifndef RGBStripController_h
#define RGBStripController_h


#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
#include <WS2812FX.h>
#if defined(ESP8266)
#define MAX_LDRVAL 1024
#else
#define MAX_LDRVAL 4095

#endif



class WS2812FX;
class Ticker;
class StripWrapper {
public:
	~StripWrapper() {
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
	virtual const char* getModeName(int i) { return NULL; };
	virtual bool isRunning() { return false; }
};

class WS2812Wrapper :public StripWrapper {
public:
	WS2812Wrapper();
	WS2812Wrapper(bool useinternaldriver);
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
	virtual const char* getModeName(int i);
	virtual bool isRunning();
	virtual void trigger(void) ;
private:
	WS2812FX* pstrip;
	bool useinternaldriver;
};

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
protected:
	uint pin;
	uint numleds;

private:
	String string_modes(void);
	StripWrapper* pStripWrapper;
	float mqtt_saturation;
	float mqtt_hue;
	CSmoothVal* pSmooth;
	bool isEnableSmooth;
	
};

#endif
