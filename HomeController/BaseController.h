
#ifndef BaseController_h
#define BaseController_h

#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#else
#include <WebServer.h>
#include <ESPmDNS.h>
#endif
#endif
#include "Utilities.h"
#include "Array.h"
#define MAXLEN_NAME 30
#if !defined(ESP8266)
void runcore(void*param);
#endif

#if defined ASYNC_WEBSERVER
#include <ESPAsyncWebServer.h>
#endif
enum CmdSource
{
	srcState = 0,
	srcTrigger = 1,
	srcMQTT = 2,
	srcSelf = 3,
	srcRestore=4
};
enum BaseCMD :uint {
	BaseOn=1,
	BaseOff=2,
	BaseSetRestore = 2048,
	BaseSaveState  =4096
};
class CBaseController
{
public:
	CBaseController();
	friend class Controllers;
	virtual String  serializestate() = 0;
	virtual bool  deserializestate(String jsonstate, CmdSource src= srcState)=0;
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
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
	virtual void setuphandlers(ESP8266WebServer& server);
#else
	virtual void setuphandlers(WebServer& server);
	
#endif
#endif

#if defined ASYNC_WEBSERVER
	virtual void setuphandlers(AsyncWebServer& server);
#endif
public:
	
	virtual void loadconfig(JsonObject& json);

	void loadconfigbase(JsonObject& json);
	void (*onstatechanged)(CBaseController *);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	virtual bool onpublishmqttex(String& endkey, String& payload,int topicnr) { return false; };
	virtual void onmqqtmessage(String topic, String payload);
	bool get_iscore() { return isCore; };
	short get_core() { return core; };
	short get_priority() { return priority; };
	virtual bool ispersiststate() { return false; }
	virtual void savestate() ;
	virtual bool loadstate()=0;
	String get_filename_state();
	virtual void set_power_on() {};
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
	void cleanbuffer();
	uint8_t * allocatebuffer(size_t size);
	uint8_t * bodybuffer; ///handle collection of post request
	size_t bodyindex; 
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
	virtual int AddCommand(P state, M mode, CmdSource src) {

		command cmd = { mode,state };

		commands.Add(cmd);
		if (this->ispersiststate() && (src == srcState || src == srcMQTT)) {
			command savecmd = {(M) BaseSaveState, state };
			DBG_OUTPUT_PORT.println("AddCommand->SaveState");
			commands.Add(savecmd);
		}
		return commands.GetSize();
	}
	virtual bool baseprocesscommands(command cmd) {
		if (cmd.mode == BaseSaveState) {
			
			this->savestate();
			return true;
		}
		return false;
	}
	//virtual void  SerilizeState() {
		//T::SerilizeState();
	//}
	virtual void set_state(P state) {
		this->state = state;
		
		if (onstatechanged != NULL) {
			
			onstatechanged(this);
		}
		
	}
	const P& get_state() {
		return state;
	}
	virtual bool loadstate() {
		return this->deserializestate(readfile(this->get_filename_state().c_str()), srcRestore);
	}
protected:
	CSimpleArray<command> commands;
	P state;


};
template<class T, typename P, typename M>
class CManualStateController : public CController<T, P, M>
{
public:
	
	CManualStateController() {
		this->manualtime = 0;
		this->mswhenrestore = 0;
		this->isrestoreactivated = false;
	}
	virtual void set_prevstate(P& state) {
		prevState = state;
	}
	virtual const P&  get_prevstate() {
		return prevState ;
	}
	virtual void loadconfig(JsonObject& json) {
		this->manualtime = json["manualtime"];
		CController<T, P, M>::loadconfig(json);
	}
	virtual int AddCommand(P state, M mode, CmdSource src) {
		if (src == srcState) { //save state
			P saved = this->get_state();
			this->set_prevstate(saved);
			if (this->manualtime != 0) {
				DBG_OUTPUT_PORT.println(this->get_name());
				DBG_OUTPUT_PORT.print("Activate manual time");
				
				this->mswhenrestore = millis() + this->manualtime * 1000;//wil be handled in next(if manualtime =0 , never)
				DBG_OUTPUT_PORT.println(this->mswhenrestore);
				DBG_OUTPUT_PORT.println("Current");
				DBG_OUTPUT_PORT.println(millis());
				this->isrestoreactivated = true;
			}
		}
		return CController<T, P, M>::AddCommand(state,mode,src);
	}
	virtual void run() {
		CController<T, P, M>::run();

		if (this->isrestoreactivated && this->mswhenrestore <= millis()) { // need restore
			DBG_OUTPUT_PORT.print(this->get_name());
			DBG_OUTPUT_PORT.println(" : Restore after manual set");
			this->restorestate();
		}
	}
	virtual void restorestate() {
		this->isrestoreactivated = false;
		P saved = this->get_prevstate();
		M val = (M)(int)BaseSetRestore;
		this->AddCommand(saved, val, srcSelf);
	}
	virtual void set_power_on() {
		P current= this->get_state();
		
		M val = (M)(int)BaseOn;
		
		this->AddCommand(current, val, srcSelf);
	};
protected: 
	P prevState;
	int manualtime;
	int mswhenrestore;
	bool isrestoreactivated;
};


#endif