#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"

#include "Utilities.h"
#include "BaseController.h"
#include "EncoderController.h"
#ifdef ESP32
portMUX_TYPE DRAM_ATTR encoderMux = portMUX_INITIALIZER_UNLOCKED;
#endif

//REGISTER_CONTROLLER(RelayDimController)
#ifndef DISABLE_ENCODER
REGISTER_CONTROLLER_FACTORY(EncoderController)
#endif
const size_t bufferSize = JSON_OBJECT_SIZE(20);

EncoderController::EncoderController() {
	this->isinvert = false;
	this->pin = 0;
	this->pinA = 0;
	this->pinB = 0;
	this->btnhistory = 0;
	this->coreMode = Both;
	this->isEnabled = true;
	this->encoderSteps = 2;
	this->_minEncoderValue = -1 << 15;
	this->_maxEncoderValue = (int)((1 << 15)-1);

}
EncoderController::~EncoderController() {

}
String  EncoderController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["rotateDelta"] = this->get_state().rotateDelta;
	root["isPressed"] = this->get_state().isPressed;

	String json;
	json.reserve(128);
	serializeJson(root, json);

	return json;
}
bool  EncoderController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(F("deserialize failed: "));
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	EncoderState newState;
	newState.rotateDelta = root["rotateDelta"];

	this->AddCommand(newState, EncoderSet, src);
	//this->set_state(newState);
	return true;

}
void EncoderController::loadconfig(JsonObject& json) {
	Encoder::loadconfig(json);
	loadif(pin, json, FPSTR(szPinText));
	loadif(pinA, json, "pina");
	loadif(pinB, json, "pinb");
	loadif(encoderSteps, json, "encoderSteps");

}
void EncoderController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)] = pin;
	json["pina"] = pinA;
	json["pinb"] = pinB;
	json["encoderSteps"] = encoderSteps;
	
	json[FPSTR(szservice)] = "EncoderController";
	
	json[FPSTR(szname)] = "Encoder";
	Encoder::getdefaultconfig(json);
}
void  EncoderController::setup() {
#ifdef ENCODER_DEBUG
	DBG_OUTPUT_PORT.println(F("RelayDimController::setup()"));
#endif
	if (pinA > 0 && pinB > 0) {
		pinMode(this->pinA, INPUT);
		pinMode(this->pinB, INPUT);
	}
	if (pin>0) {
		pinMode(pin, INPUT_PULLUP);
	}
#ifdef ESP8266

#endif
#ifdef ESP32
	EncoderController*self = this;
	if (pinA > 0 && pinB > 0) {
		attachInterruptArg(digitalPinToInterrupt(this->pinA), EncoderController::Encoder_ISR, this, CHANGE);
		attachInterruptArg(digitalPinToInterrupt(this->pinB), EncoderController::Encoder_ISR, this, CHANGE);
	}
#endif
}

void EncoderController::run() {

	if (this->commands.GetSize() == 0) {
		int16_t delta = encoderChanged();
		if (delta != 0) {
				command newcmd;

				newcmd.mode = EncoderSetDelta;
				newcmd.state.rotateDelta = delta;// analogRead(pin);
				newcmd.state.delta_ms = millis();
		#ifdef  ENCODER_DEBUG
				DBG_OUTPUT_PORT.println("Encoder set delta:"+String(delta));
		#endif 
				//this->commands.Add(newcmd);
				this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
		}
	}
	command cmd;

	while (commands.Dequeue(&cmd)) {
#ifdef ENCODER_DEBUG
		//DBG_OUTPUT_PORT.println(F("EncoderController::run()"));
#endif
		EncoderState newState = cmd.state;
		switch (cmd.mode) {
		case EncoderSetBtn:
			newState.isPressed = cmd.state.isPressed;

			break;
		case EncoderSetDelta:
			newState.rotateDelta = cmd.state.rotateDelta;
			break;
		case EncoderSet:
			newState = cmd.state;
			break;

		default:break;
		}
		this->set_state(newState);
	}
	Encoder::run();
}
void EncoderController::runcore() {
	if(pin>0)
		this->handleButtonState();
	Encoder::runcore();
}
void EncoderController::handleButtonState() {
	this->btnhistory = (this->btnhistory << 1) | !(digitalRead(pin));
	uint16_t adjusted = this->btnhistory & ENCODER_BUTTON_ADJUST_MASK;
	command newcmd;
	newcmd.state = this->get_state();
	if (adjusted == ENCODER_BUTTON_DOWN_MASK && !newcmd.state.isDown) {
		newcmd.mode = EncoderSetBtn;
		newcmd.state.button_ms = millis();
		newcmd.state.isDown = true;
		newcmd.state.isPressed = false;
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
	else if (adjusted == ENCODER_BUTTON_PRESSED_MASK && !newcmd.state.isPressed) {
		newcmd.mode = EncoderSetBtn;
		newcmd.state.button_ms = millis();
		newcmd.state.isDown = false;
		newcmd.state.isPressed = true;
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
}
void EncoderController::set_state(EncoderState state) {
	EncoderState oldState = this->get_state();
	Encoder::set_state(state);
}

//encoder implementation
void IRAM_ATTR EncoderController::Encoder_ISR(void* arg)
{
	EncoderController* self = static_cast<EncoderController*>(arg);
#ifdef ENCODER_DEBUG
	//DBG_OUTPUT_PORT.println("Encoder_ISR");
#endif
#ifdef ESP32
	portENTER_CRITICAL_ISR(&(encoderMux));
#endif
	if (self->isEnabled) {

	
		self->old_AB <<= 2;                   //remember previous state

		//Serial.print("OldAB= ");
		//Serial.println(old_AB, BIN);

		int8_t ENC_PORT = ((digitalRead(self->pinB)) ? (1 << 1) : 0) | ((digitalRead(self->pinA)) ? (1 << 0) : 0);

		//Serial.print("ENC_PORT= ");
		//Serial.println(ENC_PORT, BIN);
#ifdef ENCODER_DEBUG
		//DBG_OUTPUT_PORT.println("ENC_PORT= " + String(ENC_PORT));
#endif
		self->old_AB |= (ENC_PORT & 0x03);  //add current state

		//Serial.print("NewAB= ");
		//Serial.println(old_AB, BIN);		

		//Serial.print("old_AB & 0x0f= ");
		//Serial.println(( old_AB & 0x0f ), BIN);

		self->encoder0Pos += (self->enc_states[(self->old_AB & 0x0f)]);
#ifdef ENCODER_DEBUG
		
	   // DBG_OUTPUT_PORT.println("Encoder_ISR increment:" + String(self->enc_states[(self->old_AB & 0x0f)]));
		//DBG_OUTPUT_PORT.println("Encoder_ISR 1:" + String(self->encoder0Pos));
		//DBG_OUTPUT_PORT.println("Encoder_ISR min:" + String(self->_minEncoderValue));
		//DBG_OUTPUT_PORT.println("Encoder_ISR max:" + String(self->_maxEncoderValue));
#endif
		if (self->encoder0Pos > (self->_maxEncoderValue))
			self->encoder0Pos = self->_circleValues ? self->_minEncoderValue : self->_maxEncoderValue;
		if (self->encoder0Pos < (self->_minEncoderValue))
			self->encoder0Pos = self->_circleValues ? self->_maxEncoderValue : self->_minEncoderValue;
#ifdef ENCODER_DEBUG
		//DBG_OUTPUT_PORT.println("Encoder_ISR 2:"+String(self->encoder0Pos) );
#endif
		//Serial.print("encoder0Pos= ");
		//Serial.println(this->encoder0Pos);	
		//Serial.println("---------------");
	}
#ifdef ESP32
	portEXIT_CRITICAL_ISR(&(encoderMux));
#endif
}


int16_t EncoderController::readEncoder()
{
	return (this->encoder0Pos / this->encoderSteps);
}

int16_t EncoderController::encoderChanged() {
	int16_t _encoder0Pos = readEncoder();

	int16_t encoder0Diff = _encoder0Pos - this->lastReadEncoder0Pos;

	this->lastReadEncoder0Pos = _encoder0Pos;
	return encoder0Diff;
}