#ifndef ServoHVController_h
#define ServoHVController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"


struct ServoHVState
{
	bool isOn = true;
	int ldrValue = 0;
};
enum ServoHVCMD :uint { Measure, LDRSaveState = 4096 };

class ServoHVController;

typedef CController<ServoHVController, ServoHVState, ServoHVCMD> Servo;
class ServoHVController : public Servo
{
public:
	ServoHVController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(ServoHVState state);

protected:
	uint pin;
};


DEFINE_CONTROLLER_FACTORY(ServoHVController)

#endif#pragma once
