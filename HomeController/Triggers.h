#ifndef Triggers_h
#define Triggers_h_h
#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Array.h"

#define NEXT_DAY_SEC (1 * 24 * 60 * 60)
#define SEC_TOLLERANCE 1200  //2 min
//forward declaration
class Controllers;
class RGBStripController;
class TimeController;
class LDRController;
class RelayController;
class RFController;
class Trigger{
public:
	Trigger() {};
	friend class Triggers;
public:
	virtual void loadconfig(JsonObject& json);
	const char* get_src() { return src.c_str(); };
	virtual void handleloop(CBaseController*pBase, Controllers* pctls);
	

protected:
	String src;
	String dst;
	String type;
};
template<class SRC,class DST>
class TriggerFromService :public Trigger {

public :
	
	TriggerFromService();
	virtual void handleloop(CBaseController* pBase, Controllers* pctls);
	virtual void handleloopsvc(SRC* ps, DST* pd);
	DST* get_dstctl() { return pDst; };
	SRC* get_srcctl() { return pSrc; };

protected:
	void set_dstctl(DST *pCtl) { pDst = pCtl; };
	void set_srcctl(SRC *pCtl) { pSrc = pCtl; };
private:
	SRC *pSrc;
	DST *pDst;
};
class Triggers :public CSimpleArray< Trigger*>
{
public:
	Triggers() {};
	static Trigger* CreateByType(const char* nametype);
	void setup();

	void loadconfig();


};

enum TimeType :uint {
	dailly = 0
};
template<typename TM>
class CBaseTimeTrigger :public Trigger {

public:
	CBaseTimeTrigger();
	virtual void parsetime(JsonObject& json, TM & rec);
	virtual void processrecord(time_t currentTime, TM& rec, Controllers* pctlss);
	virtual void handleloop(CBaseController*pBase, Controllers* pctls);
	long get_timeoffs() { return timeoffs; };
	TimeController* get_timectl();
	void set_timectl(TimeController *pCtl);
	void set_timeoffs(long val) { timeoffs = val; };
protected:
	virtual void dotrigger(TM & rec, Controllers* pctlss) = 0;
	bool istimetotrigger(time_t time, time_t currentTime);
	CSimpleArray<TM> times;
private :
	long timeoffs;
	TimeController* pTime;
};
///time to rgb
struct timerecOn {
	int time = 0;
	bool isOn = false;
	TimeType timetype = dailly;
	time_t timeToTrigger = 0;
	time_t lastTriggered = 0;
};
struct timerecRGB: public timerecOn {
	//int time = 0;
	
	int brightness = 0;
	int wxmode = 0;
//	bool isOn = false;
	uint color;
//	TimeType timetype = dailly;
//	time_t timeToTrigger=0;
//	time_t lastTriggered=0;
	bool isLdr = false;
};
class TimeToRGBStripTrigger :public CBaseTimeTrigger<timerecRGB> {
public:
	TimeToRGBStripTrigger();
	virtual void loadconfig(JsonObject& json);
protected:
	RGBStripController* get_stripctl() { return pStrip; };
	void set_stripctl(RGBStripController *pCtl) { pStrip = pCtl; };
private:
	RGBStripController *pStrip;
	//void parsetime(JsonObject& json, timerecRGB & rec);
	
	
	void dotrigger(timerecRGB & rec, Controllers* pctlss);
};

class TimeToRelayTrigger :public CBaseTimeTrigger<timerecOn> {
public:
	TimeToRelayTrigger();
	virtual void loadconfig(JsonObject& json);
protected:
	RelayController* get_relayctl() { return pRelay; };
	void set_relayctl(RelayController *pCtl) { pRelay = pCtl; };
private:
	RelayController *pRelay;
	//void parsetime(JsonObject& json, timerecRGB & rec);


	void dotrigger(timerecOn & rec, Controllers* pctlss);
};
class LDRToRGBStrip :public TriggerFromService< LDRController, RGBStripController>{
public:
	LDRToRGBStrip();

	virtual void handleloopsvc(LDRController* ps, RGBStripController* pd);

protected:

private:

};
class LDRToRelay :public TriggerFromService< LDRController, RelayController> {
public:
	LDRToRelay();

	virtual void handleloopsvc(LDRController* ps, RelayController* pd);
	virtual void loadconfig(JsonObject& json);

protected:

private:
	short valueOn;
	short valueOff;
};

struct RFRecord {
	long rfkey = -1;
	bool isswitch = true;
	bool isOn = true;
};
class RFToRelay :public TriggerFromService< RFController, RelayController> {
public:
	RFToRelay();

	virtual void handleloopsvc(RFController* ps, RelayController* pd);
	virtual void loadconfig(JsonObject& json);
	void processrecord(RFRecord& rec, RelayController* pr);
protected:

private:
	long last_tick;
	CSimpleArray<RFRecord> rfs;
};
#endif
