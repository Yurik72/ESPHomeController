#ifndef RelayDimController_h
#define RelayDimController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define PWM_FREQ 50 //PWM requency 
#define PWM_BIT 16  // PWM bits
#define DIM_MIN_VAL 0
#define DIM_MAX_VAL 0xFF
#define DIM_MIN_PULSE_WIDTH       500     // the shortest pulse sent to a servo  
#define DIM_MAX_PULSE_WIDTH      2500     // the longest pulse sent to a servo 
#define DIM_MAX_FROM_BITS(bits)  0x1<<bits 

#define DIM_FREQ 5000
#define DIM_RESOLUTION 8

#define CALC_VAL(val,invert) map(constrain(((!invert)?val:(DIM_MAX_VAL-val)),0,DIM_MAX_VAL),DIM_MIN_VAL,DIM_MAX_VAL,DIM_MIN_PULSE_WIDTH,DIM_MAX_FROM_BITS(PWM_BIT))

#define DIMCALC_VAL(val,invert) constrain(((!invert)?val:(DIM_MAX_VAL-val)),DIM_MIN_VAL,DIM_MAX_VAL)

struct RelayDimState
{
	bool isOn = false;
	bool isLdr = false;
	int brightness = 0;
	int ldrValue = 0;
};
enum RelayDimCMD {
	DimRelayOn = BaseOn,
	DimRelayOff = BaseOff,
	DimSwitch = 4,
	DimSet = 8,
	DimSetBrigthness = 16,
	DimSetLdrVal = 32,
	DimSetIsLdr = 128,
	DimRelayRestore = 2048
};

class RelayDimController;
typedef CManualStateController<RelayDimController, RelayDimState, RelayDimCMD> RelayDim;
class RelayDimController : public RelayDim
{
public:
	RelayDimController();
	~RelayDimController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	virtual void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(RelayDimState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	void setBrightness(uint8_t br);
	int getLDRBrightness(int brigtness, int ldrval);
	virtual void onmqqtmessage(String topic, String payload);
	virtual bool onpublishmqttex(String& endkey, String& payload, int topicnr);
protected:
	uint pin;
	bool isinvert;
#ifdef ESP32
	int channel;
#endif
	CSmoothVal* pSmooth;
	bool isEnableSmooth;
};
DEFINE_CONTROLLER_FACTORY(RelayDimController)

#endif