#ifndef BME680Controller_h
#define BME680Controller_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define QUALITY_EXCELLENT_LEVEL 500.0
#define QUALITY_POOR_LEVEL 2500.0

#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif
#ifdef USE_ADAFRUIT_680
class Adafruit_BME680;
#else
class BME680_Class;
#endif

struct BME680State
{
	bool isOn;
	double temp=0.0;
	double hum = 0.0;
	double pres = 0.0;
	double gas = 0.0;
	uint32_t gas_resistance = 0.0;
	long last_measure_ms;
	
};
enum BME680CMD {
	BME680On = 1,
	BME680Off = 2,
	BME680Set = 3,
	BME680Measure=4,
	BME680Restore = 2048
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
	float calc_PPM(float val);
	float readScaled(float val, float a, float b);
	int getpsevdoIaqGasScore(int  val);

	uint8_t air_quality_level(float ppm, uint8_t min, uint8_t max);
	uint8_t i2caddr;
	bool uselegacy;
	bool isinit;
	double temp_corr;
	double hum_corr;
	float factor = 1.0;
	int   gas_lower_limit;
	int   gas_upper_limit;
#ifdef USE_ADAFRUIT_680
	Adafruit_BME680  * pbme;
	uint32_t ReadGasReference();
	int GetHumidityScore(float current_humidity);
	float hum_reference = 40;
	void PrepareCalibrate();
#else
	BME680_Class * pbme;
#endif
	
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice_temp;
	homekit_characteristic_t * hap_temp;

	homekit_service_t* hapservice_hum;
	homekit_characteristic_t * hap_hum;

	homekit_service_t* hapservice_press;
	homekit_characteristic_t * hap_press;

	homekit_service_t* hapservice_gas;
	homekit_characteristic_t * hap_gas;
	homekit_characteristic_t * hap_level;
#endif
};
DEFINE_CONTROLLER_FACTORY(BME680Controller)

#endif
