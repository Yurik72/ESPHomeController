#ifndef LDRController_h
#define LDRController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

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

#endif
protected:
	uint pin;
	float cvalmin;
	float cvalmax;
	String cfmt;
	uint8_t meassure_num;
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice;
	homekit_characteristic_t * hap_level;
	String hapservice_type ;
	

#endif
};
DEFINE_CONTROLLER_FACTORY(LDRController)

#endif