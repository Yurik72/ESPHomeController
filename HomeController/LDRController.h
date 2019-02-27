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
protected:
	uint pin;
};
DEFINE_CONTROLLER_FACTORY(LDRController)

#endif