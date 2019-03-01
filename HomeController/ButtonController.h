#ifndef ButtonController_h
#define ButtonController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define MAX_BUTTONS 8

#define ADJUST_MASK      0b1111000000111111
#define PRESSED_MASK     0b1111000000000000
#define DOWN_MASK        0b0000000000111111

struct ButtonState
{
	uint8_t idx = 0;
	bool isPressed = false;
	bool isDown = false;
};
enum ButtonCMD :uint {SetBtn, BtnSaveState = 4096 };
enum  enumstate { ispressed = 1, isdown = 2 };
class ButtonController;
typedef CController<ButtonController, ButtonState, ButtonCMD> Button;
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
protected:
	uint8_t pin[MAX_BUTTONS];
	void update(uint8_t idx);
	void addhistory(bool bit, uint8_t idx);

	bool is_down(uint8_t idx);
	bool is_pressed(uint8_t idx);
private:
	uint16_t btnhistory[MAX_BUTTONS];
	uint64_t btnpresstime[MAX_BUTTONS];
	enumstate  btnstate [MAX_BUTTONS];
	uint8_t btncount;
	
};
DEFINE_CONTROLLER_FACTORY(ButtonController)
#endif