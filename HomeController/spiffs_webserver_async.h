

#if defined(ESP8266)
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif 
#include <ESPAsyncWebServer.h>


#define server asserver


#define SETUP_FILEHANDLES   server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html"); \
						server.on("/browse", handleFileBrowser); \
server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", ""); }, handleFileUpload); \
server.on("/reboot", handleReboot); \
server.on("/jsonsave", handleJsonSave); \
 server.onNotFound(onNotFoundRequest); \
server.serveStatic("/", SPIFFS, "/www/").setCacheControl("max-age=600");\
setupself();
//do something useful

File fsUploadFile;
int uploadpos = 0;
static bool* pExReboot = NULL;
const char* szPlaintext = "text/plain";
void setupself() {
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
	
}
static void setExternRebootFlag(bool *pb) {
	pExReboot = pb;
}
void handleReboot(AsyncWebServerRequest *request) {
	if (pExReboot) {
		*pExReboot = true;
		request->send(200, szPlaintext, "OK");
	}
	else
		request->send(200, szPlaintext, "NOK");
}
unsigned char h2int(char c)
{
	if (c >= '0' && c <= '9') {
		return((unsigned char)c - '0');
	}
	if (c >= 'a' && c <= 'f') {
		return((unsigned char)c - 'a' + 10);
	}
	if (c >= 'A' && c <= 'F') {
		return((unsigned char)c - 'A' + 10);
	}
	return(0);
}
String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
	char c;
	String ret = "";

	for (byte t = 0; t < input.length(); t++)
	{
		c = input[t];
		if (c == '+') c = ' ';
		if (c == '%') {


			t++;
			c = input[t];
			t++;
			c = (h2int(c) << 4) | h2int(input[t]);
		}

		ret.concat(c);
	}
	return ret;
}
void  onNotFoundRequest(AsyncWebServerRequest *request) {
	//Handle Unknown Request
	DBG_OUTPUT_PORT.println("On not found");
	DBG_OUTPUT_PORT.println(request->url());
	String path = request->url();
	
	if (path.indexOf(".") == -1) { //some body asking non existing service. can happen as well with react routing
		DBG_OUTPUT_PORT.println("redirecting to index");
		request->send(SPIFFS, "/index.html", "text/html");
		return;
	}
	
	request->send(404);
}
void handleFileList(AsyncWebServerRequest *request) {

	String path = "/";
	if (request->hasArg("dir")) {
		//server.send(500, "text/plain", "BAD ARGS");
		//return;
		path = request->arg("dir");
	}
#if defined(ESP8266)
	Dir dir = SPIFFS.openDir(path);
#else
	File root = SPIFFS.open(path);


	if (!root) {
		DBG_OUTPUT_PORT.println("- failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		DBG_OUTPUT_PORT.println(" - not a directory");
		return;
	}
#endif
	String output = "{\"success\":true, \"is_writable\" : true, \"results\" :[";
	bool firstrec = true;
#if !defined(ESP8266)
	File file = root.openNextFile();
	while (file) {
#else
	while (dir.next()) {
#endif
		if (!firstrec) { output += ','; }  //add comma between recs (not first rec)
		else {
			firstrec = false;
		}
#if !defined(ESP8266)
		//if (file.isDirectory())
		//	continue;
		String fn = file.name();
#else
		String fn = dir.fileName();
#endif
#if !defined(ESP8266)
		fn.remove(0, 1); //remove slash
		if (file.isDirectory()) {
			output += "{\"is_dir\":true";
		}
		else {
			output += "{\"is_dir\":false";
		}
#else

		fn.remove(0, 1); //remove slash
		output += "{\"is_dir\":false";
#endif
		output += ",\"name\":\"" + fn;
#if !defined(ESP8266)
		output += "\",\"size\":" + String(file.size());
#else
		output += "\",\"size\":" + String(dir.fileSize());
#endif
		output += ",\"path\":\"";
		output += "\",\"is_deleteable\":true";
		output += ",\"is_readable\":true";
		output += ",\"is_writable\":true";
		output += ",\"is_executable\":true";
		output += ",\"mtime\":1452813740";   //dummy time
		output += "}";
#if !defined(ESP8266)
		file = root.openNextFile();
#endif
	}
	output += "]}";
	//DebugPrintln("got list >"+output);
	request->send(200, "text/json", output);


	}
void handleFileDeleteByName(AsyncWebServerRequest *request,String path) {
	DBG_OUTPUT_PORT.println("handleFileDeleteByName: " + path);
	if (path == "/")
		return request->send(500, "text/plain", "BAD PATH");
	String filetodel = path;
	if (!filetodel.startsWith("/"))
		filetodel = "/" + filetodel;

	if (!SPIFFS.exists(filetodel))
		return request->send(404, szPlaintext, "FileNotFound");
	SPIFFS.remove(filetodel);
	request->send(200, szPlaintext, "");

}
void handleFileDelete(AsyncWebServerRequest *request) {
	String path;
	DBG_OUTPUT_PORT.println("handleFileDeleteByName start");
	if (request->args() == 0) return request->send(500, "text/plain", "BAD ARGS");
	path = request->arg((size_t)0).c_str();
	handleFileDeleteByName(request,path);
	
}

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	if (!index) {
		DBG_OUTPUT_PORT.println("UploadStart ->: ");

		String filenameupl = filename;
		uploadpos = 0;
		if (!filenameupl.startsWith("/")) filenameupl = "/" + filenameupl;
		String dirname = "";
		if (request->hasArg("dir")) {
			dirname = request->arg("dir");
			if (dirname.length() > 0) {
				if (!dirname.startsWith("/")) dirname = "/" + dirname;
				filenameupl = dirname + filenameupl;
			}
		}
		DBG_OUTPUT_PORT.println(filenameupl);
		fsUploadFile = SPIFFS.open(filenameupl, "w");
	}
	for (size_t i = 0; i < len; i++) {
		if (fsUploadFile)
			fsUploadFile.write(data[i]);
	}
	uploadpos += len;
	if (final) {
		if (fsUploadFile)
			fsUploadFile.close();
		DBG_OUTPUT_PORT.println("UploadEnd:");
	}
}

void handleFileBrowser(AsyncWebServerRequest *request) {
	if (request->hasArg("do") && request->arg("do") == "list") {
		handleFileList(request);
	}
	else
		if (request->hasArg("do") && request->arg("do") == "delete") {
			handleFileDeleteByName(request,request->arg("file").c_str());
		}
		else
			if (request->hasArg("do") && request->arg("do") == "download") {
				//handleFileDownload(server.arg("file").c_str());
			}
			else
			{
				request->send(SPIFFS, "/filebrowse.html", "text/html");

				//MyWebServer.isDownloading = true; //need to stop all cloud services from doing anything!  crashes on upload with mqtt...
			}
}

void handleJsonSave(AsyncWebServerRequest *request)
{

	if (request->args() == 0)
		return request->send(500, szPlaintext, F("BAD JsonSave ARGS"));

	String fname = "/" + request->arg((size_t)0);
	fname = urldecode(fname);

	DBG_OUTPUT_PORT.println("handleJsonSave: " + fname);


	File file = SPIFFS.open(fname, "w");
	if (file) {
		file.println(request->arg(1));  //save json data
		file.close();
	}
	else  //cant create file
		return request->send(500, szPlaintext, F("JSONSave FAILED"));
	request->send(200, szPlaintext, "");

}