#include "config.h"
#if !defined(ESP8266) &&  !defined(ASYNC_WEBSERVER)
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WebServer.h>
//#include <WiFiUdp.h>
#include <Update.h>
#include "StreamString.h"
#include "ESP32HTTPUpdateServer.h"
#include "esp_wps.h"

static const char serverIndex[] PROGMEM =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update'>"
"<input type='submit' value='Update'>"
"</form>"
"<div id='prg'>progress: 0%</div>"
"<script>"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
" $.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!')"
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>";
bool* pExternReboot = NULL;
ESP32HTTPUpdateServer::ESP32HTTPUpdateServer(bool serial_debug)
{
  _serial_output = serial_debug;
  _server = NULL;
  _username = NULL;
  _password = NULL;
  _authenticated = false;
}
void ESP32HTTPUpdateServer::setExternRebootFlag(bool *pb) {
	pExternReboot = pb;
}
void ESP32HTTPUpdateServer::setup(WebServer *server, const char * path, const char * username, const char * password)
{
    _server = server;
    _username = (char *)username;
    _password = (char *)password;

    // handler for the /update form page
    _server->on(path, HTTP_GET, [=](){
      if(_username != NULL && _password != NULL && !_server->authenticate(_username, _password))
        return _server->requestAuthentication();
      _server->send_P(200, PSTR("text/html"), serverIndex);
    });
	_server->on(path, HTTP_POST, [=]() {
		_server->sendHeader("Connection", "close");
		_server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
		//esp_wifi_wps_disable();
		delay(100);
		_server->client().stop();
		Serial.println("Start rebooting"); 
		ESP.restart();
	}, [=]() {
		HTTPUpload& upload = _server->upload();
		if (upload.status == UPLOAD_FILE_START) {
			Serial.printf("Update: %s\n", upload.filename.c_str());
			if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {//start with max available size
				Update.printError(Serial);
			}
		}
		else if (upload.status == UPLOAD_FILE_WRITE) {
			/* flashing firmware to ESP*/
			if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
				Update.printError(Serial);
			}
		}
		else if (upload.status == UPLOAD_FILE_END) {
			if (Update.end(true)) { //true to set the size to the current progress
				Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
				
			}
			else {
				Update.printError(Serial);
			}
		}
	});
    
}

void ESP32HTTPUpdateServer::_setUpdaterError()
{
  
}
#endif