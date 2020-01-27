
#include "BaseController.h"

#include "config.h"
#if defined ASYNC_WEBSERVER
#include <ESPAsyncWebServer.h>
#endif
#include "Controllers.h"
#if defined(ESP8266)
#include <Ticker.h>
#endif

//CSimpleArray<ControllerRecord*> Factories::ctlfactories;
static CSimpleArray<ControllerRecord*>*   pctlfactories=NULL;
static CSimpleArray<TriggerRecord*>* ptriggerfactories=NULL;


void ICACHE_FLASH_ATTR Factories::registerController(const __FlashStringHelper* name, func_create_ctl f)
{

	if (!pctlfactories)
		pctlfactories = new CSimpleArray<ControllerRecord*>();

	ControllerRecord* prec = new ControllerRecord(name, f);
	pctlfactories->Add(prec);

};
void ICACHE_FLASH_ATTR Factories::registerTrigger(const __FlashStringHelper* name,  func_create_trg f)
{
	//DBG_OUTPUT_PORT.print("registerTrigger");
	if (!ptriggerfactories)
		ptriggerfactories = new CSimpleArray<TriggerRecord*>();
	TriggerRecord* prec = new TriggerRecord(name, f);
	ptriggerfactories->Add(prec);
};
ControllerFactory* ICACHE_FLASH_ATTR Factories::get_ctlfactory(const  String& name) {
	if (!pctlfactories)
		return NULL;
	/*
	for (int i = 0;i < pctlfactories->GetSize();i++)
		//if (pctlfactories->GetAt(i)->name == name)
	   if(strcmp_P(name.c_str(), (PGM_P)pctlfactories->GetAt(i)->name)==0)
			   return pctlfactories->GetAt(i)->pCtl;
			   */
	return NULL;
	
};
CBaseController* ICACHE_FLASH_ATTR Factories::CreateController(const  String& name) {
#ifdef FACTORY_DEBUG
	DBG_OUTPUT_PORT.print("CreateController ->");
	DBG_OUTPUT_PORT.println(name);
#endif
	if (!pctlfactories) {
#ifdef FACTORY_DEBUG
		DBG_OUTPUT_PORT.println("factories is not defined");
		return NULL;
#endif
	}
	for (int i = 0;i < pctlfactories->GetSize();i++)
		//if (pctlfactories->GetAt(i)->name == name)
		if (strcmp_P(name.c_str(), (PGM_P)pctlfactories->GetAt(i)->name) == 0) {
#ifdef FACTORY_DEBUG
			DBG_OUTPUT_PORT.println("Factory found");
#endif
			ControllerRecord* pRec = pctlfactories->GetAt(i);
			if (pRec->f) {
#ifdef FACTORY_DEBUG
				DBG_OUTPUT_PORT.println("using functions");
#endif
				return pRec->f();
			}
#ifdef FACTORY_DEBUG
			DBG_OUTPUT_PORT.println("using class factory method");
#endif
			return NULL;// pRec->pCtl->create();
		}
	return NULL;
}
/*
void  ICACHE_FLASH_ATTR Factories::Trace() {
	
	DBG_OUTPUT_PORT.print("Factory size: ");
	if (pctlfactories)
	 DBG_OUTPUT_PORT.println(pctlfactories->GetSize());
	
//	for (int i = 0;i < Factories::ctlfactories.GetSize();i++)
//		DBG_OUTPUT_PORT.println(Factories::ctlfactories.GetAt(i)->name);
			
};
*/
/*
TriggerFactory* ICACHE_FLASH_ATTR Factories::get_triggerfactory(const  String& name) {
	if (!ptriggerfactories) {
		DBG_OUTPUT_PORT.println("Factories not created");
		return NULL;
	}

	for (int i = 0;i < ptriggerfactories->GetSize();i++)
		//if (ptriggerfactories->GetAt(i)->name == name)
		if (strcmp_P(name.c_str(), (PGM_P)ptriggerfactories->GetAt(i)->name) == 0)
			return ptriggerfactories->GetAt(i)->pCtl;
	return NULL;
};
*/
 Trigger* ICACHE_FLASH_ATTR Factories::CreateTrigger(const  String& name) {
	if (!ptriggerfactories) {
		DBG_OUTPUT_PORT.println(F("Trigger Factories not created"));
		return NULL;
	}

	for (int i = 0;i < ptriggerfactories->GetSize();i++)
		//if (ptriggerfactories->GetAt(i)->name == name)
		if (strcmp_P(name.c_str(), (PGM_P)ptriggerfactories->GetAt(i)->name) == 0) {
			TriggerRecord* pRec = ptriggerfactories->GetAt(i);
			if (pRec->f) {
				DBG_OUTPUT_PORT.println(F("Trigger creation fron function pointer"));
				return pRec->f();
			}
			//return ptriggerfactories->GetAt(i)->pCtl->create();
		}
	return NULL;
}
String ICACHE_FLASH_ATTR Factories::string_controllers(void) {
	if (!pctlfactories)
		return "";
	const size_t bufferSize = JSON_ARRAY_SIZE(pctlfactories->GetSize() + 1) + pctlfactories->GetSize()*JSON_OBJECT_SIZE(2);
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	for (int i = 0; i < pctlfactories->GetSize(); i++) {
		JsonObject object = json.createNestedObject();
		
		object[FPSTR(szname)] = pctlfactories->GetAt(i)->name;
	}
//	JsonObject object = json.createNestedObject();

	String json_str;
	json_str.reserve(2048);
	serializeJson(json, json_str);
	
	return json_str;
}
String ICACHE_FLASH_ATTR Factories::string_triggers(void) {

	const size_t bufferSize = JSON_ARRAY_SIZE(ptriggerfactories->GetSize() + 1) + ptriggerfactories->GetSize()*JSON_OBJECT_SIZE(2);
	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonArray json = jsonBuffer.to<JsonArray>();
	for (uint8_t i = 0; i < ptriggerfactories->GetSize(); i++) {
		JsonObject object = json.createNestedObject();

    	object["name"] = ptriggerfactories->GetAt(i)->name;
	}
	JsonObject object = json.createNestedObject();

	String json_str;
	json_str.reserve(4096);
	serializeJson(json, json_str);

	return json_str;
}
CBaseController::CBaseController() {
	this->coreMode = NonCore;
	this->core = 0;
	this->enabled = false;
	this->_cached_next_run = 0;
	this->onstatechanged = NULL;
	this->bodybuffer = NULL;
	this->bodyindex = 0;
	this->interval = 1000;
	this->priority = 1;
	this->isforcedinterval = false;
	this->statemon = false;
	this->accessory_type=1;
	this->ishap=false;
	
#if defined(ESP8266)
	this->pTicker = NULL;;
#endif
}
void CBaseController::set_name(const char* name) {
	strncpy(this->name, name, MAXLEN_NAME);
}
void CBaseController::cleanbuffer() {
	if (bodybuffer != NULL)
		free(bodybuffer);
	this->bodybuffer = NULL;
	this->bodyindex = 0;
}
uint8_t * CBaseController::allocatebuffer(size_t size) {
	this->cleanbuffer();
	this->bodybuffer=(uint8_t *)malloc(size + 1);
	this->bodybuffer[size] = NULL;
	this->bodyindex = 0;
	return this->bodybuffer;
}
String CBaseController::get_filename_state() {
	
	String file = "/";
	file+=	this->get_name();
	file += "_state.json";
	return file;
}
void CBaseController::savestate() {
	//DBG_OUTPUT_PORT.println("savestate");
	//DBG_OUTPUT_PORT.println(this->get_filename_state().c_str());
	savefile(this->get_filename_state().c_str(), this->serializestate());

	
}
void CBaseController::set_monitor_state(uint channel, bool isOn, long mask , uint masklen, uint duration) {

}
void CBaseController::report_monitor_state(uint channel, bool isOn, long mask, uint masklen, uint duration) {
	Controllers::getInstance()->set_monitor_state(channel, isOn, mask, masklen, duration);
}
void CBaseController::report_monitor_on(uint channel) {
	this->report_monitor_state(channel, true, 0b11, 2, 1000);
}
void CBaseController::report_monitor_off(uint channel) {
	this->report_monitor_state(channel, false);
}
void CBaseController::report_monitor_shortblink(uint channel) {
	this->report_monitor_state(channel, true, 0b10, 2, 500);
}
void CBaseController::force_nextruninterval(unsigned long forceinterval) {
	_cached_next_run = millis() + forceinterval;
	this->isforcedinterval = true;
}
bool CBaseController::shouldRun(unsigned long time) {
	// If the "sign" bit is set the signed difference would be negative
	bool time_remaining = (time - _cached_next_run) & 0x80000000;

	// Exceeded the time limit, AND is enabled? Then should run...
	return !time_remaining && enabled;
}
void CBaseController::runned(unsigned long time) {
	// Saves last_run
	last_run = time;

	// Cache next run
	if(!this->isforcedinterval)
		_cached_next_run = last_run + interval;
	this->isforcedinterval = false;
}
void CBaseController::run() {

	// Update last_run and _cached_next_run
	//DBG_OUTPUT_PORT.println("Base run");
	runned();
}
void CBaseController::runcore() {

	// Update last_run and _cached_next_run
	//DBG_OUTPUT_PORT.println("Base run");
	//	runned();
}
void CBaseController::loadconfig(JsonObject& json) {

}
String CBaseController::getdefaultconfig() {
	DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(40));
	JsonObject root = jsonBuffer.to<JsonObject>();
	root["enabled"]= enabled;
	root["interval"]= interval;
	
	this->getdefaultconfig(root);
	String json;
	json.reserve(200);
	serializeJson(root, json);

	return json;
}
void CBaseController::getdefaultconfig(JsonObject& json) {

}
void CBaseController::loadconfigbase(JsonObject& json) {
	enabled = json["enabled"];
	statemon= json["statemon"];
	loadif(repch, json, "repch");
	loadif(ishap,json,"ishap");
#ifdef	ENABLE_NATIVE_HAP
	loadif(accessory_type,json,"acctype");

#endif
	interval= json["interval"].as<unsigned long>();
	this->loadconfig(json);
}
#if !defined ASYNC_WEBSERVER
#if defined(ESP8266)
void CBaseController::setuphandlers(ESP8266WebServer& server) {
	//ESP8266WebServer* _server = &server;
#else
void CBaseController::setuphandlers(WebServer& server) {
	//WebServer* _server = &server;
#endif
}
#endif
void CBaseController::setup() {
#if !defined(ESP8266)
	//DBG_OUTPUT_PORT.println("CBaseController::setup");
	
	if (this->get_coremode()== Core || this->get_coremode()==Both) {
	
		DBG_OUTPUT_PORT.println("CBaseController::xTaskCreatePinnedToCore");
		xTaskCreatePinnedToCore(
			runcoreloop,
			this->get_name(),
			8192,
			this,
			this->get_priority(),
			&taskhandle,
			this->get_core());
	}
#else
	if (this->get_coremode() == Core || this->get_coremode() == Both) {
		this->pTicker = new Ticker();
		this->pTicker->attach_ms<CBaseController*>(!this->interval?1:this->interval, CBaseController::callback, this);
		
	}
#endif
}
#if defined(ESP8266)
#warning "There are NO RTOS on ESP8266, please be carefull and do not call delay in run or run core methods of this service"

void CBaseController::callback(CBaseController* self) {
	self->oncallback();
}
void CBaseController::oncallback() {
	this->runcore();
	if (this->get_coremode() == Core && this->shouldRun())
		this->run();
}
#endif

bool CBaseController::onpublishmqtt(String& endkey, String& payload) {
	return false;
}
void CBaseController::onmqqtmessage(String topic, String payload) {

}
#if defined ASYNC_WEBSERVER
void CBaseController::setuphandlers(AsyncWebServer& server) {

}
#endif

#if !defined(ESP8266)
void runcoreloop(void*param)
{
	CBaseController* self = static_cast<CBaseController*>(param);

	for (;;) {


		if (self != NULL ) // &&  self->shouldRun())
			self->runcore();
		if (self->get_coremode() == Core && self->shouldRun()) {
			self->run();
			//DBG_OUTPUT_PORT.println("CBaseController::run from core");
		}
	    // https://github.com/espressif/arduino-esp32/issues/595
		vTaskDelay(10);
	}
}
#endif



/*
template<class T, typename P, typename M>
int CController<T,P,M>::AddCommand(P state, M mode, CmdSource src)
{
	command cmd = { mode,state };
	commands.Add(cmd);
	return commands.GetSize();
};
*/
