#ifndef MotionController_h
#define MotionController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif

struct MotionState
{
	bool isOn=false;
	bool isTriggered = false;
	long tmTrigger = 0;
};
enum MotionCMD {
	MotionOn=BaseOn,
	MotionOff= BaseOff,
	MotionSwitch=4,
	MotionSet = 8,
	MotionRestore = 2048
};
class MotionController;
typedef CController<MotionController, MotionState, MotionCMD> Motion;
class MotionController: public Motion
{
public:
	MotionController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	virtual void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(MotionState state);
	
	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual void onmqqtmessage(String topic, String payload);
#ifdef	ENABLE_NATIVE_HAP
	virtual void setup_hap_service();
	static void hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);

	virtual void notify_hap();

#endif
protected:
	uint8_t pin;

#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice;
#endif
};
DEFINE_CONTROLLER_FACTORY(MotionController)

#endif