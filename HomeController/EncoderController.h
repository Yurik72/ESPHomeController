#ifndef EncoderController_h
#define EncoderController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define ENCODER_BUTTON_ADJUST_MASK      0b1111000000111111
#define ENCODER_BUTTON_PRESSED_MASK     0b1111000000000000
#define ENCODER_BUTTON_DOWN_MASK        0b0000000000111111


struct EncoderState
{
	bool isPressed = false;
	bool isDown = false;
	bool isLongPressed = false;
	bool isVeryLongPressed = false;
	int rotateDelta = 0;
	unsigned long delta_ms = 0;
	unsigned long button_ms = 0;
};
enum EncoderCMD :uint 
{  
		EncoderSetBtn=1,
		EncoderSetDelta=2,
		EncoderSet = 4
};


class EncoderController;
class CButtonController;
typedef CController<EncoderController, EncoderState, EncoderCMD> Encoder;
class EncoderController : public Encoder
{
public:
	EncoderController();
	~EncoderController();

	virtual void getdefaultconfig(JsonObject& json);
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	virtual void loadconfig(JsonObject& json);
	virtual void run();
	virtual void runcore();
	virtual void set_state(EncoderState state);
// encoder specific
	static void IRAM_ATTR Encoder_ISR(void* arg);
	int16_t readEncoder();
	int16_t encoderChanged();

protected:
	uint pin;
	uint pinA;
	uint pinB;
	bool isinvert;
	bool isEnabled;
	uint16_t btnhistory;

private:
	void handleButtonState();
	volatile int16_t encoder0Pos = 0;
	int16_t _minEncoderValue ;
	int16_t _maxEncoderValue ;
	bool _circleValues = false;
	int8_t enc_states[16] = { 0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0 };
	uint8_t old_AB;
	int16_t lastReadEncoder0Pos=0;
	uint8_t encoderSteps;

};
DEFINE_CONTROLLER_FACTORY(EncoderController)

#endif