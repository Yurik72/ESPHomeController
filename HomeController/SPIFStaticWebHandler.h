#ifndef SPIFStaticWebHandler_h
#define SPIFStaticWebHandler_h

#if defined(ESP8266)
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif 
#include <ESPAsyncWebServer.h>

#ifdef ESP32
#define SPIFILE_IS_REAL(f) (f == true && !f.isDirectory())
#else
#define SPIFILE_IS_REAL(f) (f == true)
#endif

class SPIFStaticWebHandler :public AsyncWebHandler {
	using File = fs::File;
	using FS = fs::FS;
private:
	bool _getFile(AsyncWebServerRequest *request) {
		// Remove the found uri
		String path = request->url().substring(_uri.length());
		//Serial.println("_getFile");
		//Serial.println(request->url());
		// We can skip the file check and look for default if request is to the root of a directory or that request path ends with '/'
		bool canSkipFileCheck = (_isDir && path.length() == 0) || (path.length() && path[path.length() - 1] == '/');

		path = _path + path;

		// Do we have a file or .gz file
		if (!canSkipFileCheck && _fileExists(request, path))
			return true;

		// Can't handle if not default file
		if (_default_file.length() == 0)
			return false;

		// Try to add default file, ensure there is a trailing '/' ot the path.
		if (path.length() == 0 || path[path.length() - 1] != '/')
			path += "/";
		path += _default_file;

		return _fileExists(request, path);
	};
	bool _fileExists(AsyncWebServerRequest *request, const String& path) {
		bool fileFound = false;
		bool gzipFound = false;
		//Serial.println("_fileExists");
		//Serial.println(path);
		String gzip = path + ".gz";
		if (_isAlwaysGzipIfExists) {
			//Serial.println("_isAlwaysGzipIfExists");
			request->_tempFile = _fs.open(gzip, "r");
			gzipFound = SPIFILE_IS_REAL(request->_tempFile);
			if (!gzipFound) {
				//Serial.println("!gzipFound");
				request->_tempFile = _fs.open(path, "r");
				fileFound = SPIFILE_IS_REAL(request->_tempFile);
			}
		}
		else {   // prev implementation
			if (_gzipFirst) {
				request->_tempFile = _fs.open(gzip, "r");
				gzipFound = SPIFILE_IS_REAL(request->_tempFile);
				if (!gzipFound) {
					request->_tempFile = _fs.open(path, "r");
					fileFound = SPIFILE_IS_REAL(request->_tempFile);
				}
			}
			else {
				request->_tempFile = _fs.open(path, "r");
				fileFound = SPIFILE_IS_REAL(request->_tempFile);
				if (!fileFound) {
					request->_tempFile = _fs.open(gzip, "r");
					gzipFound = SPIFILE_IS_REAL(request->_tempFile);
				}
			}
		}
		bool found = fileFound || gzipFound;

		if (found) {
			// Extract the file name from the path and keep it in _tempObject
			size_t pathLen = path.length();
			char * _tempPath = (char*)malloc(pathLen + 1);
			snprintf(_tempPath, pathLen + 1, "%s", path.c_str());
			request->_tempObject = (void*)_tempPath;

			// Calculate gzip statistic
			if (!_isAlwaysGzipIfExists) {
				_gzipStats = (_gzipStats << 1) + (gzipFound ? 1 : 0);
				if (_gzipStats == 0x00) _gzipFirst = false; // All files are not gzip
				else if (_gzipStats == 0xFF) _gzipFirst = true; // All files are gzip
				else _gzipFirst = _countBits(_gzipStats) > 4; // IF we have more gzip files - try gzip first
			}
		}
		///YK

		
	   
	   


	    //Serial.println(gzipFound);
		//Serial.println(found);
		return found;
	};
	uint8_t _countBits(const uint8_t value) const 
	{
		uint8_t w = value;
		uint8_t n;
		for (n = 0; w != 0; n++) w &= w - 1;
		return n;
	};
protected:
	FS _fs;
	String _uri;
	String _path;
	String _default_file;
	String _cache_control;
	String _last_modified;
	AwsTemplateProcessor _callback;
	bool _isDir;
	bool _gzipFirst;
	uint8_t _gzipStats;
	bool _isAlwaysGzipIfExists;
public:
	SPIFStaticWebHandler(const char* uri, FS& fs, const char* path, const char* cache_control)
		: _fs(fs), _uri(uri), _path(path), _default_file("index.htm"), _cache_control(cache_control), _last_modified(""), _callback(nullptr)
		{
			// Ensure leading '/'
			if (_uri.length() == 0 || _uri[0] != '/') _uri = "/" + _uri;
			if (_path.length() == 0 || _path[0] != '/') _path = "/" + _path;

			// If path ends with '/' we assume a hint that this is a directory to improve performance.
			// However - if it does not end with '/' we, can't assume a file, path can still be a directory.
			_isDir = _path[_path.length() - 1] == '/';

			// Remove the trailing '/' so we can handle default file
			// Notice that root will be "" not "/"
			if (_uri[_uri.length() - 1] == '/') _uri = _uri.substring(0, _uri.length() - 1);
			if (_path[_path.length() - 1] == '/') _path = _path.substring(0, _path.length() - 1);

			// Reset stats
			_gzipFirst = false;
			_gzipStats = 0xF8;
			_isAlwaysGzipIfExists = false;
	};

	void setAlwaysGzipIfExists(bool b) { _isAlwaysGzipIfExists = b; };
	virtual bool canHandle(AsyncWebServerRequest *request) override final {
		if (request->method() != HTTP_GET
			|| !request->url().startsWith(_uri)
			|| !request->isExpectedRequestedConnType(RCT_DEFAULT, RCT_HTTP)
			) {
			return false;
		}
		if (_getFile(request)) {
			// We interested in "If-Modified-Since" header to check if file was modified
			if (_last_modified.length())
				request->addInterestingHeader("If-Modified-Since");

			if (_cache_control.length())
				request->addInterestingHeader("If-None-Match");

			DEBUGF("[AsyncStaticWebHandler::canHandle] TRUE\n");
			return true;
		}

		return false;
	};
	virtual void handleRequest(AsyncWebServerRequest *request) override final {
		// Get the filename from request->_tempObject and free it
		//Serial.println("handleRequest");
		//Serial.println(request->url());
		String filename = String((char*)request->_tempObject);
		free(request->_tempObject);
		request->_tempObject = NULL;
		if ((_username != "" && _password != "") && !request->authenticate(_username.c_str(), _password.c_str()))
			return request->requestAuthentication();

		if (request->_tempFile == true) {
			String etag = String(request->_tempFile.size());
			if (_last_modified.length() && _last_modified == request->header("If-Modified-Since")) {
				request->_tempFile.close();
				request->send(304); // Not modified
			}
			else if (_cache_control.length() && request->hasHeader("If-None-Match") && request->header("If-None-Match").equals(etag)) {
				request->_tempFile.close();
				AsyncWebServerResponse * response = new AsyncBasicResponse(304); // Not modified
				response->addHeader("Cache-Control", _cache_control);
				response->addHeader("ETag", etag);
				request->send(response);
			}
			else {
				AsyncWebServerResponse * response = new AsyncFileResponse(request->_tempFile, filename, String(), false, _callback);
				if (_last_modified.length())
					response->addHeader("Last-Modified", _last_modified);
				if (_cache_control.length()) {
					response->addHeader("Cache-Control", _cache_control);
					response->addHeader("ETag", etag);
				}
				request->send(response);
			}
		}
		else {
			request->send(404);
		}
	};
	SPIFStaticWebHandler& setIsDir(bool isDir) {
		_isDir = isDir;
		return *this;
	};
	SPIFStaticWebHandler& setDefaultFile(const char* filename) {
		_default_file = String(filename);
		return *this;
	};
	SPIFStaticWebHandler& setCacheControl(const char* cache_control) {
		_cache_control = String(cache_control);
		return *this;
	};
	SPIFStaticWebHandler& setLastModified(const char* last_modified) {
		_last_modified = String(last_modified);
		return *this;
	};
	SPIFStaticWebHandler& setLastModified(struct tm* last_modified) {
		char result[30];
		strftime(result, 30, "%a, %d %b %Y %H:%M:%S %Z", last_modified);
		return setLastModified((const char *)result);
	};
#ifdef ESP8266
	SPIFStaticWebHandler& setLastModified(time_t last_modified) {
		return setLastModified((struct tm *)gmtime(&last_modified));
	};
	SPIFStaticWebHandler& setLastModified() {
		time_t last_modified;
		if (time(&last_modified) == 0) //time is not yet set
			return *this;
		return setLastModified(last_modified);
	}; //sets to current time. Make sure sntp is runing and time is updated
#endif
	SPIFStaticWebHandler& setTemplateProcessor(AwsTemplateProcessor newCallback) { _callback = newCallback; return *this; }
};
#endif
