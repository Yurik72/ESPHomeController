#ifndef DallasController_h
#define DallasController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
#include <OneWire.h>
#include <DallasTemperature.h>

struct DallasState
{
	bool isOn;
	double temp = 0.0;
	double hum = 0.0;
	double pres = 0.0;
};
enum DallasCMD {
	BMEOn = 1,
	BMEOff = 2,
	BMESet = 3,
	BMEMeasure = 4,
	BMERestore = 2048
};
class DallasController;
typedef CController<DallasController, DallasState, DallasCMD> Dallas;
class DallasController : public Dallas
{
public:
	DallasController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();

	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual void onmqqtmessage(String topic, String payload);
protected:
	void meassure(DallasState& state);

	uint8_t i2caddr;
	uint pin;
	DallasTemperature* psensors;
	OneWire* poneWire;
};
DEFINE_CONTROLLER_FACTORY(DallasController)

#endif
