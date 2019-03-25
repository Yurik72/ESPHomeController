#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "RCSwitch.h"
#include "IRController.h"

//REGISTER_CONTROLLER(RFController)
REGISTER_CONTROLLER_FACTORY(IRController)

const size_t bufferSize = JSON_OBJECT_SIZE(5);



IRController::IRController() {
	this->pin = 0;
	this->pinsend = 0;
	//this->pSwitch = NULL;
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
	json["service"] = "RFController";
	json["name"] = "RF";
	IR::getdefaultconfig(json);
}

void  IRController::setup() {
	IR::setup();
//	this->pSwitch = new RCSwitch();
//	this->pSwitch->enableReceive(this->pin);
	//pinMode(pin, INPUT);
	//digitalWrite(pin, LOW);
}

void IRController::run() {
	bool savepersist = false;

	

}
void IRController::savepersist(IRState psstate) {
#ifdef	RFCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("RF savepersist");

#endif
	bool exist = false;
	for (int i = 0;i < this->persistdata.GetSize();i++) {
		if (this->persistdata.GetAt(i).token == psstate.irtoken) {
			exist = true;
			break;
		}
	}
	if (!exist) {
#ifdef	RFCONTROLLER_DEBUG
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
		const char * szName = arr[i]["name"].as<char*>();
		strncpy(dt.name, szName, IRDATANAME_MAXLEN);
		dt.token = arr[i]["token"];;

		this->persistdata.Add(dt);
	}
#ifdef	RFCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.print("RF persist loaded count->");
	DBG_OUTPUT_PORT.println(this->persistdata.GetSize());
#endif
}
IRData IRController::deserializeIRData(String strdata) {
	const size_t jsonsize = JSON_OBJECT_SIZE(40);
	DynamicJsonDocument jsonBuffer(jsonsize);
	DeserializationError error = deserializeJson(jsonBuffer, strdata);
	if (error) {
		DBG_OUTPUT_PORT.print("deserializeRFData error");
		DBG_OUTPUT_PORT.println(error.c_str());
		IRData empty;
		return empty;
	}
	JsonObject json = jsonBuffer.as<JsonObject>();
	return deserializeIRData(json);
}
IRData IRController::deserializeIRData(JsonObject& json) {
	IRData dt;
	const char * szName = json["name"].as<char*>();
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
String IRController::string_rfdata() {
	const size_t jsonsize = JSON_ARRAY_SIZE(this->persistdata.GetSize() + 1) + this->persistdata.GetSize()*JSON_OBJECT_SIZE(40);
	DynamicJsonDocument jsonBuffer(jsonsize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	for (uint8_t i = 0; i < this->persistdata.GetSize(); i++) {
		JsonObject object = json.createNestedObject();
		IRData dt = this->persistdata.GetAt(i);
		object["name"] = dt.name;
		object["token"] = dt.token;

	}
	String json_str;
	json_str.reserve(2048);
	serializeJson(json, json_str);

	return json_str;
}
void IRController::saveperisttofile() {
#ifdef	IRCONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("RF saveperisttofile()");
	DBG_OUTPUT_PORT.println(getfilename_data());
	DBG_OUTPUT_PORT.println(this->string_rfdata());
#endif
	savefile(getfilename_data().c_str(), this->string_rfdata());
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
			self->string_rfdata().c_str());

		request->send(response);
		DBG_OUTPUT_PORT.println(ESP.getFreeHeap());

	});
	path = "/";
	path += this->get_name();
	path += String("/send");
	server.on(path.c_str(), HTTP_GET, [self](AsyncWebServerRequest *request) {
		DBG_OUTPUT_PORT.println("IR Controller send");
		if (!request->hasArg("name"))
			return request->send(500, "text/plain", "BAD ARGS");
		String name = request->arg("name");
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