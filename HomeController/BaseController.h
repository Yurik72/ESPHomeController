
#ifndef BaseController_h
#define BaseController_h

#include <Arduino.h>
#include <ArduinoJson.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#else
#include <WebServer.h>
#include <ESPmDNS.h>
#endif

#include "Array.h"
#define MAXLEN_NAME 30
#if !defined(ESP8266)
void runcore(void*param);
#endif
class CBaseController
{
public:
	CBaseController();
	virtual String  serializestate() = 0;
	virtual bool  deserializestate(String jsonstate)=0;
	virtual void setup() ;
	const char* get_name() {
		return name;
	}
	void set_name(const char* name);
	bool shouldRun(unsigned long time);

	bool shouldRun() { return shouldRun(millis()); };
	void runned(unsigned long time);

	// Default is to mark it runned "now"
	void runned() { runned(millis()); }
	virtual void run();
	bool isenabled() { return enabled; }
#if defined(ESP8266)
	virtual void setuphandlers(ESP8266WebServer& server);
#else
	virtual void setuphandlers(WebServer& server);
	
#endif
public:
	
	virtual void loadconfig(JsonObject& json);

	void loadconfigbase(JsonObject& json);
	void (*onstatechanged)(CBaseController *);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual void onmqqtmessage(String topic, String payload);
	bool get_iscore() { return isCore; };
	short get_core() { return core; };
	short get_priority() { return priority; };
protected:
	
	char name[MAXLEN_NAME];
	unsigned long interval;
	// Last runned time in Ms
private:
	unsigned long last_run;
	// Scheduled run in Ms (MUST BE CACHED) 
	unsigned long _cached_next_run;
	bool enabled;
	bool isCore;
	short core;
	short priority;
#if !defined(ESP8266)
	TaskHandle_t taskhandle;
#endif
};
template<class T,typename P,typename M>
class CController:public CBaseController
{

public:
	struct command
	{
		M mode;
		P   state;
	};
	int AddCommand(P state, M mode)
	{
		command cmd = { mode,state };
		commands.Add(cmd);
		return commands.GetSize();
	};

	virtual void  SerilizeState() {
		//T::SerilizeState();
	}
	virtual void set_state(P state) {
		this->state = state;
		if (onstatechanged != NULL)
			onstatechanged(this);
	}
	const P& get_state() {
		return state;
	}

protected:
	CSimpleArray<command> commands;
	P state;


};


#endif