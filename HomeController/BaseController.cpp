

#include "BaseController.h"

#include "config.h"
#if defined ASYNC_WEBSERVER
#include <ESPAsyncWebServer.h>
#endif
CBaseController::CBaseController() {
	this->isCore = false;
	this->core = 0;
	this->enabled = false;
	this->_cached_next_run = 0;
	this->onstatechanged = NULL;
	this->bodybuffer = NULL;
	this->bodyindex = 0;
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
void CBaseController::loadconfig(JsonObject& json) {

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
	if (this->get_iscore()) {
		unsigned char tname[50];
		
		xTaskCreatePinnedToCore(
			runcore,
			this->get_name(),
			8192,
			this,
			this->get_priority(),
			&taskhandle,
			this->get_core());
	}
#endif
}

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
void runcore(void*param)
{
	CBaseController* self = static_cast<CBaseController*>(param);

	for (;;) {

		if (self != NULL &&  self->shouldRun())
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
