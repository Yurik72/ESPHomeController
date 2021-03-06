#include <Arduino.h>
#include "config.h"
 
#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif
#if defined(ESP8266)
#include "time.h"
#endif
#include "Utilities.h"
#include <Ticker.h>


const char szPinText[] PROGMEM = "pin";
const char szbrightnessText[] PROGMEM = "brightness";
const char szisOnText[] PROGMEM = "isOn";
const char szStatusText[] PROGMEM = "Status";
const char szi2caddr[] PROGMEM = "i2caddr";
const char szpinsda[] PROGMEM = "pinsda";
const char szpinslc[] PROGMEM = "pinslc";



const char sztimeoffs[] PROGMEM = "timeoffs";
const char szdayloffs[] PROGMEM = "dayloffs";
const char szserver[] PROGMEM = "server";
const char szenablesleep[] PROGMEM = "enablesleep";
const char szsleepinterval[] PROGMEM = "sleepinterval";
const char szbtnwakeup[] PROGMEM = "btnwakeup";
const char szsleeptype[] PROGMEM = "sleeptype";
const char szrestartinterval[] PROGMEM = "restartinterval";
const char sznumshortsleeps[] PROGMEM = "numshortsleeps";

const char sznumleds[] PROGMEM = "numleds";
const char szissmooth[] PROGMEM = "issmooth";
const char szrgb_startled[] PROGMEM = "rgb_startled";
const char szismatrix[] PROGMEM = "ismatrix";
const char szmatrixwidth[] PROGMEM = "matrixwidth";
const char szmatrixType[] PROGMEM = "matrixtype";
const char szapiKey[] PROGMEM = "apiKey";

const char sztemp[] PROGMEM = "temp";;
const char szhum[] PROGMEM = "hum";;
const char szpres[] PROGMEM = "pres";;


const char szservice[] PROGMEM = "service";
const char szname[] PROGMEM = "name";
const char szParseJsonFailText[] PROGMEM = "parse Json() failed: ";

bool ICACHE_FLASH_ATTR  writeConfigFS(bool saveConfig) {
	if (saveConfig) {
		//FS save
		//updateFS = true;
		DBG_OUTPUT_PORT.print("Saving config: ");
		DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(40));
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
bool ICACHE_FLASH_ATTR readConfigFS() {
	//read configuration from FS JSON
#ifdef	ENABLE_HOMEBRIDGE
	memset(mqtt_host, 0, sizeof(mqtt_host));
	memset(mqtt_port, 0, sizeof(mqtt_port));
	memset(mqtt_user, 0, sizeof(mqtt_user));
	memset(mqtt_pass, 0, sizeof(mqtt_pass));
#endif
	if (SPIFFS.exists("/config.json")) {
		//file exists, reading and loading
		DBG_OUTPUT_PORT.print("Reading config file... ");
		File configFile = SPIFFS.open("/config.json", "r");
		if (configFile) {
			DBG_OUTPUT_PORT.println("Opened!");
			//size_t size = configFile.size();

			DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(4) + 300);
			DeserializationError error = deserializeJson(jsonBuffer, configFile.readString());
			DBG_OUTPUT_PORT.print("Config: ");
			if (!error) {
				DBG_OUTPUT_PORT.println(" Parsed!");
				JsonObject json = jsonBuffer.as<JsonObject>();
				serializeJson(json, DBG_OUTPUT_PORT);
				char localHost[32];
				memset(localHost, 0, sizeof(localHost));
				const char * jsondata = json[name_localhost_host];
				if (jsondata && strlen(jsondata) > 0) {
					strcpy(localHost, json[name_localhost_host]);
					if (strlen(localHost) > 0)
						strcpy(HOSTNAME, localHost);
					else
						DBG_OUTPUT_PORT.println("Invalid hostname param in config");
				}
				else
					DBG_OUTPUT_PORT.println("Invalid load host name");
				//TO DO !!!
				//isOffline = json["offline"];

				loadif(accessory_type, json, "acctype");

#if defined ENABLE_HOMEBRIDGE
				jsondata = json[name_mqtt_host];
				if (jsondata && strlen(jsondata) > 0)
					strcpy(mqtt_host, jsondata);
				jsondata = json[name_mqtt_port];
				if (jsondata && strlen(jsondata) > 0)
					strcpy(mqtt_port, jsondata);

				jsondata = json[name_mqtt_user];
				if (jsondata && strlen(jsondata) > 0)
					strcpy(mqtt_user, jsondata);

				jsondata = json[name_mqtt_pass];
				if (jsondata && strlen(jsondata) > 0)
					strcpy(mqtt_pass, jsondata);

				
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
String getFormattedDateTime(time_t t) {
	struct tm * timeinfo = localtime(&t);
	String datetime= String(timeinfo->tm_mon) + " " + String(timeinfo->tm_mday) + " " + String(1900 + timeinfo->tm_year);
	datetime += " " + String(timeinfo->tm_hour) + ":" + String(timeinfo->tm_min) + ":" + String(timeinfo->tm_sec);
	return datetime;
}
String ICACHE_FLASH_ATTR getFormattedTime(time_t tt) {

	unsigned long hours = (tt % 86400L) / 3600;
	String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

	unsigned long minutes = (tt % 3600) / 60;
	String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

	unsigned long seconds = tt % 60;
	String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

	return hoursStr + ":" + minuteStr + ":" + secondStr;
}
String ICACHE_FLASH_ATTR getFormattedTime_HH_MM(time_t tt) {

	unsigned long hours = (tt % 86400L) / 3600;
	String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

	unsigned long minutes = (tt % 3600) / 60;
	String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);



	return hoursStr + ":" + minuteStr;
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
time_t ICACHE_FLASH_ATTR apply_hours_minutes(time_t src, int h, int m,long offs) {

	struct tm *tminfo;
	tminfo = localtime(&src);
	tminfo->tm_hour = h;
	tminfo->tm_min = m;
	tminfo->tm_sec = 0;

	
	return mklocaltime(tminfo,offs);
}
time_t	 mklocaltime(struct tm *_timeptr, long offs) {
	return mktime(_timeptr) + offs;
}
uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Convert Hue/Saturation/Brightness values to a packed 32-bit RBG color.
// hue must be a float value between 0 and 360
// saturation must be a float value between 0 and 1
// brightness must be a float value between 0 and 1
uint32_t  HSVColor(float h, float s, float v) {

	h = constrain(h, 0.0, 360.0);
	s = constrain(s, 0.0, 1.0);
	v = constrain(v, 0.0, 1.0);

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

void ColorToHSI(uint32_t rgbcolor, uint32_t brightness,	double &Hue, double &Saturation, double &Intensity)
{
	uint32_t r = REDVALUE(rgbcolor);
	uint32_t g = GREENVALUE(rgbcolor);
	uint32_t b = BLUEVALUE(rgbcolor);

	if ((r < 0 && g < 0 && b < 0) || (r > 255 || g > 255 || b > 255))
	{
		Hue = Saturation = Intensity = 0;
		return;
	}

	if (g == b)
	{
		if (b < 255)
		{
			b = b + 1;
		}
		else
		{
			b = b - 1;
		}
	}
	uint32_t nImax, nImin, nSum, nDifference;
	nImax = MAXHS(r, b);
	nImax = MAXHS(nImax, g);
	nImin = MINHS(r, b);
	nImin = MINHS(nImin, g);
	nSum = nImin + nImax;
	nDifference = nImax - nImin;

	Intensity = (float)nSum / 2;

	if (Intensity < 128)
	{
		Saturation = (255 * ((float)nDifference / nSum));
	}
	else
	{
		Saturation = (float)(255 * ((float)nDifference / (510 - nSum)));
	}

	if (Saturation != 0)
	{
		if (nImax == r)
		{
			Hue = (60 * ((float)g - (float)b) / nDifference);
		}
		else if (nImax == g)
		{
			Hue = (60 * ((float)b - (float)r) / nDifference + 120);
		}
		else if (nImax == b)
		{
			Hue = (60 * ((float)r - (float)g) / nDifference + 240);
		}

		if (Hue < 0)
		{
			Hue = (60 * ((float)b - (float)r) / nDifference + 120);
		}
	}
	else
	{
		Hue = -1;
	}

	return;
}
String ICACHE_FLASH_ATTR readfile(const char* filename) {
	File file = SPIFFS.open(filename, "r");
	String res;
	if (file) {
		size_t size = file.size();
		// Allocate a buffer to store contents of the file.
		res=file.readString();
		file.close();
	}
	return res;
}
bool ICACHE_FLASH_ATTR savefile(const char* filename, String data) {
	File file = SPIFFS.open(filename, "w");
	if (file) {
		file.println(data);  //save json data
		file.close();
		return true;
	}
	DBG_OUTPUT_PORT.println("Failed to save file" );
	return false;
}

CSmoothVal::CSmoothVal() {
	pTicker = NULL;
	isactive = false;
}
void CSmoothVal::start(int from, int to, funconchangeval func, funconend onendfunc, uint32_t duration , uint32_t count ) {
	this->stop();
	this->pTicker = new Ticker();
	this->from = from;
	this->to = to;
	this->duration = duration;
	this->count = count;
	this->counter = 0;
	this->onchangeval = func;
	this->onendfunc = onendfunc;
	this->isactive = true;
	this->pTicker->attach_ms<CSmoothVal*>(this->duration / this->count, CSmoothVal::callback,this);
}
void CSmoothVal::oncallback() {
	this->counter++;
	if (this->counter > this->count) {
		this->stop();
		
		if (this->onendfunc)
			this->onendfunc();
	}
	else {
		uint32_t curval = 0;
		if(this->to >= this->from)
			curval=this->counter*(this->to - this->from) / this->count + this->from;
		else
			curval= this->from - this->counter*(this->from - this->to) / this->count ;
		if(this->onchangeval)
			this->onchangeval(curval);
	}

}
void CSmoothVal::callback(CSmoothVal* self) {
	self->oncallback();
}
void CSmoothVal::stop() {
	if (this->pTicker) {
		//if (this->pTicker->active())
		//	this->pTicker->detach();
		delete this->pTicker;
		this->pTicker = NULL;
	}
	this->isactive = false;
	this->isactive = false;
}

#ifdef  ESP32
int first_espchannel=0;
int current_espchannel=0;

void set_first_channel(int val) {

	first_espchannel = val;
	current_espchannel = val;
}
int get_next_espchannel() {
	return current_espchannel++;
}
#endif

#ifndef ASYNC_WEBSERVER
void configModeCallback(WiFiManager *myWiFiManager) {
#else
void configModeCallback(AsyncWiFiManager *myWiFiManager) {
#endif 
	DBG_OUTPUT_PORT.println("Entered config mode");
	DBG_OUTPUT_PORT.println(WiFi.softAPIP());
	//if you used auto generated SSID, print it
	DBG_OUTPUT_PORT.println(myWiFiManager->getConfigPortalSSID());
	//entered config mode, make led toggle faster

}

double map_i_f(float val, uint in_min, uint in_max, float out_min, float out_max) {
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}





uint32_t calcTempColorSimple(float temp, float temp_min, float temp_max) {
	uint32_t res = 0;
	if (temp <= temp_min) {
		res = Color(0, 0, 255);
	}
	else if (temp >= temp_max) {
		res = Color(255, 0, 0);
	}
	else {
		float mid = (temp_max + temp_min) / 2.0;
		if (temp < mid) {
			res = Color(
				0,
				map((uint32_t)(temp * 100.0), (uint32_t)(temp_min * 100.0), (uint32_t)(mid * 100.0), 0, 255),
				map((uint32_t)(temp * 100.0), (uint32_t)(temp_min * 100.0), (uint32_t)(mid * 100.0), 255, 0));
		}
		else {
			res = Color(
				map((uint32_t)(temp * 100.0), (uint32_t)(mid * 100.0), (uint32_t)(temp_max * 100.0), 0, 255),
				map((uint32_t)(temp * 100.0), (uint32_t)(mid * 100.0), (uint32_t)(temp_max * 100.0), 255, 0),
				0);
		}
	}

	return res;
}
void TraceColor(char* msg, uint32_t color) {
	String message = String(msg) + String("Red:") + String(REDVALUE(color)) + String(" ") + String("Green:") + String(GREENVALUE(color)) + String(" ") + String("blue:") + String(BLUEVALUE(color));
	DBG_OUTPUT_PORT.println(message);
}
JsonObject getRootObject(DynamicJsonDocument buf) {

	return buf.as<JsonObject>();
}
int CalculateIAQLevel(int iaqscore) {
	//String IAQ_text = "air quality is ";
	//score = (100 - score) * 5;
	int res;
	if (iaqscore >= 301)  res = 5;//               IAQ_text += "Hazardous";
	else if (iaqscore >= 201 && iaqscore <= 300)res = 4;// IAQ_text += "Very Unhealthy";
	else if (iaqscore >= 176 && iaqscore <= 200) res = 3;//IAQ_text += "Unhealthy";
	else if (iaqscore >= 151 && iaqscore <= 175)res = 2;// IAQ_text += "Unhealthy for Sensitive Groups";
	else if (iaqscore >= 51 && iaqscore <= 150) res = 1;//IAQ_text += "Moderate";
	else if (iaqscore >= 00 && iaqscore <= 50) res = 0;// IAQ_text += "Good";
	//Serial.print("IAQ Score = " + String(score) + ", ");
	return res;
}
#ifdef ESP32
bool get_json_bool(JsonObject obj, const char* name) {
	bool b=obj[name];
	return b;
}
double get_json_double(JsonObject obj, const char* name) {
	double d= obj[name];
	return d;
}
bool get_json_bool(JsonObject obj, const __FlashStringHelper* name) {
	bool b = obj[name];
	return b;
}
double get_json_double(JsonObject obj, const __FlashStringHelper* name) {
	double d = obj[name];
	return d;
}
#else
bool get_json_bool(JsonObject obj, const __FlashStringHelper* name) {
	bool b = obj[name];
	return b;
}
double get_json_double(JsonObject obj, const __FlashStringHelper* name) {
	double d = obj[name];
	return d;
}
#endif