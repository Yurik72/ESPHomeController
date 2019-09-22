#ifndef Triggers_h
#define Triggers_h_h
#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Array.h"
#include "BaseController.h"
#include "RFController.h"
#include "RelayDimController.h"
#include "DallasController.h"
#include "BME280Controller.h"
#include "OledController.h"
#include "ThingSpeakClient.h"
#include "CWeatherDisplay.h"
#include "ButtonController.h"

#define NEXT_DAY_SEC (1 * 24 * 60 * 60)
#define SEC_TOLLERANCE 1200  //2 min
//forward declaration
class Controllers;
class RGBStripController;
class TimeController;
class LDRController;
class RelayController;
class RFController;
class RelayDimController;
class BME280Controller;
class WeatherDisplayController;
class ButtonController;
class Trigger {
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
struct timerecRelayDim : public timerecOn {

	int brightness = 0;
	bool isLdr = false;
};
class TimeToRelayDimTrigger :public CBaseTimeTrigger<timerecRelayDim> {
public:
	TimeToRelayDimTrigger();
	virtual void loadconfig(JsonObject& json);
protected:
	RelayDimController* get_relayctl() { return pRelayDim; };
	void set_relayctl(RelayDimController *pCtl) { pRelayDim = pCtl; };
private:
	RelayDimController *pRelayDim;
	//void parsetime(JsonObject& json, timerecRGB & rec);


	void dotrigger(timerecRelayDim & rec, Controllers* pctlss);
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

struct RFRecord:public RFData {
	
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
	long last_token;
	CSimpleArray<RFRecord> rfs;
	long delaywait;
};

class DallasToRGBStrip :public TriggerFromService< DallasController, RGBStripController> {
public:
	DallasToRGBStrip();

	virtual void handleloopsvc(DallasController* ps, RGBStripController* pd);
	virtual void loadconfig(JsonObject& json);
protected:
	uint32_t calcColor(float temp);
	
private:
	float temp_min ;
	float temp_max ;
};

class BMEToOled :public TriggerFromService< BME280Controller, OledController> {
public:
	enum DMODE: uint8_t {
		all = 0,
		temp=1,
		hum=2,
		pres=3
	};
	BMEToOled();

	virtual void handleloopsvc(BME280Controller* ps, OledController* pd);
	virtual void loadconfig(JsonObject& json);
	static String format_doublestr(const char* fmt, double val);
protected:
	DMODE mode = all;
private:
	float temp_min;
	float temp_max;
};

class BMEToRGBMatrix :public TriggerFromService< BME280Controller, RGBStripController> {
public:
	enum DMODE : uint8_t {

		temp = 1,
		hum = 2,
		pres = 3,
		all = 4,
		//all_color_random=5,
		all_color_colorwheel = 5,
		max = 6
	};
	BMEToRGBMatrix();

	virtual void handleloopsvc(BME280Controller* ps, RGBStripController* pd);
	virtual void loadconfig(JsonObject& json);
	
protected:
	DMODE mode = temp;
	String get_temp_text(double val);
	String get_humidity_text(double val);
	String get_pressure_text(double val);
};
class BMEToThingSpeak :public TriggerFromService< BME280Controller, ThingSpeakController> {
public:

	BMEToThingSpeak();

	virtual void handleloopsvc(BME280Controller* ps, ThingSpeakController* pd);
	virtual void loadconfig(JsonObject& json);

protected:
private:
	uint8_t t_ch, h_ch, p_ch;
};
class BMEToWeatherDisplay :public TriggerFromService< BME280Controller, WeatherDisplayController> {
public:
	enum DMODE : uint8_t {

		temp = 1,
		hum = 2,
		pres = 3
	};
	BMEToWeatherDisplay();

	virtual void handleloopsvc(BME280Controller* ps, WeatherDisplayController* pd);
	virtual void loadconfig(JsonObject& json);

protected:
private:
	uint8_t t_ch, h_ch, p_ch;
};

class TimeToWeatherDisplay :public TriggerFromService< TimeController, WeatherDisplayController> {
public:

	TimeToWeatherDisplay();

	virtual void handleloopsvc(TimeController* ps, WeatherDisplayController* pd);
	virtual void loadconfig(JsonObject& json);

protected:
private:

};
class WeatherForecastToWeatherDisplay :public TriggerFromService< WeatherClientController, WeatherDisplayController> {
public:

	WeatherForecastToWeatherDisplay();

	virtual void handleloopsvc(WeatherClientController* ps, WeatherDisplayController* pd);
	virtual void loadconfig(JsonObject& json);

protected:
	unsigned long last_load = 0;
private:

};
class ButtonToWeatherDisplay :public TriggerFromService< ButtonController, WeatherDisplayController> {
public:

	ButtonToWeatherDisplay();

	virtual void handleloopsvc(ButtonController* ps, WeatherDisplayController* pd);
	virtual void loadconfig(JsonObject& json);

protected:
private:
	long lasttriggered = 0;
	uint8_t idx = 0;
};

class LDRToThingSpeak :public TriggerFromService< LDRController, ThingSpeakController> {
public:
	LDRToThingSpeak();

	virtual void handleloopsvc(LDRController* ps, ThingSpeakController* pd);
	virtual void loadconfig(JsonObject& json);

protected:

private:
	uint8_t ch;
};
//DEFINE_TRIGGER_FACTORY(TimeToRGBStripTrigger)

DEFINE_TRIGGER_FACTORY(TimeToRGBStripTrigger)
DEFINE_TRIGGER_FACTORY(TimeToRelayTrigger)
DEFINE_TRIGGER_FACTORY(LDRToRelay)
DEFINE_TRIGGER_FACTORY(LDRToRGBStrip)
DEFINE_TRIGGER_FACTORY(RFToRelay)
DEFINE_TRIGGER_FACTORY(TimeToRelayDimTrigger)
DEFINE_TRIGGER_FACTORY(DallasToRGBStrip)
DEFINE_TRIGGER_FACTORY(BMEToOled)
DEFINE_TRIGGER_FACTORY(BMEToRGBMatrix)
DEFINE_TRIGGER_FACTORY(BMEToThingSpeak)
DEFINE_TRIGGER_FACTORY(BMEToWeatherDisplay)
DEFINE_TRIGGER_FACTORY(TimeToWeatherDisplay)
DEFINE_TRIGGER_FACTORY(WeatherForecastToWeatherDisplay)
DEFINE_TRIGGER_FACTORY(ButtonToWeatherDisplay)
DEFINE_TRIGGER_FACTORY(LDRToThingSpeak)

//DEFINE_TRIGGER_FACTORY(LDRToRelay)
//DEFINE_TRIGGER_FACTORY(LDRToRGBStrip)
//DEFINE_TRIGGER_FACTORY(RFToRelay)

#endif
