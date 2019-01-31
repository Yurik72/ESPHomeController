#ifndef BME280Controller_h
#define BME280Controller_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

class Adafruit_BME280;
struct BMEState
{
	bool isOn;
	double temp=0.0;
	double hum = 0.0;
	double pres = 0.0;
};
enum BMECMD {
	BMEOn = 1,
	BMEOff = 2,
	BMESet = 3,
	BMEMeasure=4,
	BMERestore = 2048
};
class BME280Controller;
typedef CController<BME280Controller, BMEState, BMECMD> BME;
class BME280Controller : public BME
{
public:
	BME280Controller();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	
	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual void onmqqtmessage(String topic, String payload);
protected:
	void meassure(BMEState& state);
	void directmeassure(BMEState& state);
	void write8(byte reg, byte value);
	uint8_t i2caddr;
	bool uselegacy;
	bool isinit;
	Adafruit_BME280* pbme;
};


#endif