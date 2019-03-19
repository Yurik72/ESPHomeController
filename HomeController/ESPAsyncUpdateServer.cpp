#include "config.h"
#if defined(ASYNC_WEBSERVER)
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

//#include <WiFiUdp.h>
#if !defined(ESP8266)
#include <Update.h>
#endif
#include "StreamString.h"
#include "ESPAsyncUpdateServer.h"

#include <ESPAsyncWebServer.h>
//#include "esp_wps.h"
bool shouldReboot = false;
bool* pExternReboot = NULL;
ESPAsyncHTTPUpdateServer::ESPAsyncHTTPUpdateServer(bool serial_debug)
{
	_serial_output = serial_debug;
	_server = NULL;
	_username = NULL;
	_password = NULL;
	_authenticated = false;
}
void ESPAsyncHTTPUpdateServer::setExternRebootFlag(bool *pb) {
	pExternReboot = pb;
}
void ESPAsyncHTTPUpdateServer::setup(AsyncWebServer& server, const char * path, const char * username, const char * password)
{
	
	_username = (char *)username;
	_password = (char *)password;

	server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
	});
	server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
		shouldReboot = !Update.hasError();
		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
		response->addHeader("Connection", "close");
		request->send(response);
		if (shouldReboot) {
			if (pExternReboot) {
				*pExternReboot = true;
			}
			else
				ESP.restart();
		}
	}, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
		if (!index) {
			Serial.printf("Update Start: %s\n", filename.c_str());
#if defined(ESP8266)
			Update.runAsync(true);
			if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
				Update.printError(Serial);
				}
#else
			if (!Update.begin(UPDATE_SIZE_UNKNOWN))
				Update.printError(Serial);
#endif			
		}
		if (!Update.hasError()) {
			if (Update.write(data, len) != len) {
				Update.printError(Serial);
			}
		}
		if (final) {
			if (Update.end(true)) {
				Serial.printf("Update Success: %uB\n", index + len);
			}
			else {
				Update.printError(Serial);
			}
		}
	});


}
void ESPAsyncHTTPUpdateServer::_setUpdaterError() {

}
#endif