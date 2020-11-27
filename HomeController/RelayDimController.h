#ifndef RelayDimController_h
#define RelayDimController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#endif



struct RelayDimState
{
	bool isOn = false;
	bool isLdr = false;
	int brightness = 0;
	int ldrValue = 0;
	int get_br_100(){
		return map(brightness,0,0xFF,0,100);
	}
	void set_br_100(int val){
		brightness= map(val,0,100,0,0xFF);
	};
};
enum RelayDimCMD {
	DimRelayOn = BaseOn,
	DimRelayOff = BaseOff,
	DimSwitch = 4,
	DimSet = 8,
	DimSetBrigthness = 16,
	DimSetLdrVal = 32,
	DimSetIsLdr = 128,
	DimRelayRestore = 2048
};

class RelayDimController;
typedef CManualStateController<RelayDimController, RelayDimState, RelayDimCMD> RelayDim;
class RelayDimController : public RelayDim
{
public:
	RelayDimController();
	~RelayDimController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void setup();
	virtual void loadconfig(JsonObject& json);
	virtual void run();
	virtual void set_power_on();
	virtual void set_state(RelayDimState state);
	virtual bool onpublishmqtt(String& endkey, String& payload);
	void setBrightness(uint8_t br);
//	virtual bool ispersiststate() { return true; }
	int getLDRBrightness(int brigtness, int ldrval);
	virtual void onmqqtmessage(String topic, String payload);
	virtual bool onpublishmqttex(String& endkey, String& payload, int topicnr);

#ifdef	ENABLE_NATIVE_HAP
	virtual void setup_hap_service();
	static void hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);

	virtual void notify_hap();

#endif
protected:
	uint pin;
	bool isinvert;
	uint8_t poweronbr;
#ifdef	ENABLE_NATIVE_HAP
	homekit_service_t* hapservice;
	homekit_characteristic_t * hap_on;
	homekit_characteristic_t * hap_br;

#endif
#ifdef ESP32
	int channel;
#endif
	CSmoothVal* pSmooth;
	bool isEnableSmooth;
};
DEFINE_CONTROLLER_FACTORY(RelayDimController)

#endif