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
//#include "BaseController.h"

class Triggers;
class CBaseController;

class Controllers :public CSimpleArray< CBaseController*>
{
public:
	Controllers();
	static Controllers* getInstance();
	static CBaseController* CreateByName(const char* name);
	CBaseController* GetByName(const char* name);
	void setup();
	void handleloops();
	void connectmqtt();
	void onWifiDisconnect();
	bool get_isWifiConnected() { return isWifiConnected; };
	void set_isneedreconnectwifi(bool val);
	void set_monitor_state(uint channel, bool isOn, long mask = 0b10, uint masklen = 2, uint duration = 100);
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
};
void onstatechanged(CBaseController *);
#ifdef ENABLE_HOMEBRIDGE
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total);
void realconnectToMqtt();
#endif
#endif