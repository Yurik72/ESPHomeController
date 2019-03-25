#ifndef IRController_h
#define IRController_h

#include <Arduino.h>
#include <ArduinoJson.h>

#include "BaseController.h"

#define IRDATANAME_MAXLEN 20
struct IRState
{
	bool isOn = true;
	bool isReceive = false;
	bool isSend = false;
	long irtoken = 0;
	long timetick = 0;

};

struct IRData {
public:
	IRData() { memset(this->name, 0, IRDATANAME_MAXLEN); };
	IRData(IRState& state) : IRData() {
		this->token = state.irtoken;

	}
	void SetState(IRState& state) {
		state.irtoken = this->token;

	}
	long token = 0;

	char name[IRDATANAME_MAXLEN];
};
enum IRCMD :uint { IROnReceive, IRSend, IRSaveReceive, IRSaveState = 4096 };

class IRController;

typedef CController<IRController, IRState, IRCMD> IR;
class IRController : public IR
{
public:
	IRController();
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

	void irsend(IRState sendstate);
	void savepersist(IRState psstate);
	IRData* getdata_byname(String& name);
	String string_rfdata();
	void saveperisttofile();
	void load_persist();
	String getfilename_data();
	static IRData deserializeIRData(JsonObject& json);
	static IRData deserializeIRData(String strdata);
	static String serializeIRData(IRData data);
protected:
	uint pin;
	uint pinsend;
private:

	bool store_recdata;
	CSimpleArray< IRData> persistdata;
};

DEFINE_CONTROLLER_FACTORY(IRController)
#endif