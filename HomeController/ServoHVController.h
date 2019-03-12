#ifndef ServoHVController_h
#define ServoHVController_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"


struct ServoHVState
{
	bool isOn = true;
	int posH = 0;
	int posV = 0;
};
enum ServoHVCMD :uint { ServoSet,ServoSetH,ServoSetV,ServoOff, ServoSaveState = 4096 };

class ServoHVController;

typedef CController<ServoHVController, ServoHVState, ServoHVCMD> Servo;
class ServoHVController : public Servo
{
public:
	ServoHVController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	virtual void loadconfig(JsonObject& json);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(ServoHVState state);

protected:
	int pin;
	int pinV;
	int pinH;
	int channelV;
	int channelH;
	int delayOnms;
	long nextOff;
};


DEFINE_CONTROLLER_FACTORY(ServoHVController)

#endif
