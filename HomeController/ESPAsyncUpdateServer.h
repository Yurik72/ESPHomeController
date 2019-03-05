#ifndef __HTTP_UPDATE_SERVER_ASYNC_H
#define __HTTP_UPDATE_SERVER_ASYNC_H

class AsyncWebServer;
class ESPAsyncHTTPUpdateServer
{
public:
	ESPAsyncHTTPUpdateServer(bool serial_debug = false);

	void setup(AsyncWebServer& server)
	{
		setup(server, NULL, NULL);
	}

	void setup(AsyncWebServer& server, const char * path)
	{
		setup(server, path, NULL, NULL);
	}

	void setup(AsyncWebServer& server, const char * username, const char * password)
	{
		setup(server, "/update", username, password);
	}

	void setup(AsyncWebServer& server, const char * path, const char * username, const char * password);
	void setExternRebootFlag(bool *pb);
protected:
	void _setUpdaterError();

private:
	bool _serial_output;
	AsyncWebServer *_server;
	char * _username;
	char * _password;
	bool _authenticated;
	String _updaterError;
};


#endif