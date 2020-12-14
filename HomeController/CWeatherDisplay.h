#ifndef CWeatherDisplay_h
#define CWeatherDisplay_h
#include <Arduino.h>
#include "SPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ILI9341.h>
#ifdef WEATHER_GXEPD2
#include <GxEPD2_BW.h>
//to do dynamic 
#define EPD2TYPENAME GxEPD2_213_B73 
#define EPDTYPEMACRO(name) GxEPD2_BW<name, name::HEIGHT>

#define EPDTYPE EPDTYPEMACRO(EPD2TYPENAME)

#define EPDNEW new EPDTYPE()

#endif

#include "CWeatherClient.h"

#ifdef WEATHER_GXEPD2
#include <GxEPD2_BW.h>
#endif

#define BMP_WIDTH 25
#define BMP_HEIGHT 25
#define IMAGE_ROWLEN 125
#define COLORBKG 0x0C23D4
#define COLOR_LIGHTBLUE 0x0C23D4


#define ICONCOL(x)  ((x >> 8) & 0xFF)
#define ICONROW(x) ((x >> 0) & 0xFF)

#define SETROW(x,r) x&=0xFF00;x|=r;
#define SETCOL(x,c) x&=0x00FF;x|=(c<<8);

#define SETICON(x,r,c) x=0;x|=r|(c<<8);
//define pins of TFT screen
#define TFT_CS     12
#define TFT_RST    14 
#define TFT_DC     13
#define TFT_SCLK   22
#define TFT_MOSI   21
#define TFT_MISO   18

#define TFTILI_CS     15
#define TFTILI_RST    12 
#define TFTILI_DC     4
#define TFTILI_SCLK   18
#define TFTILI_MOSI   23
#define TFTILI_MISO   19

#define SD_CS 25
#define SD_MOSI 33
#define SD_MISO 32
#define SD_SCK 35
#define F_CS 34

#define BR_MIN_VAL 0
#define BR_MAX_VAL 0xFF
#define BR_FREQ 5000
#define BR_RESOLUTION 8
#define BRCALC_VAL(val,invert) constrain(((!invert)?val:(BR_MAX_VAL-val)),BR_MIN_VAL,BR_MAX_VAL)

struct ForecastData{
  double temp=0;
  uint16_t condition;
  String day;
  uint8_t rain_chance=0;
};
struct WeatherData{
  double temp=0;
  double pressure=0;
  double humidity=0;
  double gas = 0;

  
};
struct DispRect {
	DispRect() :DispRect(0, 0, 0, 0) {};
	DispRect(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h):x(_x),y(_y),w(_w),h(_h) {};
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t w = 0;
	uint16_t h = 0;
};
typedef struct DispRect DispRect_t;
enum Orientation{
  Hor=0,
  Vert=1
};
enum DisplayType{
  ST7735=0,
  ILI9341=1,
  GxEPD2=2
};
enum DisplayMode {
	MainWether = 1,
	Info = 2,
	MaxMode = 3
};
typedef CSimpleArray<ForeceastRecord>  ForecastDataT;
struct WeatherDisplayState
{
	bool isOn=true;
	uint8_t mode = 0;
	time_t now;
	WeatherData data;
	ForeceastRecord frecord;
	uint8_t brigthness = 200;
	
};
enum WeatherDisplayCMD {
	WDRefreshAll = 2,
	WDRefreshForecast = 4,
	WDRefreshMain = 8,
	WDSetTime    =16,
	WDSetCurrentData = 32,
	WDClearForecastData = 64,
	WDAddForecastData = 128,
	WDFreeze=256,
	WDUnFreeze=512,
	WDSwitchMode=1024,
	WDRestore = 2048,
	WDSetBrigthness = 4096
};
//const String WDAY_NAMES[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const String MONTH_NAMES[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

class WeatherDisplayController;

typedef CController<WeatherDisplayController, WeatherDisplayState, WeatherDisplayCMD> WeatherDisplay;

class WeatherDisplayController :public WeatherDisplay {
public:
	WeatherDisplayController();

	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	
	virtual void set_state(WeatherDisplayState state);
	virtual void on_event(CBaseController* pSender, ControllerEvent evt, uint16_t evData);
	void refreshAll();

void draw_icon(uint8_t row,uint8_t col,uint16_t x,uint16_t y);
void draw_icon(uint16_t x,uint16_t y,String icon,const uint16_t palette[]);
void drawSSchar(uint16_t x, uint16_t y,uint8_t ch,uint16_t color);
void drawSSString(uint16_t x, uint16_t y,String str,uint16_t color);
void writePixel (uint16_t x,uint16_t y, uint16_t color);
void drawPixel (uint16_t x,uint16_t y, uint16_t color);




String format_doublestr(const char* fmt, double val);
//WeatherData& getweatherdata(){return wdata;};
ForecastDataT& getforecastdata(){return fData;};

uint16_t color8to16(uint8_t color);
Orientation get_orientation(){return orientation;};
void set_orientation(Orientation o);
protected :
void recalclayout();
void draw_time(time_t time, uint16_t color = ST7735_WHITE);
void draw_date(String date, uint16_t color = ST7735_WHITE);
void cleardisplay();
void draw_info();
void draw_infoline(uint16_t y, String label, String text);
void clear_forecast();
void draw_forecast(uint8_t idx, ForeceastRecord& data);
void adjustRotation();
void draw_forecast();
void draw_weatherdata();
void draw_time();
void clearTime();
void clearDate();
void clearRect(DispRect_t* pRect);
void diagnostic();
void refreshTime();
void setBrightness(uint8_t br);
int ColorForAirQuality(int level);
void flushDisplay(bool partial=true);
String format_date(time_t time);
Adafruit_ST7735* pDisplay7735;
Adafruit_ILI9341* pDisplay9341;
Adafruit_GFX* pDisplay;
#ifdef WEATHER_GXEPD2
EPDTYPE* pDisplayEPD2;
#endif 
uint16_t y_offs_forecast;
uint16_t height_forecast;
uint16_t width_forecast;
uint8_t max_forecast;
uint16_t y_offs_dt_text=5;
uint16_t x_offs_time_text = 170;
uint8_t fontsize_forecast_temp_h=1;
bool isdraw_forecastcondition=false;
uint16_t forecast_cond_offset=70;
uint16_t main_offset_y_wetaher=1;
uint16_t start_y_weather = 25;



ForecastDataT fData;
Orientation orientation;
DisplayType disptype;

DisplayMode dispMode = MainWether;

uint8_t pcs = TFTILI_CS;
uint8_t prst = TFTILI_RST;
uint8_t pdc = TFTILI_DC;
uint8_t psclk = TFTILI_SCLK;
uint8_t pmosi = TFTILI_MOSI;
uint8_t pmiso = TFTILI_MISO;
uint8_t pbr = 0;
uint8_t dac_i2c = 0;

uint8_t brigthness = 200;
time_t disp_time;
uint16_t dt_color = ST7735_WHITE;
uint16_t bk_color = ST7735_BLACK;
uint16_t forecast_day_color = ST7735_GREEN;
uint16_t forecast_temp_color = ST7735_YELLOW;
uint16_t forecast_precip_color = COLOR_LIGHTBLUE;
uint16_t* icon_palette = 0;
uint8_t  dt_text_size = 2;
uint8_t  tm_text_size = 3;

DispRect_t rect_time;
DispRect_t rect_forecast;
DispRect_t rect_temp;
DispRect_t rect_date;
DispRect_t rect_hum;
DispRect_t rect_press;
#ifdef ESP32
int br_channel = 0;;
#endif
};

DEFINE_CONTROLLER_FACTORY(WeatherDisplayController)
#endif
