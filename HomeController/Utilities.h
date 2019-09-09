#ifndef utilities_h
#define utilities_h
#include  <Arduino.h>
#include <ArduinoJson.h> 
#include "config.h"
#include <functional>

#if defined ASYNC_WEBSERVER
#include "ESPAsyncWiFiManager.h"
#define USE_EADNS
#else
#include <WiFiManager.h>        
#endif

//color converters
#define REDVALUE(x) ((x >> 16) & 0xFF)
#define GREENVALUE(x)  ((x >> 8) & 0xFF)
#define BLUEVALUE(x) ((x >> 0) & 0xFF)
// conversion to HUE/Saturation
#define MAXHS(x,y) ((x)>(y) ? (x) : (y))
#define MINHS(x,y) ((x)<(y) ? (x) : (y))

bool writeConfigFS(bool saveConfig);
bool readConfigFS();
String getFormattedTime(time_t tt);
unsigned long GetHours(time_t tt);
unsigned long GetMinutes(time_t tt);
time_t apply_hours_minutes_fromhhmm(time_t src, int hhmm, long offs);
time_t apply_hours_minutes(time_t src, int h, int m, long offs);
time_t	  mklocaltime(struct tm *_timeptr, long offs);
String readfile(const char* filename);
bool savefile(const char* filename, String data);
uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
uint32_t HSVColor(float h, float s, float v);
void ColorToHSI(uint32_t rgbcolor, uint32_t brightness,	double &Hue, double &Saturation, double &Intensity);

class Ticker;
typedef std::function<void(int curval)> funconchangeval;
typedef std::function<void()> funconend;
class CSmoothVal {

public:
	CSmoothVal();
   void start(int from, int to, funconchangeval func, funconend onendfunc=NULL, uint32_t duration = 1000, uint32_t count = 20);
   void stop();
   static void callback(CSmoothVal* self);
   void oncallback();
   bool isActive() { return isactive; };
private:
	Ticker* pTicker;
	uint32_t from;
	uint32_t to;
	uint32_t duration;
	uint32_t count;
	uint32_t counter;
	funconchangeval onchangeval;
	funconend onendfunc;
	bool isactive;
};
#ifdef  ESP32
extern  int first_espchannel;
extern  int current_espchannel;
void set_first_channel(int val);
int get_next_espchannel();
#endif

#ifndef ASYNC_WEBSERVER
void configModeCallback(WiFiManager *myWiFiManager);
#else
void configModeCallback(AsyncWiFiManager *myWiFiManager);
#endif 


extern const char szPinText[] ;
extern const char szbrightnessText[];
extern const char szisOnText[];
extern const char szStatusText[];
extern const char szi2caddr[];
extern const char szpinsda[];
extern const char szpinslc[];

extern const char sztimeoffs[];
extern const char szdayloffs[];
extern const char szserver[];
extern const char szenablesleep[];
extern const char szsleepinterval[];
extern const char szbtnwakeup[];
extern const char szsleeptype[];
extern const char szrestartinterval[];
extern const char sznumshortsleeps[];




extern const char szservice[];
extern const char szname[];

extern const char szParseJsonFailText[];

template<typename T>
void loadif(T& var, JsonObject& json, char * key) {
	if (json.containsKey(key)) {
		var = json[key].as<T>();
	}
}
template<typename T>
void loadif(T& var, JsonObject& json,const char * key) {
	if (json.containsKey(key)) {
		var = json[key].as<T>();
	}
}
template<typename T>
void loadif(T& var, JsonObject& json, const __FlashStringHelper*  key) {
	if (json.containsKey(key)) {
		var = json[key].as<T>();
	}
}
double map_i_f(float val, uint in_min, uint in_max, float out_min, float out_max);

template<typename T>
String format_str(const char* fmt, T val) {

	String res;
	char buff[50];
	snprintf(buff, sizeof(buff), fmt, val);
	res = buff;
	return res;
}

#endif
