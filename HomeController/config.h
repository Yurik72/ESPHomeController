#ifndef config_h
#define config_h

//#ifndef ESP8266
#define ENABLE_NATIVE_HAP      //if defined will communicate Apple Home Kit with native protocols
//#endif

#ifdef ESP8266
//#define ENABLE_HOMEBRIDGE    //if defined will communicate to MQTT Home bdridge
#endif
#define HTTP_OTA             // If defined, enable ESP8266HTTPUpdateServer/ESP32HTTPUpdateServer/ESPAsyncUpdateServer OTA code.


#if defined(ENABLE_NATIVE_HAP) && defined(ENABLE_HOMEBRIDGE)
#error    Strange combination, which is not tested
#endif

#if defined(ENABLE_NATIVE_HAP) && defined(ESP8266)
#warning     Native hap is on development on  ESP8266
#endif




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
#define CUSTOM_WEBASYNCFILEHANDLER  //Use custom file handler to distinguish real file and gzip 
#define CUSTOM_WEBASYNCFILEHANDLER_GZIP_FIRST

//#define OLED_DEBUG
//#define BMETRIGGER_DEBUG
//#define DALLASCONTROLLER_DEBUG
//#define DALLSATRIGGER_DEBUG
//#define TRIGGER_DEBUG  //debug triggers output enable
//#define TIMECONTROLLER_DEBUG  //debug timecontroller output enable

//#define TIMECONTROLLER_FULL_DEBUG  //debug timecontroller output enable
//#define LDRCONTROLLER_DEBUG  //debug ldrcontroller output enable
//#define BMECONTROLLER_DEBUG    //debug bme280controller output enable
//#define RF_TRIGGER_DEBUG         //debug rf trigger
//#define RELAYDIM_DEBUG    //debug relay dim
//#define SERVO_DEBUG    //debug relay dim
//#define MQTT_DEBUG
//#define FACTORY_DEBUG
//#define RFCONTROLLER_DEBUG
//#define RGBSTRIP_DEBUG
//#define MENU_DEBUG
//#define BUTTON_DEBUG
//#define THINGSPEAK_DEBUG
//#define WEATHER_DEBUG
//#define ENCODER_DEBUG

//#define WEATHER_GXEPD2

#if defined(ESP8266) && defined(WEATHER_GXEPD2)
#undef WEATHER_GXEPD2
#endif

//DISABLE /ENABLE services Section
// Importnat due to iram limitation on ESp8266 (virtual function  table)
//#define RF_SNIFFER
#define RGB
//#define RF_RELAY

#ifdef ESP8266
#define DISABLE_OLED
#define DISABLE_THINGSPEAK
#define DISABLE_WEATHER
#define DISABLE_WEATHERDISPLAY
#ifdef RF_SNIFFER
#define DISABLE_RGB
#define DISABLE_RELAY
#define DISABLE_RELAYDIM
#define DISABLE_ENCODER
#endif

#ifdef RGB
#define DISABLE_MENU
#define DISABLE_IR
#define DISABLE_RF
#define DISABLE_BUTTON
#endif

#ifdef RF_RELAY
#define DISABLE_MENU
#define DISABLE_IR
#define DISABLE_RGB
#define DISABLE_RELAYDIM
#define DISABLE_DALLAS
#define DISABLE_BUTTON
#define DISABLE_RELAYBLINK
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
extern bool isAPMode;
extern int accessory_type;
//extern  String globlog;

#define VERSION "0.91"
#if defined ASYNC_WEBSERVER
#define ASYNC "\"true\""
#else
#define ASYNC "\"false\""
#endif




/// spiffs web server
extern const char szfilesourceroot[] ;
//extern const char* filestodownload [] ;
///Loggger and debugger
#endif
