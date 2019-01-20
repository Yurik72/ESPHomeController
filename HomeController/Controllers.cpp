#include "config.h"
#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif
#include <ArduinoJson.h>

#include "RelayController.h"
#include "TimeController.h"
#include "Triggers.h"
#include "RGBStripController.h"
#include "LDRController.h"
#include "Controllers.h"
#include <WiFiClient.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>          
#else
#include <WiFi.h>          
#endif
#ifdef ENABLE_HOMEBRIDGE

AsyncMqttClient amqttClient;
Ticker mqttReconnectTimer;
#endif
static Controllers* _instance;

Controllers::Controllers():
	triggers(*(new Triggers()))
{
	_instance = this;
	
}
CBaseController* Controllers::GetByName(const char* name) {
	for (int i = 0;i < this->GetSize();i++) {
		if (strcmp(name, this->GetAt(i)->get_name()) == 0)
			return this->GetAt(i);
	}
	return NULL;
}
void Controllers::setup() {

	this->loadconfig();
	connectmqtt();
	for (int i = 0; i < this->GetSize(); i++)
		this->GetAt(i)->setup();

}
void Controllers::loadconfig() {
	String filename = "/services.json";
	//int capacity = JSON_ARRAY_SIZE(5) + 5 * JSON_OBJECT_SIZE(70);
	int capacity = JSON_ARRAY_SIZE(5) + 2 * JSON_OBJECT_SIZE(4) + 262;
	if (SPIFFS.exists(filename)) {
		//file exists, reading and loading
		DBG_OUTPUT_PORT.println("Read services configuration: ");
		File configFile = SPIFFS.open(filename, "r");
		if (configFile) {
			DynamicJsonDocument jsonBuffer(capacity);
			//const char* jsonchar = "[{\"service\":\"RelayController\",\"name\":\"Relay\",\"enabled\":true,\"interval\":100},{\"service\":\"TimeController\",\"name\":\"Time\",\"enabled\":true,\"interval\":1000}]";

			DeserializationError error = deserializeJson(jsonBuffer,configFile.readString());
			
			if (!error) {
			
				JsonArray arr = jsonBuffer.as<JsonArray>();
				for (int i = 0; i < arr.size(); i++) {
					String servicename= arr[i]["service"].as<String>();
					CBaseController* controller= Controllers::CreateByName(servicename.c_str());
					if (controller == NULL) {
						DBG_OUTPUT_PORT.print(String("Service not found:")+servicename);
					}
					else {
						String name = arr[i]["name"].as<String>();
						controller->set_name(name.c_str());
						JsonObject json = arr[i];
						controller->loadconfigbase(json);
						this->Add(controller);

						controller->onstatechanged = &onstatechanged;

						DBG_OUTPUT_PORT.print("Controllers added:");
						DBG_OUTPUT_PORT.println(name);
					}
				}
			}
			else {
				DBG_OUTPUT_PORT.print("Deserialize error:");
				DBG_OUTPUT_PORT.println(error.c_str());
			}
		}
	}
	else {
		DBG_OUTPUT_PORT.println("File not found services.json");
	}
	
	triggers.loadconfig();
}
#if defined(ESP8266)
void Controllers::setuphandlers(ESP8266WebServer& server){
	ESP8266WebServer* _server = &server;
#else
void Controllers::setuphandlers(WebServer& server){
	WebServer* _server = &server;
#endif
	for (int i = 0;i < this->GetSize();i++) {
		CBaseController*ctl = this->GetAt(i);
		String path = "/";
		path+=ctl->get_name();
		String pathget = path+String("/get_state");
		_server->on(pathget, HTTP_GET, [=]() {
			DBG_OUTPUT_PORT.println("start processing");
			//_server->send_P(200, PSTR("text/html"), "OK");
			_server->sendHeader("Access-Control-Allow-Origin", "*");
			_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
			_server->send_P(200, PSTR("text/html"), ctl->serializestate().c_str());
			DBG_OUTPUT_PORT.println("Processed");
		});
		String pathset = path + String("/set_state");
		///to support CORS PREFLIGHT requests//  cross domain feathure
		_server->on(pathset, HTTP_OPTIONS, [=]() {
			DBG_OUTPUT_PORT.println("start processing preflight");
			//_server->send_P(200, PSTR("text/html"), "OK");
			_server->sendHeader("Access-Control-Allow-Origin", "*");
			_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
			_server->sendHeader("Access-Control-Allow-Headers", "origin, content-type, accept");
			_server->send_P(200, PSTR("text/html"), "OK");
			DBG_OUTPUT_PORT.println("Processed preflight");
		});
		_server->on(pathset, HTTP_POST, [=]() {
			DBG_OUTPUT_PORT.println("sending header from POST");
			String body;
			if (_server->hasArg("plain")) {
				DBG_OUTPUT_PORT.println("has plain");
				body = _server->arg("plain");
				DBG_OUTPUT_PORT.println(body);
			}
			else {
				DBG_OUTPUT_PORT.println("DO NOT has splain");
			}
			_server->sendHeader("Connection", "close");
			_server->send(200, "text/plain",  "OK");
			if (body.length() > 0) {
				ctl->deserializestate(body);
			}

		});
		this->GetAt(i)->setuphandlers(server);
	}
	//this->loadconfig();
}
CBaseController* Controllers::CreateByName(const char* name) { //to be rewrite by class factory
	if (strcmp(name, "RelayController") == 0) {
		return new RelayController();
	}
	else if (strcmp(name, "TimeController") == 0) {
		return new TimeController();
	}
	else if (strcmp(name, "RGBStripController") == 0) {
		return new RGBStripController();
	}
	else if (strcmp(name, "LDRController") == 0) {
		return new LDRController();
	};
	return NULL;
};
void Controllers::handleloops() {
	for (int i = 0; i < this->GetSize(); i++) {
		CBaseController*ctl = this->GetAt(i);


		if (ctl->shouldRun() && !ctl->get_iscore() && ctl->isenabled()) {

			ctl->run();

			for (int j = 0;j < this->triggers.GetSize();j++) {
				Trigger* tr = this->triggers.GetAt(j);
				if (strcmp(ctl->get_name(), tr->get_src()) == 0)
					tr->handleloop(ctl,this);
			}
		}
	}
}

void onstatechanged(CBaseController * ctl)
{
#ifdef ENABLE_HOMEBRIDGE
	String endkey;
	String payload;
	if (ctl->onpublishmqtt(endkey, payload)) {
		String outtopic = (String)HOSTNAME;
		outtopic += "/out/";
		outtopic += ctl->get_name();
		outtopic += "/";
		outtopic += endkey;
		amqttClient.publish(outtopic.c_str(), qossub, false, payload.c_str());
	}
		 
#endif
}
void Controllers::connectmqtt() {
#ifdef ENABLE_HOMEBRIDGE
	if (strlen(mqtt_host)>0 && atoi(mqtt_port) > 0) {
		amqttClient.onConnect(onMqttConnect);
		amqttClient.onDisconnect(onMqttDisconnect);
		amqttClient.onMessage(onMqttMessage);
		amqttClient.setServer(mqtt_host, atoi(mqtt_port));
		if (mqtt_user != "" || mqtt_pass != "") amqttClient.setCredentials(mqtt_user, mqtt_pass);
		amqttClient.setClientId(HOSTNAME);

		realconnectToMqtt();
	}
#endif
}
#ifdef ENABLE_HOMEBRIDGE
void onMqttConnect(bool sessionPresent) {
	String inTopic = String(HOSTNAME) + String("/in/#");
	DBG_OUTPUT_PORT.print("MQTT: Subscribing : ");
	DBG_OUTPUT_PORT.println(inTopic);
	uint16_t packetIdSub1 = amqttClient.subscribe((char *)inTopic.c_str(), qossub);
}
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
	if (WiFi.isConnected()) {
		Serial.println("Connecting to MQTT...");
		mqttReconnectTimer.attach(2, realconnectToMqtt);
	}
}
void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
	DBG_OUTPUT_PORT.print("MQTT: Recieved "); 
	DBG_OUTPUT_PORT.println(topic);
	String stopic = topic;
	if (!stopic.startsWith((String)HOSTNAME)) {
		DBG_OUTPUT_PORT.print("MQTT: Ignore");
		return;
	}
	stopic.replace((String)HOSTNAME + (String)"/", "");
	char *pDelim = strtok((char*)stopic.c_str(), "/");
	char* pchildName = NULL;
	char* pchildTopic = NULL;
	if (pDelim != NULL) {
		pchildName= strtok(NULL, "/");
		if(pchildName)
			pchildTopic = strtok(NULL, "/");
	}
	if (pDelim == NULL) {
		DBG_OUTPUT_PORT.println("MQTT: no subtopic, ignored");
		return;
	}

	DBG_OUTPUT_PORT.println(pchildName);
	CBaseController* pController = _instance->GetByName(pchildName);
	if(pController==NULL) {
		DBG_OUTPUT_PORT.println("MQTT: no controllers, ignored");
		return;
	}
	uint8_t * payload = (uint8_t *)malloc(length + 1);
	memcpy(payload, payload_in, length);
	payload[length] = NULL;

	String spayload_in = (char*)payload;;
	String schildtopic = pchildTopic;
	pController->onmqqtmessage(pchildTopic, spayload_in);
}
void realconnectToMqtt() {
	DBG_OUTPUT_PORT.println("Connecting to MQTT...");
	mqttReconnectTimer.detach();
	amqttClient.connect();
}
#endif