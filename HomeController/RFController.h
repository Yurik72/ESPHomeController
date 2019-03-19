#ifndef RFController_h
#define RFController_h

#include <Arduino.h>
#include <ArduinoJson.h>

#include "BaseController.h"

#define RFDATANAME_MAXLEN 20
struct RFState
{
	bool isOn = true;
	bool isReceive = false;
	bool isSend = false;
	long rftoken = 0;
	int  rfprotocol = 0;
	int  rfdatalen = 0;
	int  rfdelay = 0;
	long timetick = 0;

};

struct RFData {
public:
	RFData() { memset(this->name, 0, RFDATANAME_MAXLEN); };
	RFData(RFState& state): RFData() {
		this->token = state.rftoken;
		this->protocol = state.rfprotocol;
		this->len = state.rfdatalen;
		this->pulse = state.rfdelay;
	}
	void SetState(RFState& state) {
		 state.rftoken= this->token;
		 state.rfprotocol = this->protocol;
		 state.rfdatalen=this->len ;
		 state.rfdelay =this->pulse ;
	}
	long token = 0;
	int protocol = 1;
	int len = 0;
	int pulse = 0;
	char name[RFDATANAME_MAXLEN];
};
enum RFCMD :uint { OnReceive,Send,SaveReceive, RFSaveState = 4096 };
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

	void rfsend(RFState sendstate);
	void savepersist(RFState psstate);
	RFData* getdata_byname(String& name);
	String string_rfdata();
	void saveperisttofile();
	void load_persist();
	String getfilename_data();
	static RFData deserializeRFData(JsonObject& json);
	static RFData deserializeRFData(String strdata);
	static String serializeRFData(RFData data);
protected:
	uint pin;
	uint pinsend;
private:
	RCSwitch* pSwitch;
	bool store_recdata;
	CSimpleArray< RFData> persistdata;
};

DEFINE_CONTROLLER_FACTORY(RFController)
#endif