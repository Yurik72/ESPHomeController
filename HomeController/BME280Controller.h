#ifndef BME280Controller_h
#define BME280Controller_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
#include "BMEHistory.h"

#define PERIOD_HISTORY_MS 36000
#define MAX_HISTORY_RECORDS 20

#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif

class Adafruit_BME280;
struct BMEState
{
	bool isOn;
	double temp=0.0;
	double hum = 0.0;
	double pres = 0.0;
	long gtime = 0;
};
enum BMECMD {
	BMEOn = 1,
	BMEOff = 2,
	BMESet = 3,
	BMEMeasure=4,
	BMERestore = 2048
};
class BME280Controller;
typedef CBMEHistory<BMEState> BME280History;
typedef CController<BME280Controller, BMEState, BMECMD> BME;
class BME280Controller : public BME
{
public:
	BME280Controller();
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
	void meassure(BMEState& state);
	void directmeassure(BMEState& state);
	void write8(byte reg, byte value);
	uint8_t i2caddr;
	bool uselegacy;
	bool usehistory;
	bool isinit;
	double temp_corr;
	double hum_corr;
	Adafruit_BME280* pbme;
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice_temp;
	homekit_characteristic_t * hap_temp;

	homekit_service_t* hapservice_hum;
	homekit_characteristic_t * hap_hum;

	homekit_service_t* hapservice_press;
	homekit_characteristic_t * hap_press;
#endif
	BME280History* pHistory;

};
DEFINE_CONTROLLER_FACTORY(BME280Controller)

#endif
