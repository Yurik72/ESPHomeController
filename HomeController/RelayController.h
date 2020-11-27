#ifndef RelayController_h
#define RelayController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif

struct RelayState
{
	bool isOn=false;
	
};
enum RelayCMD {
	RelayOn=BaseOn,
	RelayOff= BaseOff,
	Switch=4,
	Set=8,
	RelayRestore = 2048
};
class RelayController;
typedef CManualStateController<RelayController, RelayState, RelayCMD> Relay;
class RelayController: public Relay
{
public:
	RelayController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	virtual void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(RelayState state);
	virtual void set_power_on();
	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual void onmqqtmessage(String topic, String payload);
#ifdef	ENABLE_NATIVE_HAP
	virtual void setup_hap_service();
	static void hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);

	virtual void notify_hap();

#endif
protected:
	uint pin;
	bool isinvert;
	bool ispower_on;
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice;
#endif
};
DEFINE_CONTROLLER_FACTORY(RelayController)

#endif