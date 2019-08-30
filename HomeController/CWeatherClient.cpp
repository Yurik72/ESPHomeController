#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "CWeatherClient.h"
#include "HTTPSimpleClient.h"

const size_t bufferSize = JSON_OBJECT_SIZE(20);
#ifndef DISABLE_WEATHER
REGISTER_CONTROLLER_FACTORY(WeatherClientController)
#endif

WeatherClientController::WeatherClientController(){
  
 }

String  WeatherClientController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;


	String json;

	serializeJson(root, json);

	return json;
}
bool  WeatherClientController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	WeatherState newState;
	newState.isOn = root[FPSTR(szisOnText)];



	//this->set_state(newState);
	return true;

}
void WeatherClientController::loadconfig(JsonObject& json) {
	Weather::loadconfig(json);
	uri = json["uri"].as<String>();
	
}
void WeatherClientController::getdefaultconfig(JsonObject& json) {

	json[FPSTR(szservice)] = "WeatherClientController";
	json[FPSTR(szname)] = "Weather";

	Weather::getdefaultconfig(json);
}
void  WeatherClientController::setup() {
	
}

void WeatherClientController::run() {

	if (this->commands.GetSize() == 0) {
		command newcmd;
		newcmd.mode = WRefresh;
		this->read_data();


		//this->commands.Add(newcmd);
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
	command cmd;

	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		WeatherState newState = cmd.state;

		this->set_state(newState);
	}
	Weather::run();
}
void WeatherClientController::set_state(WeatherState state) {


	Weather::set_state(state);

	//	digitalWrite(pin, (state.isOn ^ this->isinvert) ? HIGH : LOW);

}


 bool WeatherClientController::read_data(){
  HTTPSimpleClient http;
 
    http.begin(uri); //Specify the URL
    int httpCode = http.GET();                                        //Make the request
    data.RemoveAll();
    String payload;
    if (httpCode > 0) { //Check for the returning code
 
        payload = http.getString();
        Serial.println(httpCode);
        
      }
 
    else {
      Serial.println("Error on HTTP request");
      return false;
    }
 
    http.end(); //Free the resources
    int capacity = 1024*3;
    DynamicJsonDocument jsonBuffer(capacity);
Serial.println("Start parsing");
   DeserializationError error = deserializeJson(jsonBuffer, payload);
   if (!error) {
  
      JsonObject root = jsonBuffer.as<JsonObject>();
      JsonArray jsonardaypart=root["daypart"].as<JsonArray>();
      
      JsonArray arrdaypartname=jsonardaypart[0]["daypartName"].as<JsonArray>();
      JsonArray arricon=jsonardaypart[0]["iconCode"].as<JsonArray>();
      JsonArray arrprecipchanse=jsonardaypart[0]["precipChance"].as<JsonArray>();
      JsonArray arrpreciptype=jsonardaypart[0]["precipType"].as<JsonArray>();
      JsonArray arrtemp=jsonardaypart[0]["temperature"].as<JsonArray>();
      JsonArray arrphraseshort=jsonardaypart[0]["wxPhraseShort"].as<JsonArray>();
      
 
      for (int i = 0; i < arrdaypartname.size(); i++){
   
        ForeceastRecord rec;
        String daypartName=arrdaypartname[i].as<String>();
		if (daypartName != "null") {
			if (daypartName.length() >= REC_CHAR_MAX) {
				if (daypartName.endsWith("night")) {
					daypartName = daypartName.substring(0, 3);
					daypartName += " ";
					daypartName += "night";
				}
				else {
					daypartName = daypartName.substring(0, REC_CHAR_MAX - 1);
				}
			}
			strncpy(rec.text, daypartName.c_str(), REC_CHAR_MAX);

			rec.temperature = arrtemp[i].as<uint8_t>();
			rec.precipChance = arrprecipchanse[i].as<uint8_t>();
			memset(rec.icon, 0, REC_CHAR_MAX);

			strncpy(rec.icon, arricon[i].as<String>().c_str(), REC_CHAR_MAX);
			memset(rec.phraseshort, 0, REC_CHAR_MAX);

			strncpy(rec.phraseshort, arrphraseshort[i].as<String>().c_str(), REC_CHAR_MAX);
			data.Add(rec);
		}
      }
    }
    else{
	   DBG_OUTPUT_PORT.println(F("Wether Erorr parse json"));
    }

 }
 
