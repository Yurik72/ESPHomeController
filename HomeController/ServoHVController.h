#ifndef ServoHVController_h
#define ServoHVController_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"
/*
* Different servos require different pulse widths to vary servo angle, but the range is
* an approximately 500 - 2500 microsecond pulse every 20ms(50Hz).In general, hobbyist servos
* sweep 180 degrees, so the lowest number in the published range for a particular servo
* represents an angle of 0 degrees, the middle of the range represents 90 degrees, and the top
* of the range represents 180 degrees.So for example, if the range is 1000us to 2000us,
*1000us would equal an angle of 0, 1500us would equal 90 degrees, and 2000us would equal 180
* degrees.We vary pulse width(recall that the pulse period is already set to 20ms) as follows :
*/
#define MIN_PULSE_WIDTH       500     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      2500     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define MIN_SERVO_VAL 0
#define MAX_SERVO_VAL 180
#define SERVO_BITS 16
#define SERVO_FREQ 50

#define CALC_SERVO_PULSE(val) map(val,MIN_SERVO_VAL,MAX_SERVO_VAL,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH)
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
	int minH;
	int maxH;
	int minV;
	int maxV;
};


DEFINE_CONTROLLER_FACTORY(ServoHVController)

#endif
