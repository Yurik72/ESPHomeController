#ifndef RelayBlinkController_h
#define RelayBlinkController_h
#include <Arduino.h>
#include <ArduinoJson.h>

//#include "RelayController.h"
#define MAX_RELAY_CHANNELS 3

#define BIT_FROM_POS(k,n) (n & (1 << k)) >> k  // k-th bit of n

struct BlinkData {
	long mask= 0b10;
	uint masklen=2;
	uint duration= 100;
};
struct RelayBlinkState
{
	uint channelnum=0;
	bool isOn[MAX_RELAY_CHANNELS];
	BlinkData data;

};
enum RelayBlinkCMD {
	RelayBlinkOn = BaseOn,
	RelayBlinkOff = BaseOff,
	RelayBlinkSwitch = 4,
	RelayBlinkSet = 8,
	RelayBlinkSetData = 16,
	RelayBlinkRestore = 2048
};
class RelayBlinkController;
typedef CManualStateController<RelayBlinkController, RelayBlinkState, RelayBlinkCMD> RelayBlink;
class RelayBlinkController :public RelayBlink {
public:
	RelayBlinkController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void loadconfig(JsonObject& json);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	virtual void run();
	virtual void set_state(RelayBlinkState state);
	virtual void set_monitor_state(uint channel, bool isOn, long mask = 0b10, uint masklen = 2, uint duration = 100);
protected:
	long mask[MAX_RELAY_CHANNELS];
	uint masklen[MAX_RELAY_CHANNELS];
	uint duration[MAX_RELAY_CHANNELS];
private:
	long nextms[MAX_RELAY_CHANNELS];
	int position[MAX_RELAY_CHANNELS];
	uint pins[MAX_RELAY_CHANNELS];
	uint numchannels;
	bool isinvert;
};
DEFINE_CONTROLLER_FACTORY(RelayBlinkController)
#endif
