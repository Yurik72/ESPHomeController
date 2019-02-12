#ifndef ButtonController_h
#define ButtonController_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include "BaseController.h"

#define ADJUST_MASK      0b1111000000111111
#define PRESSED_MASK     0b1111000000000000
#define DOWN_MASK        0b0000000000111111

struct ButtonState
{
	bool isPressed = false;
	bool isDown = false;
};
enum ButtonCMD :uint {SetBtn, BtnSaveState = 4096 };

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
	uint pin;
	void update();
	void addhistory(bool bit);

	bool is_down();
	bool is_pressed();
private:
	uint16_t btnhistory;
	uint64_t btnpresstime;
	
};

#endif