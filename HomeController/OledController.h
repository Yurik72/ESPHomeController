#ifndef oledcontroller_h
#define oledcontroller_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include <functional>
#include "Array.h"
#include <SSD1306.h>
#include "BaseController.h"

struct OledState
{
	bool isOn = false;

	uint8_t brightness = 0;
	String text;
	size_t x = 0;
	size_t y = 0;
	uint8_t fontsize = 1;
	bool isClear = false;
};
enum OLEDCMD {
	OledOn = BaseOn,
	OledOff = BaseOff,

	OledSetBrigthness = 16,
	OledDrawText = 32,
	OledClear=64,
	OledSetFont=128
};
class OledController;
typedef CController<OledController, OledState, OLEDCMD> Oled;
class OledController :public Oled
{
public:
	OledController() {

	};
	virtual void setup();
	virtual void setup(int adr= 0x3c, int pinSda =5, int pinScl =4) {};
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	
	void loadconfig(JsonObject& json);
	virtual int get_maxy() { return 0; };
	virtual int get_maxx() { return 0; };
	virtual void clear() {};
	virtual void update() {};
	virtual int drawline(size_t idx, const String& text) {};
	virtual void drawtext(size_t x, size_t y, const String& text) {};
protected:
	uint8_t i2caddr;
};

//class SSD1306;

class OledControllerSSD1306 :public  OledController {
public:
	OledControllerSSD1306();
	~OledControllerSSD1306();
	virtual int get_maxy() { return 64; };
	virtual int get_maxx() { return 128; };
	virtual void getdefaultconfig(JsonObject& json);
	virtual void run();
	virtual void setup();
	virtual void setup(int adr = 0x3c, int pinSda = 21, int pinScl = 22);
	virtual int drawline(size_t idx, const String& text) ;
	virtual void drawtext(size_t x, size_t y, const String& text) ;
	virtual void set_state(OledState state);
	virtual void clear() ;
	virtual void update() ;
protected:
	SSD1306* pdisplay; 
};
DEFINE_CONTROLLER_FACTORY(OledControllerSSD1306)
#endif