#include <Arduino.h>
#include "config.h"
#include <ArduinoJson.h>  
#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif
#if defined(ESP8266)
#include "time.h"
#endif
#include "Utilities.h"

bool writeConfigFS(bool saveConfig) {
	if (saveConfig) {
		//FS save
		//updateFS = true;
		DBG_OUTPUT_PORT.print("Saving config: ");
		DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(4));
		JsonObject json = jsonBuffer.to<JsonObject>();
#if defined ENABLE_HOMEBRIDGE
		json[name_mqtt_host] = mqtt_host;
		json[name_mqtt_port] = mqtt_port;
		json[name_mqtt_user] = mqtt_user;
		json[name_mqtt_pass] = mqtt_pass;
#endif
		json[name_localhost_host] = HOSTNAME;
		//      SPIFFS.remove("/config.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
		File configFile = SPIFFS.open("/config.json", "w");
		if (!configFile) DBG_OUTPUT_PORT.println("failed to open config file for writing");

		serializeJson(json, DBG_OUTPUT_PORT);
		serializeJson(json, configFile);
		configFile.close();
		//updateFS = false;
		return true;
		//end save
	}
	else {
		DBG_OUTPUT_PORT.println("SaveConfig is False!");
		return false;
	}
}
// Read search_str to FS
bool readConfigFS() {
	//read configuration from FS JSON

	if (SPIFFS.exists("/config.json")) {
		//file exists, reading and loading
		DBG_OUTPUT_PORT.print("Reading config file... ");
		File configFile = SPIFFS.open("/config.json", "r");
		if (configFile) {
			DBG_OUTPUT_PORT.println("Opened!");
			size_t size = configFile.size();

			DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(4) + 300);
			DeserializationError error = deserializeJson(jsonBuffer, configFile.readString());
			DBG_OUTPUT_PORT.print("Config: ");
			if (!error) {
				DBG_OUTPUT_PORT.println(" Parsed!");
				JsonObject json = jsonBuffer.as<JsonObject>();
				serializeJson(json, DBG_OUTPUT_PORT);
				char localHost[32];
				strcpy(localHost, json[name_localhost_host]);
				if (strlen(localHost)>0);
					strcpy(HOSTNAME, localHost);
#if defined ENABLE_HOMEBRIDGE
				strcpy(mqtt_host, json[name_mqtt_host]);
				strcpy(mqtt_port, json[name_mqtt_port]);
				strcpy(mqtt_user, json[name_mqtt_user]);
				strcpy(mqtt_pass, json[name_mqtt_pass]);
#endif
				return true;
			}
			else {
				DBG_OUTPUT_PORT.print("Failed to load json config: ");
				DBG_OUTPUT_PORT.println(error.c_str());
			}
		}
		else {
			DBG_OUTPUT_PORT.println("Failed to open /config.json");
		}
	}
	else {
		DBG_OUTPUT_PORT.println("Coudnt find config.json");
	}

	return false;
}
String getFormattedTime(time_t tt) {

	unsigned long hours = (tt % 86400L) / 3600;
	String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

	unsigned long minutes = (tt % 3600) / 60;
	String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

	unsigned long seconds = tt % 60;
	String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

	return hoursStr + ":" + minuteStr + ":" + secondStr;
}
unsigned long GetHours(time_t tt) {
	return (tt % 86400L) / 3600;
}
unsigned long GetMinutes(time_t tt) {
	return (tt % 3600) / 60;;
}
time_t apply_hours_minutes_fromhhmm(time_t src, int hhmm, long offs) {
	return apply_hours_minutes(src, hhmm / 100, hhmm % 100, offs);
}
time_t apply_hours_minutes(time_t src, int h, int m,long offs) {

	struct tm *tminfo;
	tminfo = localtime(&src);
	tminfo->tm_hour = h;
	tminfo->tm_min = m;
	tminfo->tm_sec = 0;

	
	return mklocaltime(tminfo,offs);
}
time_t	mklocaltime(struct tm *_timeptr, long offs) {
	return mktime(_timeptr) + offs;
}
uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Convert Hue/Saturation/Brightness values to a packed 32-bit RBG color.
// hue must be a float value between 0 and 360
// saturation must be a float value between 0 and 1
// brightness must be a float value between 0 and 1
uint32_t HSVColor(float h, float s, float v) {

	h = constrain(h, 0, 360);
	s = constrain(s, 0, 1);
	v = constrain(v, 0, 1);

	int i, b, p, q, t;
	float f;

	h /= 60.0;  // sector 0 to 5
	i = floor(h);
	f = h - i;  // factorial part of h

	b = v * 255;
	p = v * (1 - s) * 255;
	q = v * (1 - s * f) * 255;
	t = v * (1 - s * (1 - f)) * 255;

	switch (i) {
	case 0:
		return Color(b, t, p);
	case 1:
		return Color(q, b, p);
	case 2:
		return Color(p, b, t);
	case 3:
		return Color(p, q, b);
	case 4:
		return Color(t, p, b);
	default:
		return Color(b, p, q);
	}
}
