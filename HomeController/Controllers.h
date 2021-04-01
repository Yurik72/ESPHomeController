#ifndef Controllers_h
#define Controllers_h
#include "Array.h"
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#else
#include <WebServer.h>
#include <ESPmDNS.h>
#endif
#endif

#ifdef ENABLE_HOMEBRIDGE
#include <Ticker.h>
#include <AsyncMqttClient.h> 
#endif

#if defined ASYNC_WEBSERVER
#if defined(ESP8266)
#include <ESPAsyncWebServer.h>
#else
#include <ESPAsyncWebServer.h>
#endif 
#endif

#define DELAY_MS_RECONNECT 300000 //5 min
#include "BaseController.h"
#include "TimeController.h"
class Triggers;
class CBaseController;
#ifdef ENABLE_NATIVE_HAP
void init_hap_storage();
void storage_changed(char * szstorage,int size);
#endif

enum controllers_setup_phase
{
	setup_phase_none = 0,
	setup_phase_before_wifi = 1,
	setup_phase_after_wifi = 2
};

class Controllers :public CSimpleArray< CBaseController*>
{
public:
	Controllers();
	static Controllers* getInstance();
	static CBaseController* CreateByName(const char* name);
	CBaseController* GetByName(const char* name);
	void setup_before_wifi();
	void setup();
	void handleloops();
	void connectmqtt();
	void onWifiDisconnect();
	bool get_isWifiConnected() { return isWifiConnected; };
	void set_isneedreconnectwifi(bool val);
	void set_monitor_state(uint channel, bool isOn, long mask = 0b10, uint masklen = 2, uint duration = 100);
	virtual void raise_event(CBaseController* pSender,ControllerEvent evt, uint16_t evData);
	void setTimeCtl(TimeController* p) { pTimeCtl = p; };
	TimeController* getTimeCtl() { return  pTimeCtl; };
	void run_trigger_for(CBaseController* ctl);
	
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
	void setuphandlers(ESP8266WebServer& server);
#else
	void setuphandlers(WebServer& server);
#endif
#endif
#if defined ASYNC_WEBSERVER
	void setuphandlers(AsyncWebServer& server);
#endif
private:
	void loadconfig();
	void checkandreconnectWifi();
	
	Triggers& triggers;
	long lastWifiReconnectms;
	bool isConnectingMode;
	bool isWifiConnected = true;
	bool isneedreconnectwifi = true;
	CBaseController* pMonitor=NULL;
	controllers_setup_phase setupphase;
	TimeController* pTimeCtl;
#if defined(ESP8266)
	bool bLoopSwitch=false;
#endif
};
void onstatechanged(CBaseController *);
#ifdef ENABLE_HOMEBRIDGE
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total);
void realconnectToMqtt();
#endif
#endif