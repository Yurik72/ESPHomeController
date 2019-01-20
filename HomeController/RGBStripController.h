#ifndef RGBStripController_h
#define RGBStripController_h

#include <WS2812FX.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

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
enum RGBCMD:uint { 
	On=1,
	Off=2,
	SetBrigthness=4,
	SetSpeed=8,
	SetColor=16,
	SetLdrVal=32
};

class RGBStripController;
typedef CController<RGBStripController, RGBState, RGBCMD> RGBStrip;
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
protected:
	uint pin;
	uint numleds;
private:
	WS2812FX* pStrip;
};

#endif
