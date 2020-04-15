#ifndef ButtonController_h
#define ButtonController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define MAX_BUTTONS 8

#define ADJUST_MASK      0b1111000000111111
#define PRESSED_MASK     0b1111000000000000
#define DOWN_MASK        0b0000000000111111

#define STATE_HISTORY_MAX 5
#define STATE_HISTORY_PERIOD_UPDATE 2000

#define LONG_ACTIONDURATION 1000
#define VERYLONG_ACTIONDURATION 20000
struct ButtonState
{
	uint8_t idx = 0;
	long changed_at = 0;
	bool isPressed = false;
	bool isDown = false;
	bool isLongPressed=false;
	bool isVeryLongPressed= false;
};
enum ButtonCMD :uint {SetBtn, BtnSaveState = 4096 };
enum  enumstate :int { none = 0,ispressed = 1, isdown = 2,islongpressed=3 };
enum  longhistoryres :int { his_none = 0, his_longpressed = 1, his_multiplepress = 2 };
class ButtonController;
typedef CController<ButtonController, ButtonState, ButtonCMD> Button;
struct btn_state_history_record
{
	long ms=0;
	enumstate state;
};
typedef btn_state_history_record btn_state_history[STATE_HISTORY_MAX];
class ButtonController : public Button
{
public:
	ButtonController();
	virtual String  serializestate();
	virtual bool  deserializestate(String jsonstate, CmdSource src = srcState);
	virtual void setup();
	void loadconfig(JsonObject& json);
	virtual void getdefaultconfig(JsonObject& json);
	virtual void run();
	virtual void runcore();
	enumstate* get_buttonsstate() { return btnstate; };
	bool is_pressed(uint8_t idx);
	bool is_down(uint8_t idx);
	long get_btn_presstime(uint8_t idx) { return btnpresstime[idx]; };
protected:
	uint8_t pin[MAX_BUTTONS];
	void update(uint8_t idx);
	void addhistory(bool bit, uint8_t idx);
	longhistoryres update_history_state(uint8_t idx, enumstate state, long ms);
	longhistoryres check_update_longhistory(uint8_t idx);
	
	
private:
	uint16_t btnhistory[MAX_BUTTONS];
	long btnpresstime[MAX_BUTTONS];
	enumstate  btnstate [MAX_BUTTONS];
	btn_state_history btn_long_history[MAX_BUTTONS];
	uint8_t btncount;
	long last_history_state_update;
	bool defstate;
	
};
DEFINE_CONTROLLER_FACTORY(ButtonController)
#endif