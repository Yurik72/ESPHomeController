
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

#if !defined(ESP8266)
#include <SPIFFS.h>
#endif

#include "config.h"
#include "Controllers.h"
#include "Triggers.h"
#include "BaseController.h"
//#include "RelayController.h"
//#include "TimeController.h"

//#include "RGBStripController.h"
//#include "LDRController.h"
//#include "BME280Controller.h"
//#include "ButtonController.h"
//#include "RFController.h"

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
#if defined ASYNC_WEBSERVER
#if defined(ESP8266)
#include <ESPAsyncWebServer.h>
#else
#include <ESPAsyncWebServer.h>
#endif 
#endif

#ifdef	ENABLE_NATIVE_HAP
extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#endif

static Controllers* _instance=NULL;

//REGISTER_CONTROLLER(RFController)
//REGISTER_CONTROLLER(LDRController)
Controllers::Controllers():
	triggers(*(new Triggers()))
{
	_instance = this;
	//globlog += "CTOR";
	lastWifiReconnectms = 0;
	isConnectingMode = false;
	setupphase = setup_phase_none;
	pTimeCtl = NULL;
	
}
CBaseController* Controllers::GetByName(const char* name) {
	for (int i = 0;i < this->GetSize();i++) {
		if (strcmp(name, this->GetAt(i)->get_name()) == 0)
			return this->GetAt(i);
	}
	return NULL;
}
void Controllers::raise_event(CBaseController* pSender, ControllerEvent evt, uint16_t evData) {
	for (int i = 0; i < this->GetSize(); i++) {
		this->GetAt(i)->on_event(pSender, evt, evData);
	}
}
Controllers* Controllers::getInstance() {
	return _instance;
}
void Controllers::setup_before_wifi() {

	
	if (setupphase != setup_phase_none)
		return;
	DBG_OUTPUT_PORT.println("Controllers::setup_before_wifi");
	this->loadconfig();
	//DBG_OUTPUT_PORT.println("Controllers::loadconfig done");
	for (int i = 0; i < this->GetSize(); i++) {
		CBaseController* ctl = this->GetAt(i);
		//DBG_OUTPUT_PORT.println(ctl->get_name());
		//DBG_OUTPUT_PORT.println("ctl->setup()");
		
		ctl->setup();
		//delay(1000);
		if (ctl->ispersiststate()) {
			DBG_OUTPUT_PORT.print(ctl->get_name());
			DBG_OUTPUT_PORT.println(" : Restore persist state");
			ctl->loadstate();
		}
		//DBG_OUTPUT_PORT.println("ctl->set_power_on()");
		ctl->set_power_on();
	}
	DBG_OUTPUT_PORT.println("Load services done ");
	setupphase = setup_phase_before_wifi;
	
}
void Controllers::setup() {

	//DBG_OUTPUT_PORT.println("Controllers::setup");
	setup_before_wifi();
	setupphase = setup_phase_after_wifi;
//	Factories::registerController("-", &global_LDRControllerFactory);
	//DBG_OUTPUT_PORT.println(globlog);
	bool ishapused = false;
	for (int i = 0; i < this->GetSize(); i++) {
		if (this->GetAt(i)->ishap) {
			ishapused = true;
			break;
		}
	}
#ifdef	ENABLE_NATIVE_HAP
	if (ishapused) {
		init_hap_storage();
		set_callback_storage_change(storage_changed);
		//DBG_OUTPUT_PORT.println((long)get_callback_storage_change());
		hap_setbase_accessorytype(accessory_type);
		hap_initbase_accessory_service(HOSTNAME, "Yurik72", "0", "EspHapCtl", VERSION);
	}
	else {
		DBG_OUTPUT_PORT.println("Home kit not activated, nothing to do");
	}
#endif
	//this->loadconfig();
	connectmqtt();

	for (int i = 0; i < this->GetSize(); i++) {
		CBaseController* ctl = this->GetAt(i);
		//ctl->setup();
		ctl->setup_after_wifi();
#ifdef	ENABLE_NATIVE_HAP
		ctl->setup_hap_service();
		//DBG_OUTPUT_PORT.println("setup done");
		//DBG_OUTPUT_PORT.println((long)get_callback_storage_change());
		ctl->notify_hap();
		//DBG_OUTPUT_PORT.println("notify done");
		//DBG_OUTPUT_PORT.println((long)get_callback_storage_change());
#endif
		ctl->set_isloaded(true);
		//ctl->set_power_on();
	}
	

	DBG_OUTPUT_PORT.println("Load services done ");
//	DBG_OUTPUT_PORT.println((long)get_callback_storage_change());

#ifdef	ENABLE_NATIVE_HAP
	if (ishapused) {
		DBG_OUTPUT_PORT.println("starting hap_init_homekit_server ");
		//set_callback_storage_change(storage_changed);
		hap_init_homekit_server();
		delay(100);// give short time to home kit
	}
#endif
	
}
void Controllers::loadconfig() {
	String filename = "/services.json";
	//int capacity = JSON_ARRAY_SIZE(5) + 5 * JSON_OBJECT_SIZE(70);
	int capacity = JSON_ARRAY_SIZE(40) + 2 * JSON_OBJECT_SIZE(80) + 262;
	if (SPIFFS.exists(filename)) {
		//file exists, reading and loading
		DBG_OUTPUT_PORT.println("Read services configuration: ");
		File configFile = SPIFFS.open(filename, "r");
		if (configFile) {
			DynamicJsonDocument jsonBuffer(capacity);
			//const char* jsonchar = "[{\"service\":\"RelayController\",\"name\":\"Relay\",\"enabled\":true,\"interval\":100},{\"service\":\"TimeController\",\"name\":\"Time\",\"enabled\":true,\"interval\":1000}]";
			//DBG_OUTPUT_PORT.println(configFile.readString());
			DeserializationError error = deserializeJson(jsonBuffer,configFile.readString());
			
			if (!error) {
			
				JsonArray arr = jsonBuffer.as<JsonArray>();
				for (int i = 0; i < arr.size(); i++) {
					String servicename= arr[i][FPSTR(szservice)].as<String>();
					//DBG_OUTPUT_PORT.print("Controllers before added:");
					//DBG_OUTPUT_PORT.println(servicename);
					CBaseController* controller= Controllers::CreateByName(servicename.c_str());
					if (controller == NULL) {
						DBG_OUTPUT_PORT.println(String("Service not found:")+servicename);
					}
					else {
						controller->onstatechanged = &onstatechanged;
						String name = arr[i][FPSTR(szname)].as<String>();
						controller->set_name(name.c_str());
						JsonObject json = arr[i];
						controller->loadconfigbase(json);
						this->Add(controller);
						if (controller->statemon) {
							this->pMonitor = controller;
						}
						#ifdef	ENABLE_NATIVE_HAP
						if(setupphase == setup_phase_after_wifi)
							controller->setup_hap_service();
						#endif

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
	DBG_OUTPUT_PORT.println("Triggers load config done");
}
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
void Controllers::setuphandlers(ESP8266WebServer& server){
	ESP8266WebServer* _server = &server;
#else
void Controllers::setuphandlers(WebServer& server){
	WebServer* _server = &server;
#endif
	_server->on("/get_info", HTTP_GET, [=]() {

		String info = "{\"version\":\"";
		info += VERSION;
		info += "\",\"async\":";
		info += ASYNC;
		info += ",\"hostname\":\"";
		info += HOSTNAME;
		info += "\"}";

		_server->sendHeader("Access-Control-Allow-Origin", "*");
		_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		_server->send(200, PSTR("application/json"), info.c_str());
	});
	_server->on("/get_availablecontrollers", HTTP_GET, [=]{
		//DBG_OUTPUT_PORT.println("get_availablecontrollers");
		_server->sendHeader("Access-Control-Allow-Origin", "*");
		_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		_server->send(200, PSTR("application/json"), Factories::string_controllers().c_str());


	});
	_server->on("/get_availabletriggers", HTTP_GET, [=] {
		//DBG_OUTPUT_PORT.println("get_availablecontrollers");
		_server->sendHeader("Access-Control-Allow-Origin", "*");
		_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		_server->send(200, PSTR("application/json"), Factories::string_triggers().c_str());


	});
	_server->on("/get_log", HTTP_GET, [=] {
		_server->sendHeader("Access-Control-Allow-Origin", "*");
		_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		_server->send(200, PSTR("application/json"), GET_CONSTCHARGLOG);

	});
	_server->on("/get_defaultconfig", HTTP_GET, [=] {
		DBG_OUTPUT_PORT.println("get_defaultconfig request");
		String cname;
		if (_server->args() > 0)
			cname = _server->arg((size_t)0);
		DBG_OUTPUT_PORT.println(cname);
		if (cname.length() == 0) {
			_server->send(500, "text/plain", "BAD ARGS");
			return;
		}
		CBaseController* pcontroller = Controllers::CreateByName(cname.c_str());
		if (pcontroller == NULL) {
			_server->send(500, "text/plain", "Non exists controller name");
			return;
		}
		_server->sendHeader("Access-Control-Allow-Origin", "*");
		_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
		_server->send(200, PSTR("application/json"), pcontroller->getdefaultconfig().c_str());

		delete pcontroller;
	});
	for (int i = 0;i < this->GetSize();i++) {
		CBaseController*ctl = this->GetAt(i);
		String path = "/";
		path+=ctl->get_name();
		String pathget = path+String("/get_state");
		_server->on(pathget, HTTP_GET, [=]() {
			
			//DBG_OUTPUT_PORT.println("start processing");
			//_server->send_P(200, PSTR("text/html"), "OK");
			_server->sendHeader("Access-Control-Allow-Origin", "*");
			_server->sendHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
			_server->send_P(200, PSTR("application/json"), ctl->serializestate().c_str());
			//_server->send(200, PSTR("application/json"), "{ \"isOn\":false }");
			
			//DBG_OUTPUT_PORT.println("Processed");
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
			//DBG_OUTPUT_PORT.println("sending header from POST");
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
#endif
#if defined ASYNC_WEBSERVER
void Controllers::setuphandlers(AsyncWebServer& server) {
	AsyncWebServer* _server = &server;
	server.on("/get_info", HTTP_GET, [](AsyncWebServerRequest *request) {

		String info = "{\"version\":\"";
		info += VERSION;
		info += "\",\"async\":";
		info += ASYNC;
		info += ",\"hostname\":\"";
		info += HOSTNAME;
		info += "\",\"mem\":";
		info += String(ESP.getFreeHeap());
		info += "}";
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", info);
		//response->addHeader("Access-Control-Allow-Origin", "*");
		//response->addHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");

		request->send(response);
	});
	server.on("/get_log", HTTP_GET, [](AsyncWebServerRequest *request) {

		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", GET_CONSTCHARGLOG);
	
		request->send(response);
	});
	server.on("/get_defaultconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
		//DBG_OUTPUT_PORT.println("get_defaultconfig request");
		String cname;
		if(request->args()>0)
			cname = request->arg((size_t)0);
		//DBG_OUTPUT_PORT.println(cname);
		
		if (cname.length() == 0) {
			request->send(500, "text/plain", "BAD ARGS");
			return;
		}
		CBaseController* pcontroller = Controllers::CreateByName(cname.c_str());
		if (pcontroller == NULL) {
			request->send(500, "text/plain", "Non exists controller name");
			return;
		}
		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", pcontroller->getdefaultconfig().c_str());
		request->send(response);
		delete pcontroller;
	});
	server.on("/get_availablecontrollers", HTTP_GET, [](AsyncWebServerRequest *request) {
		//DBG_OUTPUT_PORT.println("get_availablecontrollers");

		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", Factories::string_controllers().c_str());
		request->send(response);
		
	});
	server.on("/get_availabletriggers", HTTP_GET, [](AsyncWebServerRequest *request) {
		//DBG_OUTPUT_PORT.println("get_availablecontrollers");

		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", Factories::string_triggers().c_str());
		request->send(response);

	});
	for (int i = 0;i < this->GetSize();i++) {
		CBaseController*ctl = this->GetAt(i);
		String path = "/";
		path += ctl->get_name();
		//DBG_OUTPUT_PORT.println("setup async handlers");
		//DBG_OUTPUT_PORT.println((int)ctl);
		String pathget = path + String("/get_state");
		server.on(pathget.c_str(), HTTP_GET, [ctl](AsyncWebServerRequest *request) {

			AsyncWebServerResponse *response = request->beginResponse(200, "application/json", ctl->serializestate().c_str());
			
			request->send(response);
		});
		String pathset = path + String("/set_state");
		server.on(pathset.c_str(), HTTP_OPTIONS, [] (AsyncWebServerRequest *request){
			//DBG_OUTPUT_PORT.println("start processing preflight");
			AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "OK");
			//response->addHeader("Access-Control-Allow-Origin", "*");
			//response->addHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
			response->addHeader("Access-Control-Allow-Headers", "origin, content-type, accept");
			request->send(response);
		});
		server.on(pathset.c_str(), HTTP_POST, [](AsyncWebServerRequest *request) {
			//DBG_OUTPUT_PORT.println("sending header from POST");
			AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
			response->addHeader("Connection", "close");
			request->send(response);
		},[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
			
		},[ctl](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
			//DBG_OUTPUT_PORT.println("body load");
			//DBG_OUTPUT_PORT.printf("len %d, index %d, total %d ",len,index,total);
			//DBG_OUTPUT_PORT.println("");
			uint8_t * bodydata = NULL;
			if (index == 0) {//starts
				bodydata=ctl->allocatebuffer(total);
			}
			else { ///continue
				bodydata = ctl->bodybuffer;
			}
			memcpy(bodydata+ ctl->bodyindex, data, len);
			ctl->bodyindex += len;
			if (ctl->bodyindex >= total) { //all collected
				String body = (char*)bodydata;
				//DBG_OUTPUT_PORT.println(body);
				DBG_OUTPUT_PORT.println("Free Heap: " + String(ESP.getFreeHeap()));
				if (body.length() > 0) {
					ctl->deserializestate(body);
				}
				ctl->cleanbuffer();
				//DBG_OUTPUT_PORT.println("Buffer cleaned");
			}


		}
		);
		this->GetAt(i)->setuphandlers(server);
	}
		
}
#endif
CBaseController* Controllers::CreateByName(const char* name) { //to be rewrite by class factory
	
	//Factories::Trace();
	//tst
	return Factories::CreateController(name);
	//ControllerFactory* pFactory = Factories::get_ctlfactory(name);
	//if (pFactory) {
		
	//	return pFactory->create();
	//}
	//return NULL;
	/*
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
	}
	else if (strcmp(name, "RFController") == 0) {
		return new RFController();
	}
	else if (strcmp(name, "BME280Controller") == 0) {
		return new BME280Controller();
	};



	return NULL;
	*/
};
void Controllers::handleloops() {
#if defined(ESP8266)
	bLoopSwitch = !bLoopSwitch;
#endif	
#if defined(ENABLE_NATIVE_HAP) && defined(ESP8266)
	if (bLoopSwitch) {
		hap_homekit_loop();
		return;
	}
#endif
	for (int i = 0; i < this->GetSize(); i++) {
		CBaseController*ctl = this->GetAt(i);

		
		if (ctl->shouldRun() && (ctl->get_coremode() == NonCore  || ctl->get_coremode()==Both) && ctl->isenabled()) {
			
			this->checkandreconnectWifi();

			ctl->run();

			for (int j = 0;j < this->triggers.GetSize();j++) {
				Trigger* tr = this->triggers.GetAt(j);

				if (strcmp(ctl->get_name(), tr->get_src()) == 0) {

					tr->handleloop(ctl, this);
				}
			}
		}
	}

}
void Controllers::set_monitor_state(uint channel, bool isOn, long mask, uint masklen, uint duration) {
	
	if (pMonitor) {
		
		pMonitor->set_monitor_state(channel, isOn, mask, masklen, duration);
	}
}
void Controllers::set_isneedreconnectwifi(bool val) {
	this->isneedreconnectwifi = val;
}
void Controllers::checkandreconnectWifi() {
	if (this->isneedreconnectwifi && !isAPMode) {
		if (!WiFi.isConnected()) {
			this->isWifiConnected = false;
			if ((this->lastWifiReconnectms + DELAY_MS_RECONNECT) < millis()) {
				this->lastWifiReconnectms = millis();
				DBG_OUTPUT_PORT.println("Controllers detect lost wifi");
				WiFi.reconnect();
				unsigned long delaytime = millis() + 2000;
				isConnectingMode = true;
				while (!WiFi.isConnected() && delaytime > millis()) { //let chanse to wifi to be established 
					delay(200);
				}
				DBG_OUTPUT_PORT.println(String("Reconnect result")+String(WiFi.isConnected()));
			}
		}
		if (isConnectingMode) {
			if (WiFi.isConnected()) {
				this->isWifiConnected = true;
				isConnectingMode = false; 
					
				//DBG_OUTPUT_PORT.println("Wifi Restored connection");
#ifdef ENABLE_HOMEBRIDGE
				if (strlen(mqtt_host) > 0 && atoi(mqtt_port) > 0)
					mqttReconnectTimer.attach(2, realconnectToMqtt);
#endif
			}
		}
	}
}
void onstatechanged(CBaseController * ctl)
{

#ifdef	ENABLE_NATIVE_HAP
	if(ctl->get_ishap())
		ctl-> notify_hap();
#endif
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

	//int topiccount = ctl->onpublishmqttex(endkeys, endpayloads);
	for (int i = 0;i < 5;i++) {   ///to do
		if (!ctl->onpublishmqttex(endkey, payload,i))
			break;

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
void publishinitialstate() {
	DBG_OUTPUT_PORT.println("MQTT: Publishing initial state");
	if (!_instance)
		return;
	for (int i = 0;i < _instance->GetSize();i++)
		onstatechanged(_instance->GetAt(i));
}
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
	if (strlen(mqtt_host) > 0 && atoi(mqtt_port) > 0 && WiFi.isConnected()) {
		DBG_OUTPUT_PORT.println("Connecting to MQTT...");
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
	free(payload);
}
void realconnectToMqtt() {
	DBG_OUTPUT_PORT.println("Connecting to MQTT...");
	mqttReconnectTimer.detach();
	amqttClient.connect();
}
#endif

void Controllers::onWifiDisconnect() {
	this->isWifiConnected = false;
}

#ifdef ENABLE_NATIVE_HAP
void init_hap_storage(){
	DBG_OUTPUT_PORT.print("init_hap_storage");
	String fname="/pair.dat";
	File fsDAT=SPIFFS.open(fname, "r");
	if(!fsDAT){
		//DBG_OUTPUT_PORT.println("Failed to read pair.dat");
		//SPIFFS.remove(fname);
		//fsDAT = SPIFFS.open(fname, "w+");
		if (!fsDAT) {
			DBG_OUTPUT_PORT.println("Failed to create pair.dat");
			return;
		}
	}
	int size=hap_get_storage_size_ex();
	char* buf=new char[size];
	memset(buf,0xff,size);
	int readed=fsDAT.readBytes(buf,size);
	DBG_OUTPUT_PORT.print("Readed bytes ->");
	DBG_OUTPUT_PORT.println(readed);
	hap_init_storage_ex(buf,size);
	fsDAT.close();
	delete []buf;

}
void storage_changed(char * szstorage,int size){
	//DBG_OUTPUT_PORT.println("storage changed");
	//DBG_OUTPUT_PORT.println(szstorage);
	//DBG_OUTPUT_PORT.println(size);
	//DBG_OUTPUT_PORT.println(szstorage+7);
	String fname="/pair.dat";
	///char*  x=new char[size];
	//memset(x,0xff,size);
	//memset(x+5,0x00,5);
	//DBG_OUTPUT_PORT.println(x);
	//DBG_OUTPUT_PORT.println("storage_changed");
	SPIFFS.remove(fname);
	File fsDAT=SPIFFS.open(fname, "w+");
	if(!fsDAT){
		DBG_OUTPUT_PORT.println("Failed to open pair.dat");
		return;
	}
	fsDAT.write((uint8_t*)szstorage, size);
	//fsDAT.write((uint8_t*)x, size);
	fsDAT.close();
}

#endif