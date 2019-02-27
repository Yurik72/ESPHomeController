#ifndef TimeController_h
#define TimeController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#if defined(ESP8266)
class NTPClient;
#endif
struct TimeState
{
	time_t time;
};
enum TimeCMD { SET };
class TimeController;
typedef CController<TimeController, TimeState, TimeCMD> TimeCtl;
class TimeController : public TimeCtl
{
public :
	TimeController();
	long get_gmtoffset() { return gmtOffset_sec; };
	int get_daylightOffset() { return daylightOffset_sec; };
	int get_offs() { return gmtOffset_sec + daylightOffset_sec; };

	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(TimeState state);
#if defined(ESP8266)
	NTPClient* ptimeClient;
#endif
protected:
	long  gmtOffset_sec ;
	int   daylightOffset_sec;
	String ntpServer;
	///uint pin;
};
DEFINE_CONTROLLER_FACTORY(TimeController)
#endif
