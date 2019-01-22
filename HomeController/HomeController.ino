// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       HomeController.ino
    Created:	28.12.2018 13:18:36
    Author:     OLIK-PC\Olik
*/




#include "config.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>          
#else
#include <WiFi.h>          
#endif
#include <DNSServer.h>


#if defined(ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#else
#include <WebServer.h>
#include <ESPmDNS.h>
#endif
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager

#include <WiFiClient.h>
// ***************************************************************************
// Instanciate HTTP(80) 
// ***************************************************************************
#if defined(ESP8266)
 ESP8266WebServer server(80);
#else
 WebServer server(80);
#endif
#ifdef ENABLE_HOMEBRIDGE
#include <AsyncMqttClient.h>
#endif
#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif
#include <ArduinoJson.h>  
#include "spiffs_webserver.h"
#include "Utilities.h"

#ifdef HTTP_OTA
#if defined(ESP8266)
#include <ESP8266HTTPUpdateServer.h>
 ESP8266HTTPUpdateServer httpUpdater;
#else
#include "ESP32HTTPUpdateServer.h"
 ESP32HTTPUpdateServer httpUpdater;
#endif
#endif



 bool shouldSaveConfig = false;
////now main function
#include "Controllers.h"
#include "Triggers.h"
 Controllers controllers;

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
	wifi_station_set_hostname(const_cast<char*>(HOSTNAME));
#else
	WiFi.setHostname(HOSTNAME);
#endif
	WiFiManager wifiManager;  
	wifiManager.setAPCallback(configModeCallback);
	WiFiManagerParameter local_host(name_localhost_host, "Local hostname", HOSTNAME, 64);

#if defined ENABLE_HOMEBRIDGE
	//set config save notify callback
	
	WiFiManagerParameter hb_mqtt_host("host", "MQTT hostname", mqtt_host, 64);
	WiFiManagerParameter hb_mqtt_port("port", "MQTT port", mqtt_port, 6);
	WiFiManagerParameter hb_mqtt_user("user", "MQTT user", mqtt_user, 32);
	WiFiManagerParameter hb_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32);
	//add all your parameters here
	wifiManager.addParameter(&hb_mqtt_host);
	wifiManager.addParameter(&hb_mqtt_port);
	wifiManager.addParameter(&hb_mqtt_user);
	wifiManager.addParameter(&hb_mqtt_pass);
	wifiManager.setSaveConfigCallback(saveConfigCallback);
	

#endif
	wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
	//finally let's wait normal wifi connection
	if (!wifiManager.autoConnect(HOSTNAME)) {
		DBG_OUTPUT_PORT.println("failed to connect and hit timeout");
		//reset and try again, or maybe put it to deep sleep
		ESP.restart();  
		delay(1000);  
	}
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
#ifdef HTTP_OTA
		httpUpdater.setup(&server, "/update");  
#endif

	///finaly start  web server
	server.begin();
	//and start controllers
	controllers.setup();
	controllers.setuphandlers(server);
}

// Add the main program code into the continuous loop() function
void loop()
{
	server.handleClient();   ///handle income http request
	controllers.handleloops(); //handle controller task
}


// gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager) {
	DBG_OUTPUT_PORT.println("Entered config mode");
	DBG_OUTPUT_PORT.println(WiFi.softAPIP());
	//if you used auto generated SSID, print it
	DBG_OUTPUT_PORT.println(myWiFiManager->getConfigPortalSSID());
	//entered config mode, make led toggle faster
	
}
void saveConfigCallback() {
	shouldSaveConfig = true;
}
 