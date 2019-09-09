
#ifndef CWeatherClient_h
#define CWeatherClient_h
#include <Arduino.h>
#include "Array.h"

#define REC_CHAR_MAX 12
struct ForeceastRecord{
  char text[REC_CHAR_MAX];
  char icon[REC_CHAR_MAX];
  char phraseshort[REC_CHAR_MAX];
  uint8_t precipChance;
  uint8_t temperature;
};
struct WeatherState
{
	bool isOn;
	

};
enum WeatherCMD {
	WRefresh = 1,

	WRestore = 2048
};
class WeatherClientController;
typedef CSimpleArray<ForeceastRecord>  ForecastDataT;
typedef CController<WeatherClientController, WeatherState, WeatherCMD> Weather;
class WeatherClientController:public Weather {

public:
WeatherClientController();
virtual String  serializestate();
virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
virtual void getdefaultconfig(JsonObject& json);
virtual void setup();
void loadconfig(JsonObject& json);
virtual void run();
virtual void set_state(WeatherState state);

const ForecastDataT& getdata(){return  (const ForecastDataT&)data;};
bool read_data();
unsigned long get_last_load() { return last_load; };
 protected:
 String uri;
 ForecastDataT data;
 unsigned long last_load = 0;
 
};
DEFINE_CONTROLLER_FACTORY(WeatherClientController)
#endif
