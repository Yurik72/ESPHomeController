#include <Arduino.h>
#include <ArduinoJson.h>
#ifdef ESP32
#include <IRremote.h>
#endif

#ifdef ESP8266
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#endif
#include "config.h"
#include "RCSwitch.h"
#include "IRController.h"


//REGISTER_CONTROLLER(RFController)
#ifndef DISABLE_IR
REGISTER_CONTROLLER_FACTORY(IRController)
#endif
const size_t bufferSize = JSON_OBJECT_SIZE(5);



IRController::IRController() {
	this->pin = 0;
	this->pinsend = 0;
#ifdef ESP8266
	this->pReceiver = NULL;
#endif
	this->store_recdata = true;

}

String  IRController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isReceive"] = this->get_state().isReceive;
	root["isSend"] = this->get_state().isSend;
	root["irtoken"] = this->get_state().irtoken;

	String json;
	json.reserve(64);
	serializeJson(root, json);

	return json;
}

bool  IRController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	IRState newState;
	//newState.isPressed = root["isPressed"];


	//	this->AddCommand(newState, Measure, src);
		//this->set_state(newState);
	return true;

}
void IRController::loadconfig(JsonObject& json) {
	IR::loadconfig(json);
	pin = json[FPSTR(szPinText)];
	pinsend = json["sendpin"];
#ifdef	RFCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("RF loadconfig");
#endif
	load_persist();
}
void IRController::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szPinText)] = pin;
	json["pinsend"] = pinsend;
	json[FPSTR(szservice)] = "IRController";
	json[FPSTR(szname)] = "IR";
	IR::getdefaultconfig(json);
}

void  IRController::setup() {
	IR::setup();
#ifdef ESP8266
	this->pReceiver = new IRrecv(this->pin);
	this->pReceiver->enableIRIn();
#endif
//	this->pSwitch = new RCSwitch();
//	this->pSwitch->enableReceive(this->pin);
	//pinMode(pin, INPUT);
	//digitalWrite(pin, LOW);
}

void IRController::run() {
	bool savepersist = false;
#ifdef ESP8266
	decode_results results;
	if (pReceiver->decode(&results)) {
		// print() & println() can't handle printing long longs. (uint64_t)
		command newcmd;
		newcmd.mode = IROnReceive;
		newcmd.state.irtoken = results.value;
		pReceiver->resume();  // Receive the next value

		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
		savepersist = this->store_recdata;
	}
#endif	
	command cmd;
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		if (cmd.mode == IRSend) {
			this->irsend(cmd.state);
		}
		if (cmd.mode == IRSaveReceive) {
			this->savepersist(cmd.state);
			continue; //state not changed
		}
		this->set_state(cmd.state);
}
	if (savepersist) {  ///will proceed next cycle
		this->AddCommand(cmd.state, IRSaveReceive, srcSelf);
	}

}
void IRController::savepersist(IRState psstate) {
#ifdef	IRCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("IR savepersist");

#endif
	bool exist = false;
	for (int i = 0;i < this->persistdata.GetSize();i++) {
		if (this->persistdata.GetAt(i).token == psstate.irtoken) {
			exist = true;
			break;
		}
	}
	if (!exist) {
#ifdef	IRCONTROLLER_DEBUG
		DBG_OUTPUT_PORT.println("RF savepersist to file");
#endif
		IRData dt(psstate);
		String uname = String(millis());
		strncpy(dt.name, uname.c_str(), IRDATANAME_MAXLEN);
		this->persistdata.Add(dt);
		this->saveperisttofile();
	}


}
String IRController::getfilename_data() {
	String filename = "/";
	filename += this->get_name();
	filename += "_data.json";
	return filename;
}
void IRController::load_persist() {
#ifdef	IRCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("IR load_persist");
#endif
	String filedata = readfile(getfilename_data().c_str());
	int capacity = JSON_ARRAY_SIZE(8) + 2 * JSON_OBJECT_SIZE(20) + 262;
	DynamicJsonDocument jsonBuffer(capacity);
	DeserializationError error = deserializeJson(jsonBuffer, filedata);
	if (error) {
		DBG_OUTPUT_PORT.print("RF load_persist error");
		DBG_OUTPUT_PORT.println(error.c_str());
		return;
	}
	JsonArray arr = jsonBuffer.as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		IRData dt;
		const char * szName = arr[i][FPSTR(szname)].as<char*>();
		strncpy(dt.name, szName, IRDATANAME_MAXLEN);
		dt.token = arr[i]["token"];;

		this->persistdata.Add(dt);
	}
#ifdef	IRCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.print("IR persist loaded count->");
	DBG_OUTPUT_PORT.println(this->persistdata.GetSize());
#endif
}
IRData IRController::deserializeIRData(String strdata) {
	const size_t jsonsize = JSON_OBJECT_SIZE(40);
	DynamicJsonDocument jsonBuffer(jsonsize);
	DeserializationError error = deserializeJson(jsonBuffer, strdata);
	if (error) {
		DBG_OUTPUT_PORT.print("deserializeIRData error");
		DBG_OUTPUT_PORT.println(error.c_str());
		IRData empty;
		return empty;
	}
	JsonObject json = jsonBuffer.as<JsonObject>();
	return deserializeIRData(json);
}
IRData IRController::deserializeIRData(JsonObject& json) {
	IRData dt;
	const char * szName = json[FPSTR(szname)].as<char*>();
	strncpy(dt.name, szName, IRDATANAME_MAXLEN);
	dt.token = json["token"];;

	return dt;
}
String IRController::serializeIRData(IRData data) {
	const size_t jsonsize = JSON_OBJECT_SIZE(40);
	DynamicJsonDocument jsonBuffer(jsonsize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["token"] = data.token;

	String json;
	json.reserve(40);
	serializeJson(root, json);

	return json;
}
String IRController::string_irdata() {
	const size_t jsonsize = JSON_ARRAY_SIZE(this->persistdata.GetSize() + 1) + this->persistdata.GetSize()*JSON_OBJECT_SIZE(40);
	DynamicJsonDocument jsonBuffer(jsonsize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	for (uint8_t i = 0; i < this->persistdata.GetSize(); i++) {
		JsonObject object = json.createNestedObject();
		IRData dt = this->persistdata.GetAt(i);
		object[FPSTR(szname)] = dt.name;
		object["token"] = dt.token;

	}
	String json_str;
	json_str.reserve(2048);
	serializeJson(json, json_str);

	return json_str;
}
void IRController::saveperisttofile() {
#ifdef	IRCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("IR saveperisttofile()");
	DBG_OUTPUT_PORT.println(getfilename_data());
	DBG_OUTPUT_PORT.println(this->string_rfdata());
#endif
	savefile(getfilename_data().c_str(), this->string_irdata());
}

void IRController::irsend(IRState sendstate) {
	IRState tosend = sendstate;
	
}

IRData* IRController::getdata_byname(String& name) {
	for (int i = 0;i < this->persistdata.GetSize();i++)
		if (name == this->persistdata.GetAt(i).name)
			return &this->persistdata.GetAt(i);
	return NULL;
}
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
void RFController::setuphandlers(ESP8266WebServer& server) {
	ESP8266WebServer* _server = &server;
#else
void RFController::setuphandlers(WebServer& server) {
	WebServer* _server = &server;
#endif
}
#endif

#if defined ASYNC_WEBSERVER
void IRController::setuphandlers(AsyncWebServer& server) {


	String path = "/";
	path += this->get_name();
	path += String("/get_data");
	IRController* self = this;
	server.on(path.c_str(), HTTP_GET, [self](AsyncWebServerRequest *request) {
		// DBG_OUTPUT_PORT.println("get modes request");
		DBG_OUTPUT_PORT.println(ESP.getFreeHeap());
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json",
			self->string_irdata().c_str());

		request->send(response);
		DBG_OUTPUT_PORT.println(ESP.getFreeHeap());

	});
	path = "/";
	path += this->get_name();
	path += String("/send");
	server.on(path.c_str(), HTTP_GET, [self](AsyncWebServerRequest *request) {
		DBG_OUTPUT_PORT.println("IR Controller send");
		if (!request->hasArg(FPSTR(szname)))
			return request->send(500, "text/plain", "BAD ARGS");
		String name = request->arg(FPSTR(szname));
		IRData* pData = self->getdata_byname(name);
		if (!pData)
			return request->send(500, "text/plain", "NOT EXIST");
		command cmd;
		pData->SetState(cmd.state);
		cmd.state.isSend = true;
		self->AddCommand(cmd.state, IRSend, srcUserAction);
		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", name);

		request->send(response);


	});


}
#endif