#ifndef LDRController_h
#define LDRController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"


struct LDRState
{
	bool isOn=true;
	int ldrValue=0;
};
enum LDRCMD { Measure };
class LDRController;
typedef CController<LDRController, LDRState, LDRCMD> LDR;
class LDRController : public LDR
{
public:
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(LDRState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
protected:
	uint pin;
};


#endif