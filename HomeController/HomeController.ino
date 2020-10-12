// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       HomeController.ino
    Created:	28.12.2018 13:18:36
    Author:     Yurik72
*/
#define ESPHOMECONTROLLER

#include <Arduino.h>
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
//#include <ESPAsyncDNSServer.h>
#endif

#include <WiFiClient.h>
// ***************************************************************************
// Instanciate HTTP(80) 
// ***************************************************************************
#if defined ASYNC_WEBSERVER
#include "ESPAsyncWiFiManager.h"
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


#ifndef ASYNC_WEBSERVER
#if defined(HTTP_OTA) 
#if defined(ESP8266)
#include <ESP8266HTTPUpdateServer.h>
 ESP8266HTTPUpdateServer httpUpdater;
#else
#include "ESP32HTTPUpdateServer.h"
 ESP32HTTPUpdateServer httpUpdater;
#endif
#endif
#endif

#if defined(HTTP_OTA) && defined(ASYNC_WEBSERVER)
#include "ESPAsyncUpdateServer.h"
#endif

#if defined(HTTP_OTA) && defined(ASYNC_WEBSERVER)
 ESPAsyncHTTPUpdateServer httpUpdater;
#endif

#if !defined ASYNC_WEBSERVER
#include "spiffs_webserver.h"
#else
#include "spiffs_webserver_async.h"
#endif

#if defined ASYNC_WEBSERVER


 AsyncWiFiManager* pwifiManager=NULL;
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
//void startwifimanager();
 bool wifidirectconnect();
 void startwifimanager();
// The setup() function runs once each time the micro-controller starts

 void setup_after_wifi() {
	
	
	 if (WiFi.getMode() != WIFI_STA ) {
		 DBG_OUTPUT_PORT.println("Wifi is not connected or not STA, stop initilizing");
		 return;
	 }
#ifndef ENABLE_NATIVE_HAP
	 DBG_OUTPUT_PORT.print("Starting MDNS  host:");
	 DBG_OUTPUT_PORT.println(HOSTNAME);
	 if (!isAPMode) {
		 if (MDNS.begin(HOSTNAME)) {
			 DBG_OUTPUT_PORT.println("MDNS responder started");
			 MDNS.addService("_http", "_tcp", 80);
		 }

	 }
#endif

#if !defined(ESP8266)
	 const String FILES[] = { "/index.html", "/js/bundle.min.js.gz","/filebrowse.html" };//"/filebrowse.html"
#else
	 const String FILES[] = { "/index.html", "/filebrowse.html" };//"/filebrowse.html"
#endif
#if !defined(ESP8266)
	 if (!isAPMode) {
		 for (int i = 0; i < sizeof(FILES) / sizeof(*FILES); i++)
			 check_anddownloadfile(szfilesourceroot, FILES[i]);
		 //	downloadnotexistingfiles(szfilesourceroot, filestodownload);
	 }
#endif
	 //if (!isAPMode) {
	 SETUP_FILEHANDLES  ///setup file browser
 //}
#if defined(ASYNC_WEBSERVER)
		 setExternRebootFlag(&isReboot);
#endif
	 // ***************************************************************************
	 // Setup: update server
	 // ***************************************************************************
#if defined (HTTP_OTA) && !defined(ASYNC_WEBSERVER)
	 httpUpdater.setExternRebootFlag(&isReboot);
	 httpUpdater.setup(&server, "/update");
#endif

#if defined (HTTP_OTA) && defined(ASYNC_WEBSERVER)
	 if (!isAPMode) {
		 httpUpdater.setExternRebootFlag(&isReboot);
		 httpUpdater.setup(asserver, "/update");
	 }
#endif

	 ///finaly start  web server
#if !defined ASYNC_WEBSERVER
	 server.begin();
#endif

#if defined ASYNC_WEBSERVER
	 if (!isAPMode)
		 asserver.begin();
#endif
	 //and start controllers
	 controllers.setup();
#if !defined ASYNC_WEBSERVER
	 controllers.setuphandlers(server);
#endif

#if defined ASYNC_WEBSERVER
	 //if (!isAPMode)
	 controllers.setuphandlers(asserver);
#endif

 }
 void setup()
 {
	 DBG_OUTPUT_PORT.begin(115200); //setup serial ports


	 // ***************************************************************************
	 // Setup: SPIFFS
	 // ***************************************************************************
#if defined(ESP8266)
	 if (!SPIFFS.begin()) {
		 SPIFFS.format();
	 }
#else
	 if (!SPIFFS.begin(true)) {
		 DBG_OUTPUT_PORT.print("SPIFFS Mount failed");
	 }
#endif
	 
	 //testfs();
	 // ***************************************************************************
	 // Setup: WiFi manager
	 // ***************************************************************************
	 if (!readConfigFS()) {
		 DBG_OUTPUT_PORT.println("Fail To Load config.json ! ");
		 SPIFFS.format();  //compatibility with prev version
	 }
#if defined(ESP8266)
	 DBG_OUTPUT_PORT.print("Setup station name ");
	 DBG_OUTPUT_PORT.println(HOSTNAME);
	 wifi_station_set_hostname(const_cast<char*>(HOSTNAME));
#else
	 WiFi.setHostname(HOSTNAME);
#endif
	 controllers.setup_before_wifi();
	
#if defined(ESP8266)
	 startwifimanager();
#else
	 if (!wifidirectconnect())   
		 startwifimanager();
#endif
	 DBG_OUTPUT_PORT.println("wifi  connected");
	 //setup mdns
	 setup_after_wifi();
}

// Add the main program code into the continuous loop() function
void loop()
{
	if (isReboot) {
		delay(1000);
		ESP.restart();
		return;
	}
	
#if! defined ASYNC_WEBSERVER
	server.handleClient();   ///handle income http request
#endif
	controllers.handleloops(); //handle controller task
}

bool wifidirectconnect() {

	return false;
}
#if defined(ESP8266)
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
	//DBG_OUTPUT_PORT.println("WiFi On Disconnect.");

}
#else
void onWifiDisconnect() {
	//DBG_OUTPUT_PORT.println("WiFi On Disconnect.");
	controllers.onWifiDisconnect();
}

#endif
#if defined(ESP8266)
void onWifiConnect(const WiFiEventStationModeConnected& event) {
	//DBG_OUTPUT_PORT.println("WiFi On Disconnect.");
	//setup_after_wifi();
}
#else
void onWifiConnect() {
	//DBG_OUTPUT_PORT.println("WiFi On Connect.");
	//setup_after_wifi();
}

#endif
void startwifimanager() {



#if defined ASYNC_WEBSERVER
	DBG_OUTPUT_PORT.println("Setupr DNS ");
	DNSServer dns;
	DBG_OUTPUT_PORT.println("AsyncWiFiManager");

	//AsyncWiFiManager wifiManager(&asserver, &dns);
	pwifiManager = new AsyncWiFiManager(&asserver, &dns);
#else
	WiFiManager wifiManager;
#endif

#if defined ASYNC_WEBSERVER
	pwifiManager->setAPCallback(configModeCallback);
#else
	wifiManager.setAPCallback(configModeCallback);
#endif

#if !defined ASYNC_WEBSERVER
	WiFiManagerParameter local_host(name_localhost_host, "Local hostname", HOSTNAME, 64);
	wifiManager.addParameter(&local_host);
#else
	AsyncWiFiManagerParameter local_host(name_localhost_host, "Local hostname", HOSTNAME, 64);
	pwifiManager->addParameter(&local_host);
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
	pwifiManager->addParameter(&hb_mqtt_host);
	pwifiManager->addParameter(&hb_mqtt_port);
	pwifiManager->addParameter(&hb_mqtt_user);
	pwifiManager->addParameter(&hb_mqtt_pass);

#endif	
#endif
#if! defined ASYNC_WEBSERVER
	wifiManager.setSaveConfigCallback(saveConfigCallback);
	wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
#else
	pwifiManager->setSaveConfigCallback(saveConfigCallback);
	pwifiManager->setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
#endif
	//finally let's wait normal wifi connection
#if defined(ESP8266)
	if (!isAPMode) {
		WiFi.onStationModeDisconnected(onWifiDisconnect);
		WiFi.onStationModeConnected(onWifiConnect);
	}
#else
	WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
		onWifiDisconnect();
	}, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
	WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
		onWifiConnect();
	}, WiFiEvent_t::SYSTEM_EVENT_STA_CONNECTED);
#endif
#if defined ASYNC_WEBSERVER
	if (!pwifiManager->autoConnect(HOSTNAME, NULL, !isOffline)) {
#else
	if (!wifiManager.autoConnect(HOSTNAME, NULL)) {
#endif
		DBG_OUTPUT_PORT.println("failed to connect and hit timeout");
		//reset and try again, or maybe put it to deep sleep
		if (!isOffline) {
			ESP.restart();
			delay(1000);
		}
		else {
			DBG_OUTPUT_PORT.println("Entering offline mode");
			isAPMode = true;
#if defined ASYNC_WEBSERVER
			pwifiManager->startOfflineApp(HOSTNAME, NULL);
#endif
		}

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
	if (!isAPMode) {
		delete pwifiManager;
		pwifiManager = NULL;
	}
	else {
		pwifiManager->cleanParameters();
	}
	
}

// gets called when WiFiManager enters configuration mode
