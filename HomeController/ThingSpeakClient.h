#ifndef ThingSpeakController_h
#define ThingSpeakController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define MAX_CHANNELS 8
#define MAX_ATTEMPTS 2
struct ThingSpeakState
{
	ThingSpeakState() {
		for (int i = 0; i < MAX_CHANNELS; i++)
			data[i] = 0.0;
	}
	bool isOn;
	double data[MAX_CHANNELS] ;
	long last_measure_ms = 0;

};
enum ThingSpeakCMD {
	TsSend = 1,

	TsRestore = 2048
};
class ThingSpeakController;
typedef CController<ThingSpeakController, ThingSpeakState, ThingSpeakCMD> ThingSpeak;
class ThingSpeakController : public ThingSpeak{
public:
	ThingSpeakController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_state(ThingSpeakState state);
protected:
	bool real_send();
	void real_send_attempts();
	virtual int get_maxcommands() { return 1; }
private:
	String apiKey;
	bool chanelusage[MAX_CHANNELS];
};
DEFINE_CONTROLLER_FACTORY(ThingSpeakController)

#endif
