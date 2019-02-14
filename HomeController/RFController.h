#ifndef RFController_h
#define RFController_h

#include <Arduino.h>
#include <ArduinoJson.h>

#include "BaseController.h"


struct RFState
{
	bool isOn = true;
	bool isReceive = false;
	bool isSend = false;
	long lastReceive = 0;;
	long lastticks = 0;

};
enum RFCMD :uint { OnReceive,Send, RFSaveState = 4096 };
class RCSwitch;
class RFController;

typedef CController<RFController, RFState, RFCMD> RF;
class RFController : public RF
{
public:
	RFController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void run();
protected:
	uint pin;
private:
	RCSwitch* pSwitch;
};


#endif