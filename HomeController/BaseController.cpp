

#include "BaseController.h"

#include "config.h"
#if defined ASYNC_WEBSERVER
#include <ESPAsyncWebServer.h>
#endif

#if defined(ESP8266)
#include <Ticker.h>
#endif
CBaseController::CBaseController() {
	this->coreMode = NonCore;
	this->core = 0;
	this->enabled = false;
	this->_cached_next_run = 0;
	this->onstatechanged = NULL;
	this->bodybuffer = NULL;
	this->bodyindex = 0;
	this->interval = 1000;
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
	DBG_OUTPUT_PORT.println("savestate");
	DBG_OUTPUT_PORT.println(this->get_filename_state().c_str());
	savefile(this->get_filename_state().c_str(), this->serializestate());

	
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
	_cached_next_run = last_run + interval;
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
	interval= json["interval"];
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
	if (this->get_coremode()== Core || this->get_coremode()==Both) {
		unsigned char tname[50];
		
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
		this->pTicker->attach_ms<CBaseController*>(this->interval?1:this->interval, CBaseController::callback, this);
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
			if(self->get_coremode() == Core  &&  self->shouldRun())
				self->run();
		
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
