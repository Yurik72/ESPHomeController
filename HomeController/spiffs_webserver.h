// ***************************************************************************
// SPIFFS Webserver
// Source: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/FSBrowser
// ***************************************************************************

/*
  FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://esp8266fs.local
  edit the page by going to http://esp8266fs.local/edit
*/


#define SETUP_FILEHANDLES   server.on("/list", HTTP_GET, handleFileList); \
  server.on("/edit", HTTP_GET, []() { \
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound"); \
  }); \
  server.on("/edit", HTTP_PUT, handleFileCreate); \
  server.on("/edit", HTTP_DELETE, handleFileDelete); \
  server.on("/edit", HTTP_POST, []() { \
    server.sendHeader("Access-Control-Allow-Origin", "*"); \
    server.send(200, "text/plain", ""); \
  }, handleFileUpload); \
  server.on("/jsonsave", handleJsonSave); \
  server.on("/jsonload", handleJsonLoad); \
  server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", ""); }, handleFileUpload); \
  server.on("/browse", handleFileBrowser);   \
  server.onNotFound([]() { \
		if (!handleFileRead(server.uri())) \
			handleNotFound(); \
		});
  
File fsUploadFile;
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

  for (byte t = 0; t<input.length(); t++)
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


String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  //char code2;
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    }
    else if (isalnum(c)) {
      encodedString += c;
    }
    else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) >9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      //code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;

}

//format bytes
String formatBytes(size_t bytes) {
	if (bytes < 1024) {
		return String(bytes) + "B";
	} else if (bytes < (1024 * 1024)) {
		return String(bytes / 1024.0) + "KB";
	} else if (bytes < (1024 * 1024 * 1024)) {
		return String(bytes / 1024.0 / 1024.0) + "MB";
	} else {
		return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
	}
}


String getContentType(String filename) {
	if (server.hasArg("download")) return "application/octet-stream";
	
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

bool handleFileRead(String path) {
	DBG_OUTPUT_PORT.println("handleFileRead: " + path);
	if (path.endsWith("/")) path += "index.html";
	if(path.indexOf(".")==-1) path += "index.html"; //some body asking non existing service. can happen as well with react routing
	String contentType = getContentType(path);
	path= urldecode(path);
	String pathWithGz = path + ".gz";

	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		File file = SPIFFS.open(path, "r");
    server.sendHeader("Access-Control-Allow-Origin", "*");
		size_t sent = server.streamFile(file, contentType);
		file.close();
		return true;
	}
	return false;
}

void handleFileUpload() {
  DBG_OUTPUT_PORT.println("file upload start");

 if (server.uri() != "/upload") return;
	HTTPUpload& upload = server.upload();
	if (upload.status == UPLOAD_FILE_START) {

		String filename = upload.filename;
	
		if (!filename.startsWith("/")) filename = "/" + filename;
		String dirname = "";
		if (server.hasArg("dir")) {
			dirname = server.arg("dir");
			if (dirname.length() > 0) {
				if (!dirname.startsWith("/")) dirname = "/" + dirname;
				filename = dirname + filename;
			}
		}
		DBG_OUTPUT_PORT.print("handleFileUpload filename: "); 
		DBG_OUTPUT_PORT.println(filename);
		fsUploadFile = SPIFFS.open(filename, "w");
		filename = String();
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		//DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
	if (fsUploadFile)
		fsUploadFile.write(upload.buf, upload.currentSize);
	} else if (upload.status == UPLOAD_FILE_END) {
		if (fsUploadFile)
			fsUploadFile.close();
		DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
	}
}
bool handleFileDownload(const char* szName=NULL)
{
  String path;
  if(szName==NULL){
    if (server.args() == 0) {
      server.send(500, "text/plain", "BAD ARGS");
      return false;
    }
    path = server.arg(0);
    
  }
  else{
    path=szName;
  }
  DBG_OUTPUT_PORT.print("handleFileDownload: " + path);
  String contentType = "application/octet-stream";
  path = "/" + path;
  path = urldecode(path);
 //check if a public file.

  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
void handleFileDeleteByName(String path) {
  DBG_OUTPUT_PORT.println("handleFileDeleteByName: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  String filetodel= path;
  if(!filetodel.startsWith("/"))
    filetodel="/"+filetodel;
  
  if (!SPIFFS.exists(filetodel))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(filetodel);
  server.send(200, "text/plain", "");
  
}
void handleFileDelete() {
  String path;
  DBG_OUTPUT_PORT.println("handleFileDeleteByName start");
 if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
 path = server.arg(0);
 handleFileDeleteByName(path);
	path = String();
}

void handleFileCreate() {
	if (server.args() == 0)
		return server.send(500, "text/plain", "BAD ARGS");
	String path = server.arg(0);
	DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
	if (path == "/")
		return server.send(500, "text/plain", "BAD PATH");
	if (SPIFFS.exists(path))
		return server.send(500, "text/plain", "FILE EXISTS");
	File file = SPIFFS.open(path, "w");
	if (file)
		file.close();
	else
		return server.send(500, "text/plain", "CREATE FAILED");
	server.send(200, "text/plain", "");
	path = String();
}

void handleFileList() {

  String path = "/";
	if (server.hasArg("dir")) {
		//server.send(500, "text/plain", "BAD ARGS");
		//return;
    path = server.arg("dir");
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
  while (file){
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
  server.send(200, "text/json", output);
	/*
	//String path = server.arg("dir");
	DBG_OUTPUT_PORT.println("handleFileList: " + path);
	Dir dir = SPIFFS.openDir(path);
	path = String();
	
	String output = "[";
	while (dir.next()) {
		File entry = dir.openFile("r");
		if (output != "[") output += ',';
		bool isDir = false;
		output += "{\"type\":\"";
		output += (isDir) ? "dir" : "file";
		output += "\",\"name\":\"";
		output += String(entry.name()).substring(1);
		output += "\"}";
		entry.close();
	}
	
	output += "]";
  server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(200, "text/json", output);
  */
}
void handleFileBrowser()
{
  
  if (server.arg("do") == "list") {
    handleFileList();
  }
  else
    if (server.arg("do") == "delete") {
      handleFileDeleteByName(server.arg("file").c_str());
    }
    else
      if (server.arg("do") == "download") {
        handleFileDownload(server.arg("file").c_str());
      }
      else
      {
        if (!handleFileRead("/filebrowse.html")) { //send GZ version of embedded browser
                                //server.sendHeader("Content-Encoding", "gzip");
                                //server.send_P(200, "text/html", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE));
                             server.send(500, "text/plain", "handleFileBrowser can't proceed");
        }
        //MyWebServer.isDownloading = true; //need to stop all cloud services from doing anything!  crashes on upload with mqtt...
      }
}
void handleJsonSave()
{

  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD JsonSave ARGS");

  String fname = "/" + server.arg(0);
  fname = urldecode(fname);

  //DBG_OUTPUT_PORT.println("handleJsonSave: " + fname);


  File file = SPIFFS.open(fname, "w");
  if (file) {
    file.println(server.arg(1));  //save json data
    file.close();
  }
  else  //cant create file
    return server.send(500, "text/plain", "JSONSave FAILED");
  server.send(200, "text/plain", "");

}

void handleJsonLoad()
{
  
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD JsonLoad ARGS");
  String fname = "/" + server.arg(0);

  fname = urldecode(fname);
  //DBG_OUTPUT_PORT.println("handleJsonRead: " + fname);



  File file = SPIFFS.open(fname, "r");
  if (file) {
    server.streamFile(file, "application/json");
    file.close();
  }
}
void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}