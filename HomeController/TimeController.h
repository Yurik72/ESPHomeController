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
	time_t time_withoffs;

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
	virtual void setup_after_wifi();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(TimeState state);
#if defined(ESP8266)
	NTPClient* ptimeClient;
#endif
protected:
	void check_restart();
	bool is_shortwakeup();
	bool is_shortwakeup_next();
	long  gmtOffset_sec ;
	int   daylightOffset_sec;
	String ntpServer;
private:
	bool enablesleep;
	unsigned long sleepinterval;
	unsigned long nextsleep;
	unsigned long offsetwakeup;
	unsigned long offsetshortwakeup;
	unsigned long restartinterval;
	unsigned long nextrestart;
	uint8_t sleeptype;
	uint8_t btnwakeup;
	bool is_sleepstarted;
	uint8_t numshortsleeps;
	uint16_t sleepnumber;
	///uint pin;
};
DEFINE_CONTROLLER_FACTORY(TimeController)
#endif
