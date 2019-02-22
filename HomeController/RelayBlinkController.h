#ifndef RelayBlinkController_h
#define RelayBlinkController_h
#include <Arduino.h>
#include <ArduinoJson.h>

#include "RelayController.h"

class RelayBlinkController :public RelayController {
public:
	RelayBlinkController();
	virtual void loadconfig(JsonObject& json);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(RelayState state);
protected:
	long mask;
	int masklen;
	int duration;
private:
	long nextms;
	int position;
};
#endif
