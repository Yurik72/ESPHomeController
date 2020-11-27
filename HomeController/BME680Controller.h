#ifndef BME680Controller_h
#define BME680Controller_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif

class BME680_Class;
struct BME680State
{
	bool isOn;
	double temp=0.0;
	double hum = 0.0;
	double pres = 0.0;
	double gas = 0.0;
};
enum BME680CMD {
	BMEOn = 1,
	BMEOff = 2,
	BMESet = 3,
	BMEMeasure=4,
	BMERestore = 2048
};
class BME680Controller;
typedef CController<BME680Controller, BME680State, BME680CMD> BME680;
class BME680Controller : public BME680
{
public:
	BME680Controller();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	
	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual void onmqqtmessage(String topic, String payload);
#ifdef	ENABLE_NATIVE_HAP
	virtual void setup_hap_service();
	static void hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);

	virtual void notify_hap();

#endif
protected:
	void meassure(BME680State& state);
	void directmeassure(BME680State& state);
	void write8(byte reg, byte value);
	uint8_t i2caddr;
	bool uselegacy;
	bool isinit;
	double temp_corr;
	double hum_corr;
	BME680_Class * pbme;
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice_temp;
	homekit_characteristic_t * hap_temp;

	homekit_service_t* hapservice_hum;
	homekit_characteristic_t * hap_hum;

	homekit_service_t* hapservice_press;
	homekit_characteristic_t * hap_press;
#endif
};
DEFINE_CONTROLLER_FACTORY(BME680Controller)

#endif
