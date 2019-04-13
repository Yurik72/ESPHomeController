#ifndef config_h
#define config_h

#define ENABLE_HOMEBRIDGE    //if defined will communicate to MQTT Home bdridge
#define HTTP_OTA             // If defined, enable ESP8266HTTPUpdateServer/ESP32HTTPUpdateServer/ESPAsyncUpdateServer OTA code.

#define LOG_STRING_LOGGER
#define LOG_SERIAL

#if defined(LOG_SERIAL) && !defined(LOG_STRING_LOGGER)
#define DBG_OUTPUT_PORT Serial  // Set debug output port
#define GET_LOG String("")
#define GET_CONSTCHARGLOG GET_LOG
#endif

#if defined(LOG_SERIAL) && defined(LOG_STRING_LOGGER)
#include "Logger.h"
#define DBG_OUTPUT_PORT ESPLogger  // Set debug output port
#define GET_STRINGLOG ESPLogger.LOG()
#define GET_CONSTCHARGLOG ESPLogger.LOG().c_str()
#endif

#if defined(ESP8266)
#define MAX_LDRVAL 1024
#else
#define MAX_LDRVAL 4095

#endif




#define CONFIG_PORTAL_TIMEOUT 600/// secs , to wait configuration has been done by user
#define ASYNC_WEBSERVER    // !Important , this is switching between WebServer and AsyncWebserver.
						   //For instance given web site implementen on ReactJS produces simultaneous  request
						   //and normal WebServer is not able to do this
						   //Hovewer website will continu to send request in case of error, but this is perfomance
						   // As well browser can send simultaneous requests to resources css,js,....
//#define TRIGGER_DEBUG  //debug triggers output enable
//#define TIMECONTROLLER_DEBUG  //debug timecontroller output enable
//#define LDRCONTROLLER_DEBUG  //debug ldrcontroller output enable
//#define BMECONTROLLER_DEBUG    //debug bme280controller output enable
//#define RF_TRIGGER_DEBUG         //debug rf trigger
//#define RELAYDIM_DEBUG    //debug relay dim
//#define SERVO_DEBUG    //debug relay dim
//#define MQTT_DEBUG
//#define FACTORY_DEBUG
//#define RFCONTROLLER_DEBUG
#define MENU_DEBUG
//#define BUTTON_DEBUG


//DISABLE /ENABLE services Section
// Importnat due to iram limitation on ESp8266 (virtual function  table)
//#define RF_SNIFFER
#define RGB

#ifdef ESP8266
#ifdef RF_SNIFFER
#define DISABLE_RGB
#define DISABLE_RELAY
#define DISABLE_RELAYDIM
#endif

#ifdef RGB
#define DISABLE_MENU
#define DISABLE_IR
#define DISABLE_RF
#define DISABLE_BUTTON
#endif

#endif

//DISABLE/ENABLE end
const char name_localhost_host[] = "localhost";
const char name_mqtt_host[] = "mqtt_host";
const char name_mqtt_port[] = "mqtt_port";
const char name_mqtt_user[] = "mqtt_user";
const char name_mqtt_pass[] = "mqtt_pass";


extern char HOSTNAME[32] ;   //  hostname


//hold parameters
#if defined ENABLE_HOMEBRIDGE
extern char mqtt_host[64];
extern char mqtt_port[6] ;
extern char mqtt_user[32] ;
extern char mqtt_pass[32] ;
extern short qossub ; // AMQTT can sub qos 0 or 1 or 2



#endif
extern bool isOffline;
extern bool isReboot;
//extern  String globlog;

#define VERSION "0.9"
#if defined ASYNC_WEBSERVER
#define ASYNC "\"true\""
#else
#define ASYNC "\"false\""
#endif




#endif


///Loggger and debugger