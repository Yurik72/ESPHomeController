#ifndef RelayController_h
#define RelayController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"


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
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(RelayState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual void onmqqtmessage(String topic, String payload);
protected:
	uint pin;
	bool isinvert;
};


#endif