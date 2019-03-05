#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "RCSwitch.h"
#include "RFController.h"

REGISTER_CONTROLLER(RFController)
REGISTER_CONTROLLER_FACTORY(RFController)

const size_t bufferSize = JSON_OBJECT_SIZE(5);



RFController::RFController() {
	this->pin = 0;
	this->pinsend = 0;
	this->pSwitch = NULL;
	this->store_recdata = true;
}

String  RFController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["isReceive"] = this->get_state().isReceive;
	root["isSend"] = this->get_state().isSend;
	root["rftoken"] = this->get_state().rftoken;
	root["timetick"] = this->get_state().timetick;
	String json;
	json.reserve(64);
	serializeJson(root, json);

	return json;
}

bool  RFController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print("parseObject() failed: ");
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	RFState newState;
	//newState.isPressed = root["isPressed"];


	//	this->AddCommand(newState, Measure, src);
		//this->set_state(newState);
	return true;

}
void RFController::loadconfig(JsonObject& json) {
	RF::loadconfig(json);
	pin = json["pin"];
	pinsend = json["sendpin"];
	load_persist();
}
void RFController::getdefaultconfig(JsonObject& json) {
	json["pin"] = pin;
	json["pinsend"] = pinsend;
	json["service"] = "RFController";
	json["name"] = "RF";
	RF::getdefaultconfig(json);
}

void  RFController::setup() {
	RF::setup();
	this->pSwitch = new RCSwitch();
	this->pSwitch->enableReceive(this->pin);
	//pinMode(pin, INPUT);
	//digitalWrite(pin, LOW);
}

void RFController::run() {
	bool savepersist = this->store_recdata;

	if (this->pSwitch->available()) {
		command newcmd;
		newcmd.mode = OnReceive;
		newcmd.state.rftoken = this->pSwitch->getReceivedValue();
		newcmd.state.rfprotocol = this->pSwitch->getReceivedProtocol();
		newcmd.state.rfdatalen = this->pSwitch->getReceivedBitlength();
		newcmd.state.rfdelay = this->pSwitch->getReceivedDelay();
		newcmd.state.timetick = millis();
		DBG_OUTPUT_PORT.print("RFController receive:");
		DBG_OUTPUT_PORT.println(newcmd.state.rftoken);
		
		//Serial.println("received");
		//output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
		this->pSwitch->resetAvailable();
		savepersist &= true;
	}
	command cmd;
	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		if (cmd.mode == Send) {
			this->rfsend(cmd.state);
		}
		if (cmd.mode == SaveReceive) {
			this->savepersist(cmd.state);
			continue; //state not changed
		}
		this->set_state(cmd.state);
	}
	if (savepersist) {  ///will proceed next cycle
		this->AddCommand(cmd.state, SaveReceive, srcSelf);
	}

}
void RFController::savepersist(RFState psstate) {
	bool exist = false;
	for (int i = 0;i < this->persistdata.GetSize();i++) {
		if (this->persistdata.GetAt(i).token == psstate.rftoken) {
			exist = true;
			break;
		}
	}
	if (!exist) {
		RFData dt(psstate);
		String uname = String(millis());
		strncpy(dt.name, uname.c_str(), RFDATANAME_MAXLEN);
		this->persistdata.Add(dt);
		this->saveperisttofile();
	}

	
}
String RFController::getfilename_data() {
	String filename = this->get_name();
	filename += "_data.json";
	return filename;
}
void RFController::load_persist() {
	
	String filedata = readfile(getfilename_data().c_str());
	int capacity = JSON_ARRAY_SIZE(8) + 2 * JSON_OBJECT_SIZE(20) + 262;
	DynamicJsonDocument jsonBuffer(capacity);
	DeserializationError error = deserializeJson(jsonBuffer, filedata);
	JsonArray arr = jsonBuffer.as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		RFData dt;
		const char * szName=arr[i]["name"].as<char*>();
		strncpy(dt.name, szName, RFDATANAME_MAXLEN);
		dt.token= arr[i]["token"];;
		dt.len = arr[i]["len"];;
		dt.protocol = arr[i]["protocol"];;
		dt.pulse = arr[i]["pulse"];;
		this->persistdata.Add(dt);
	}
 }
String RFController::string_rfdata() {
	const size_t jsonsize = JSON_ARRAY_SIZE(this->persistdata.GetSize() + 1) + this->persistdata.GetSize()*JSON_OBJECT_SIZE(20);
	DynamicJsonDocument jsonBuffer(jsonsize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	for (uint8_t i = 0; i < this->persistdata.GetSize(); i++) {
		JsonObject object = json.createNestedObject();
		RFData dt = this->persistdata.GetAt(i);
		object["name"] = dt.name;
		object["token"] = dt.token;
		object["len"] = dt.len;
		object["protocol"] = dt.protocol;
		object["pulse"] = dt.pulse;
	}
	String json_str;
	json_str.reserve(2048);
	serializeJson(json, json_str);

	return json_str;
}
void RFController::saveperisttofile() {

	savefile(getfilename_data().c_str(), this->string_rfdata());
}

void RFController::rfsend(RFState sendstate) {
	this->pSwitch->enableTransmit(this->pinsend);

	this->pSwitch->setPulseLength(sendstate.rfdatalen);

	// Optional set protocol (default is 1, will work for most outlets)
	this->pSwitch->setProtocol(sendstate.rfprotocol);
	this->pSwitch->send(sendstate.rftoken, sendstate.rfdatalen);
	delay(100);
	this->pSwitch->disableTransmit();
}
RFData* RFController::getdata_byname(String& name) {
	for (int i = 0;i < this->persistdata.GetSize();i++)
		if (name == this->persistdata.GetAt(i).name)
			return &this->persistdata.GetAt(i);
	return NULL;
}
void RFController::setuphandlers(AsyncWebServer& server) {


	String path = "/";
	path += this->get_name();
	path += String("/get_data");
	RFController* self = this;
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
		DBG_OUTPUT_PORT.println("RF Controller send");
		if (!request->hasArg("name"))
			return request->send(500, "text/plain", "BAD ARGS");
		String name = request->arg("name");
		RFData* pData =self->getdata_byname(name);
		if(!pData)
			return request->send(500, "text/plain", "NOT EXIST");
		command cmd;
		pData->SetState(cmd.state);
		self->AddCommand(cmd.state, Send, srcUserAction);
		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", name);

		request->send(response);
		

	});


}