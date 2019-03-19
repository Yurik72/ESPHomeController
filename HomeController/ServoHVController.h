#ifndef ServoHVController_h
#define ServoHVController_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define MIN_PULSE_WIDTH       1500     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      8500     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define MIN_SERVO_VAL 0
#define MAX_SERVO_VAL 180
#define SERVO_BITS 16
#define SERVO_FREQ 50

#define CALC_SERVO_PULSE(val) map(val,MIN_SERVO_VAL,MAX_SERVO_VAL,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH)

struct ServoHVState
{
	bool isOn = true;
	int posH = 90;
	int posV = 90;
};
enum ServoHVCMD  { ServoSet,ServoSetH,ServoSetV,ServoOff, ServoSaveState = 4096 };

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
	int minH;
	int maxH;
	int minV;
	int maxV;
};


DEFINE_CONTROLLER_FACTORY(ServoHVController)

#endif
