#ifndef LDRController_h
#define LDRController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define OPERATING_VOLTAGE 5.0
#define RLOAD 10.0
#ifdef ESP32
#define ADC_VALUE_MAX 4095
#endif
#ifdef ESP8266
#define ADC_VALUE_MAX 1024
#endif
#define PAR_1 -0.42
#define PAR_2 1.92
#define PPM_CO2_IN_CLEAR_AIR    397.13

#define RANGE_EXCELLENT_LEVEL 500.0
#define RANGE_POOR_LEVEL 2500.0

#ifdef	ENABLE_NATIVE_HAP
extern "C" {
#include "homeintegration.h"
}
#endif
struct LDRState
{
	bool isOn=true;
	int ldrValue=0;
	double cvalue;
};
enum LDRCMD :uint { Measure, LDRSaveState = 4096 };

class LDRController;

typedef CController<LDRController, LDRState, LDRCMD> LDR;
class LDRController : public LDR
{
public:
	LDRController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(LDRState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	uint16_t meassure();
#ifdef	ENABLE_NATIVE_HAP
	virtual void setup_hap_service();
	static void hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);
	
	virtual void notify_hap();
	bool isDioxide();
	float getRoInCleanAir();
	float readScaled(float val, float a, float b);
	float calculateResistance(int sensorADC);
	float calc_PPM(int val);
	uint8_t air_quality_level(float ppm, uint8_t min, uint8_t max);
#endif
protected:
	uint pin;
	float cvalmin;
	float cvalmax;
	String cfmt;
	
	uint8_t meassure_num;
	float factor = 1.0;
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice;
	homekit_characteristic_t * hap_level;
	String hapservice_type ;
	

#endif
};
DEFINE_CONTROLLER_FACTORY(LDRController)

#endif