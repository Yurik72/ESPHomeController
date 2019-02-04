// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       HomeController.ino
    Created:	28.12.2018 13:18:36
    Author:     Yurik72
*/



#include "config.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>          
#else
#include <WiFi.h>          
#endif
#include <DNSServer.h>

#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#endif
#if defined(ESP8266)
#include <ESP8266mDNS.h>
#else
#include <ESPmDNS.h>
#endif

#include <WiFiClient.h>
// ***************************************************************************
// Instanciate HTTP(80) 
// ***************************************************************************
#if defined ASYNC_WEBSERVER
#include <ESPAsyncWiFiManager.h>
#define USE_EADNS
#else
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#endif
#ifdef ENABLE_HOMEBRIDGE
#include <AsyncMqttClient.h>
#endif
#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif
#include <ArduinoJson.h>  

#include "Utilities.h"

#if defined(HTTP_OTA) && !defined(ASYNC_WEBSERVER)
#if defined(ESP8266)
#include <ESP8266HTTPUpdateServer.h>
 ESP8266HTTPUpdateServer httpUpdater;
#else
#include "ESP32HTTPUpdateServer.h"
 ESP32HTTPUpdateServer httpUpdater;
#endif
#endif


#if defined(HTTP_OTA) && defined(ASYNC_WEBSERVER)
#include "ESPAsyncUpdateServer.h"
#endif

#if defined(HTTP_OTA) && defined(ASYNC_WEBSERVER)
 ESPAsyncHTTPUpdateServer httpUpdater;
#endif

#if defined ASYNC_WEBSERVER
#if defined(ESP8266)
 #include <ESPAsyncTCP.h>

 #else
#include <AsyncTCP.h>
#endif 
#include <ESPAsyncWebServer.h>
#endif

#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
 ESP8266WebServer server(80);
#else
WebServer server(80);
#endif
#endif



#if defined ASYNC_WEBSERVER
 AsyncWebServer asserver(80);
#endif

#if !defined ASYNC_WEBSERVER
#include "spiffs_webserver.h"
#else
#include "spiffs_webserver_async.h"
#endif


 bool shouldSaveConfig = false;
////now main function
#include "Controllers.h"
#include "Triggers.h"
 Controllers controllers;


 /// used by WiFi manager
 void saveConfigCallback() {
	 shouldSaveConfig = true;
 }

#if !defined ASYNC_WEBSERVER
 void configModeCallback(WiFiManager *myWiFiManager) {
	 DBG_OUTPUT_PORT.println("Entered config mode");
	 DBG_OUTPUT_PORT.println(WiFi.softAPIP());
	 //if you used auto generated SSID, print it
	 DBG_OUTPUT_PORT.println(myWiFiManager->getConfigPortalSSID());
	 //entered config mode, make led toggle faster

 }
#else

 void configModeCallback(AsyncWiFiManager *myWiFiManager) {
	 DBG_OUTPUT_PORT.println("Entered config mode");
	 DBG_OUTPUT_PORT.println(WiFi.softAPIP());

	 DBG_OUTPUT_PORT.println(myWiFiManager->getConfigPortalSSID());
 }
#endif
 void wificonnect();
// The setup() function runs once each time the micro-controller starts
void setup()
{
	DBG_OUTPUT_PORT.begin(115200); //setup serial ports
	// ***************************************************************************
    // Setup: SPIFFS
    // ***************************************************************************
	SPIFFS.begin();
	//testfs();
	// ***************************************************************************
	// Setup: WiFi manager
	// ***************************************************************************
	if (!readConfigFS())
		DBG_OUTPUT_PORT.println("Fail To Load config.json ! ");
#if defined(ESP8266)
	DBG_OUTPUT_PORT.print("Setup station name ");
	DBG_OUTPUT_PORT.println(HOSTNAME);
	wifi_station_set_hostname(const_cast<char*>(HOSTNAME));
#else
	WiFi.setHostname(HOSTNAME);
#endif

	wificonnect();
	//setup mdns
	DBG_OUTPUT_PORT.print("Starting MDNS  host:");
	DBG_OUTPUT_PORT.println(HOSTNAME);
	if (MDNS.begin(HOSTNAME)) {
		DBG_OUTPUT_PORT.println("MDNS responder started");
	}
	
	SETUP_FILEHANDLES  ///setup file browser

	// ***************************************************************************
	// Setup: update server
	// ***************************************************************************
#if defined (HTTP_OTA) && !defined(ASYNC_WEBSERVER)
		httpUpdater.setup(&server, "/update");  
#endif

#if defined (HTTP_OTA) && defined(ASYNC_WEBSERVER)
	httpUpdater.setup(asserver, "/update");
#endif

	///finaly start  web server
#if !defined ASYNC_WEBSERVER
	server.begin();
#endif

#if defined ASYNC_WEBSERVER
	asserver.begin();
#endif
	//and start controllers
	controllers.setup();
#if !defined ASYNC_WEBSERVER
	controllers.setuphandlers(server);
#endif

#if defined ASYNC_WEBSERVER
	controllers.setuphandlers(asserver);
#endif
}

// Add the main program code into the continuous loop() function
void loop()
{
#if! defined ASYNC_WEBSERVER
	server.handleClient();   ///handle income http request
#endif
	controllers.handleloops(); //handle controller task
	
}


void wificonnect() {
#if defined ASYNC_WEBSERVER
	DBG_OUTPUT_PORT.println("Setupr DNS ");
	DNSServer dns;
	DBG_OUTPUT_PORT.println("AsyncWiFiManager");
	
	AsyncWiFiManager wifiManager(&asserver, &dns);
#else
	WiFiManager wifiManager;
#endif


	wifiManager.setAPCallback(configModeCallback);

#if !defined ASYNC_WEBSERVER
	WiFiManagerParameter local_host(name_localhost_host, "Local hostname", HOSTNAME, 64);
	wifiManager.addParameter(&local_host);
#else
	AsyncWiFiManagerParameter local_host(name_localhost_host, "Local hostname", HOSTNAME, 64);
	wifiManager.addParameter(&local_host);
#endif

#if defined ENABLE_HOMEBRIDGE
	//set config save notify callback
#if! defined ASYNC_WEBSERVER	
	WiFiManagerParameter hb_mqtt_host("host", "MQTT hostname", mqtt_host, 64);
	WiFiManagerParameter hb_mqtt_port("port", "MQTT port", mqtt_port, 6);
	WiFiManagerParameter hb_mqtt_user("user", "MQTT user", mqtt_user, 32);
	WiFiManagerParameter hb_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32);
	//add all your parameters here
	wifiManager.addParameter(&hb_mqtt_host);
	wifiManager.addParameter(&hb_mqtt_port);
	wifiManager.addParameter(&hb_mqtt_user);
	wifiManager.addParameter(&hb_mqtt_pass);

#else
	AsyncWiFiManagerParameter hb_mqtt_host("host", "MQTT hostname", mqtt_host, 64);
	AsyncWiFiManagerParameter hb_mqtt_port("port", "MQTT port", mqtt_port, 6);
	AsyncWiFiManagerParameter hb_mqtt_user("user", "MQTT user", mqtt_user, 32);
	AsyncWiFiManagerParameter hb_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32);
	wifiManager.addParameter(&hb_mqtt_host);
	wifiManager.addParameter(&hb_mqtt_port);
	wifiManager.addParameter(&hb_mqtt_user);
	wifiManager.addParameter(&hb_mqtt_pass);

#endif	
#endif

	wifiManager.setSaveConfigCallback(saveConfigCallback);
	wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
	//finally let's wait normal wifi connection
	if (!wifiManager.autoConnect(HOSTNAME)) {
		DBG_OUTPUT_PORT.println("failed to connect and hit timeout");
		//reset and try again, or maybe put it to deep sleep
		ESP.restart();  
		delay(1000);  
	}

#if ! defined ASYNC_WEBSERVER
	if (shouldSaveConfig) {
		char localHost[32];
		strcpy(localHost, local_host.getValue());
		if (strlen(localHost) > 0);
		strcpy(HOSTNAME, localHost);
#if defined ENABLE_HOMEBRIDGE
		strcpy(mqtt_host, hb_mqtt_host.getValue());
		strcpy(mqtt_port, hb_mqtt_port.getValue());
		strcpy(mqtt_user, hb_mqtt_user.getValue());
		strcpy(mqtt_pass, hb_mqtt_pass.getValue());
#endif
		writeConfigFS(true);

	}
#endif
}

// gets called when WiFiManager enters configuration mode



