#ifndef config_h
#define config_h
#define ENABLE_HOMEBRIDGE    //if defined will communicate to MQTT Home bdridge
#define HTTP_OTA             // If defined, enable ESP8266HTTPUpdateServer OTA code.

#define DBG_OUTPUT_PORT Serial  // Set debug output port

#define CONFIG_PORTAL_TIMEOUT 600/// secs , to wait configuration has been done by user

//#define TRIGGER_DEBUG  //debug triggers output
//#define TIMECONTROLLER_DEBUG  //debug timecontroller output

//#define LDRCONTROLLER_DEBUG  //debug timecontroller outpu
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

#endif