#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif
#include "config.h"
#include "BaseController.h"
#include "Triggers.h"

#include "TimeController.h"
#include "RGBStripController.h"
#include "LDRController.h"

#include "Utilities.h"
#include "RelayController.h"
#include "RFController.h"

#include "Controllers.h"


#ifndef DISABLE_RGB
REGISTER_TRIGGER_FACTORY(TimeToRGBStripTrigger)
#endif
//REGISTER_TRIGGER_FACTORY(TimeToRelayTrigger)
#ifndef ESP8266 
REGISTER_TRIGGER_FACTORY(LDRToRelay)
REGISTER_TRIGGER_FACTORY(BMEToOled)
REGISTER_TRIGGER_FACTORY(BMEToRGBMatrix)
REGISTER_TRIGGER_FACTORY(BMEToThingSpeak)
REGISTER_TRIGGER_FACTORY(LDRToThingSpeak)

#ifndef DISABLE_WEATHERDISPLAY
REGISTER_TRIGGER_FACTORY(BMEToWeatherDisplay)
REGISTER_TRIGGER_FACTORY(TimeToWeatherDisplay)
REGISTER_TRIGGER_FACTORY(WeatherForecastToWeatherDisplay)
REGISTER_TRIGGER_FACTORY(ButtonToWeatherDisplay)
#endif
#endif

#ifndef DISABLE_RGB
REGISTER_TRIGGER_FACTORY(LDRToRGBStrip)
#endif
#ifndef DISABLE_RELAY
REGISTER_TRIGGER_FACTORY(RFToRelay)
REGISTER_TRIGGER_FACTORY(RFToMotion)

REGISTER_TRIGGER_FACTORY(TimeToRelayTrigger)
REGISTER_TRIGGER_FACTORY(ButtonToRelay)
#endif
#ifndef DISABLE_RELAYDIM
REGISTER_TRIGGER_FACTORY(TimeToRelayDimTrigger)
#endif

#ifndef DISABLE_DALLAS
REGISTER_TRIGGER_FACTORY(DallasToRGBStrip)
#endif 
void Triggers::setup() {
	this->loadconfig();
	
	//for (int i = 0; i < this->GetSize(); i++)
	//	this->GetAt(i)->setup();

}
void Triggers::loadconfig() {
	String filename = F("/triggers.json");
	int capacity = JSON_ARRAY_SIZE(5) + 2 * JSON_OBJECT_SIZE(40) + 262;
	if (SPIFFS.exists(filename)) {
		//file exists, reading and loading
		//DBG_OUTPUT_PORT.println("Read triggers configuration: ");
		File configFile = SPIFFS.open(filename, "r");
		if (configFile) {
			DynamicJsonDocument jsonBuffer(capacity);

			DeserializationError error = deserializeJson(jsonBuffer, configFile.readString());
			if (!error) {

				JsonArray arr = jsonBuffer.as<JsonArray>();
				for (int i = 0; i < arr.size(); i++) {
					JsonObject json = arr[i];
					String type = json["type"].as<String>();
					Trigger* trigger = Triggers::CreateByType(type.c_str());
					
					trigger->loadconfig(json);
					this->Add(trigger);
					DBG_OUTPUT_PORT.print("parsed trigger ->");
					DBG_OUTPUT_PORT.println(type);
				}
			}
			else {
				DBG_OUTPUT_PORT.println("Deserialize error");
			}
		}
	}
	else {
		DBG_OUTPUT_PORT.println("File not found triggers.json");
	}
	DBG_OUTPUT_PORT.print("Added triggers");
	DBG_OUTPUT_PORT.println(this->GetSize());

}

void Trigger::handleloop(CBaseController*pBase, Controllers* pctls) {
	//DBG_OUTPUT_PORT.print("triggers handle loop");
	//DBG_OUTPUT_PORT.println("Base Trigger handle loop");
	//delay(1000);
}

void Trigger::loadconfig(JsonObject& json) {
	src = json["source"].as<String>();
	dst = json["destination"].as<String>();
	

}
Trigger* Triggers::CreateByType(const char* nametype) {
//	DBG_OUTPUT_PORT.println(F("Triggers::CreateByType"));
//	DBG_OUTPUT_PORT.println(nametype);


	Trigger* pTrigger = Factories::CreateTrigger(nametype);
	if (pTrigger == NULL) {
		//DBG_OUTPUT_PORT.println(F("Trigger not created, trying with Trigger at endd "));
		pTrigger = Factories::CreateTrigger(String(nametype) + String("Trigger"));
		if (pTrigger)
			return pTrigger;
	}
	else {
		return pTrigger;
	}

	DBG_OUTPUT_PORT.println(F("Creating trigger,something wrong !!"));
	return new Trigger();
	






	/*
	
	Trigger* res = NULL;
	if (strcmp(nametype, "TimeToRGBStrip") == 0) {
		res= new TimeToRGBStripTrigger();
	}
	else if (strcmp(nametype, "LDRToRGBStrip") == 0) {
		res = new LDRToRGBStrip();
	}
	else if (strcmp(nametype, "TimeToRelay") == 0) {
		res = new TimeToRelayTrigger();
	}
	else if (strcmp(nametype, "RFToRelay") == 0) {
		res = new RFToRelay();
	}
	else {
		res = new Trigger();
	}
	//res->type = nametype;
	return res;
	*/
}
//service base;
template<class SRC, class DST>
TriggerFromService<SRC, DST>::TriggerFromService() {
	this->pDst = NULL;
	this->pSrc = NULL;
}
template<class SRC, class DST>
void TriggerFromService<SRC, DST>::handleloop(CBaseController* pBase, Controllers* pctls) {
//#ifdef	TRIGGER_DEBUG
//	DBG_OUTPUT_PORT.println("TriggerFromService handleloop");
//#endif
	if (!this->get_dstctl()) {
		CBaseController* pBaseDst = pctls->GetByName(this->dst.c_str());
		if (pBaseDst == NULL) {
			//DBG_OUTPUT_PORT.println("Destination service not found");
			return;
		}

		DST *pdst = static_cast<DST*>(pBaseDst);
		this->set_dstctl(pdst);
	}
	if (this->get_srcctl() == NULL) {

#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Setup Sourse ctl");
#endif

		SRC* psrc = static_cast<SRC*>(pBase);
		this->set_srcctl(psrc);

	}
	this->handleloopsvc(this->get_srcctl(), this->get_dstctl());
}
template<class SRC, class DST>
void TriggerFromService<SRC, DST>::handleloopsvc(SRC* ps, DST* pd) {

}
///Time Base
template<typename TM>
CBaseTimeTrigger<TM>::CBaseTimeTrigger() {
	this->pTime=NULL;
	this->timeoffs = 0;
#ifdef	TRIGGER_DEBUG
	DBG_OUTPUT_PORT.print("CBaseTimeTrigger()");

#endif
}
template<typename TM>
TimeController* CBaseTimeTrigger<TM>::get_timectl() { 
	return pTime;
};
template<typename TM>
void CBaseTimeTrigger<TM>::set_timectl(TimeController *pCtl) {
	pTime = pCtl; 
};
template<typename TM>
void CBaseTimeTrigger<TM>::parsetime(JsonObject& json, TM & rec) {
	String stime = json["time"].as<String>();
	stime.trim();
	DBG_OUTPUT_PORT.println(stime);
	cron_parse_expr(stime.c_str(), &rec.cronexpr, &rec.cron_err);
	if (rec.iscron()) {
		DBG_OUTPUT_PORT.println("Cron parsed ");
	}
	else if (rec.timetype == dailly) {
		DBG_OUTPUT_PORT.println("Cron NOT parsed ");
		DBG_OUTPUT_PORT.println(rec.cron_err);
		stime.replace(":", "");
		rec.time = stime.toInt();
	}
#ifdef	TRIGGER_DEBUG
	DBG_OUTPUT_PORT.print("Parsing time ");
	DBG_OUTPUT_PORT.println(rec.time);
#endif
}
template<typename TM>
void CBaseTimeTrigger<TM>::processrecord(time_t currentTime, TM& rec, Controllers* pctlss) {
	
#ifdef	TRIGGER_DEBUG
	DBG_OUTPUT_PORT.print("Trigger process record ");
	DBG_OUTPUT_PORT.println(getFormattedTime(currentTime));
	DBG_OUTPUT_PORT.print("timeToTrigger ");
	DBG_OUTPUT_PORT.println(getFormattedTime(rec.timeToTrigger));
	DBG_OUTPUT_PORT.print("lastTriggered ");
	DBG_OUTPUT_PORT.println(getFormattedTime(rec.lastTriggered));
	
#endif
	if (rec.iscron()) {
		bool dotrigger = false;
		currentTime -= this->get_timeoffs();
		if (rec.lastTriggered == 0) {
			rec.lastTriggered = currentTime;// -100 - this->get_timeoffs();
			//dotrigger = true;
		}
		//DBG_OUTPUT_PORT.println("CBaseTimeTrigger free heap");
		//DBG_OUTPUT_PORT.println(esp_get_free_heap_size());
		rec.timeToTrigger = cron_next(&rec.cronexpr, rec.lastTriggered+ this->get_timeoffs());
		//DBG_OUTPUT_PORT.println(esp_get_free_heap_size());
		//DBG_OUTPUT_PORT.println("Cron next run");
		//DBG_OUTPUT_PORT.println(getFormattedDateTime(rec.lastTriggered));
		//DBG_OUTPUT_PORT.println(getFormattedDateTime(rec.timeToTrigger));
		//DBG_OUTPUT_PORT.println(getFormattedDateTime(currentTime));
		//DBG_OUTPUT_PORT.println(esp_get_free_heap_size());
		if (rec.timeToTrigger < currentTime || dotrigger) {
			
			//DBG_OUTPUT_PORT.println("Do trigger");
			this->dotrigger(rec, pctlss);
			rec.lastTriggered = currentTime;
		}
	}
	else if (rec.timetype == dailly) {

		if (rec.timeToTrigger == 0) { //not triggered yet
			//to be check timezones ESP32/ESP8266
#if defined(ESP8266)
			rec.timeToTrigger = apply_hours_minutes_fromhhmm(currentTime, rec.time,0);
#else
			rec.timeToTrigger = apply_hours_minutes_fromhhmm(currentTime, rec.time, this->get_timeoffs());
#endif
#ifdef	TRIGGER_DEBUG
			DBG_OUTPUT_PORT.print("init trigger time ");
			DBG_OUTPUT_PORT.println(getFormattedTime(rec.timeToTrigger));
			
#endif
			if (rec.timeToTrigger < currentTime) { //in the past , need next day
				rec.timeToTrigger += NEXT_DAY_SEC;
#ifdef	TRIGGER_DEBUG
				DBG_OUTPUT_PORT.print("switch to next date");
				DBG_OUTPUT_PORT.println(getFormattedTime(rec.timeToTrigger));
#if !defined(ESP8266)
				struct tm *tminfo;
				tminfo = localtime(&rec.timeToTrigger);
				DBG_OUTPUT_PORT.println(asctime(tminfo));
#endif
#endif
			}
		}
		if (istimetotrigger(rec.timeToTrigger, currentTime)) {
#ifdef	TRIGGER_DEBUG
			DBG_OUTPUT_PORT.println("istimetotrigger");
#endif
			//if (!istimetotrigger(rec.lastTriggered, currentTime)) { //already triggered
			if( abs(rec.lastTriggered - rec.timeToTrigger) < SEC_TOLLERANCE){
#ifdef	TRIGGER_DEBUG
				DBG_OUTPUT_PORT.println("Switching to the next day");
#endif

				//set next time to trigger
				time_t nextday = currentTime + NEXT_DAY_SEC;
#if defined(ESP8266)
				rec.timeToTrigger = apply_hours_minutes_fromhhmm(nextday, rec.time, 0);
#else
				rec.timeToTrigger = apply_hours_minutes_fromhhmm(nextday, rec.time, this->get_timeoffs());
#endif
				
			}
			else {
				this->dotrigger(rec, pctlss);
				rec.lastTriggered = currentTime;
			}
		}
	}
}
template<typename TM>
bool CBaseTimeTrigger<TM>::istimetotrigger(time_t time, time_t currentTime) {
	return  (time < currentTime) && (abs(time - currentTime) < SEC_TOLLERANCE);  //2min
}
template<typename TM>
void CBaseTimeTrigger<TM>::handleloop(CBaseController*pBase, Controllers* pctlss) {

#ifdef	TRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("Trigger handle loop");

	DBG_OUTPUT_PORT.print(this->src);
	DBG_OUTPUT_PORT.println(this->dst);
		
#endif
	
	if (this->get_timectl()==NULL) {

#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Setup time ctl");
#endif
		TimeController*pTime = static_cast<TimeController*>(pBase);
		this->set_timectl(pTime);
		this->set_timeoffs(pTime->get_gmtoffset());
#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println(pTime->get_gmtoffset());
#endif
	}
	time_t currentTime = get_timectl()->get_state().time;
	for (int i = 0;i < this->times.GetSize();i++)
		this->processrecord(currentTime, this->times.GetAt(i), pctlss);

}

/////time to rgb
TimeToRGBStripTrigger::TimeToRGBStripTrigger() {
	
	this->pStrip = NULL;
}
void TimeToRGBStripTrigger::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		timerecRGB & rec = *(new timerecRGB());
		JsonObject json = arr[i];
		this->parsetime(json,rec);
		rec.isOn = arr[i][FPSTR(szisOnText)].as<bool>();
		rec.color= arr[i]["color"].as<int>();
		rec.brightness = arr[i]["bg"].as<int>();
		rec.wxmode = arr[i]["wxmode"].as<int>();
		rec.isLdr = arr[i]["isLdr"].as<int>();
		rec.fadetm= arr[i]["fadetm"].as<int>();
		times.Add(rec);
	}
}
TimeToRelayTrigger::TimeToRelayTrigger() {

	this->pRelay = NULL;
}
void TimeToRelayTrigger::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		timerecRGB & rec = *(new timerecRGB());
		JsonObject json = arr[i];
		this->parsetime(json, rec);
		rec.isOn = arr[i][FPSTR(szisOnText)].as<bool>();


		times.Add(rec);
}
}
void TimeToRelayTrigger::dotrigger(timerecOn & rec, Controllers* pctlss) {
#ifdef	TRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("TimeToRelayTrigger::dotrigger");
#endif
	if (!this->get_relayctl()) {
		CBaseController* pBase = pctlss->GetByName(this->dst.c_str());
		if (pBase == NULL) {
			//DBG_OUTPUT_PORT.println("Destination service not found");
			return;
	}

		RelayController *pR = static_cast<RelayController*>(pBase);
		this->set_relayctl(pR);
}
	RelayState newstate = this->get_relayctl()->get_state();

	if (rec.isOn) {
#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Mode On");
#endif
		newstate.isOn = true;
		this->get_relayctl()->AddCommand(newstate, RelayOn, srcTrigger);
		
	}
	else {
#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Mode Off");
#endif
		this->get_relayctl()->AddCommand(newstate, RelayOff, srcTrigger);
	}

}



void TimeToRGBStripTrigger::dotrigger(timerecRGB & rec, Controllers* pctlss) {
//#ifdef	TRIGGER_DEBUG
	//DBG_OUTPUT_PORT.println("TimeToRGBStripTrigger::dotrigger");
//#endif
	DBG_OUTPUT_PORT.println("TimeToRGBStripTrigger::dotrigger");
	if (!this->get_stripctl()) {
		CBaseController* pBase = pctlss->GetByName(this->dst.c_str());
		if (pBase == NULL) {
			//DBG_OUTPUT_PORT.println("Destination service not found");
			return;
		}

		RGBStripController *pStrip = static_cast<RGBStripController*>(pBase);
		this->set_stripctl(pStrip);
	}
	RGBState newstate = this->get_stripctl()->get_state();

	if (rec.isOn) {
#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Mode On");
#endif
		//DBG_OUTPUT_PORT.println("Sending command On");
		
		newstate.isOn = true;
		newstate.isLdr = rec.isLdr;
		newstate.brightness = rec.brightness;
		newstate.color = rec.color;
		newstate.fadetm = rec.fadetm;
		this->get_stripctl()->AddCommand(newstate, SetRGB, srcTrigger);
		//DBG_OUTPUT_PORT.println("Add command");
		//DBG_OUTPUT_PORT.println(this->get_stripctl()->get_commands_size());
		//newstate.isLdr = rec.isLdr;
		//this->get_stripctl()->AddCommand(newstate, SetLdrVal, srcTrigger);
		//newstate.brightness = rec.brightness;
		//this->get_stripctl()->AddCommand(newstate, SetBrigthness, srcTrigger);
		//newstate.color = rec.color;
		//this->get_stripctl()->AddCommand(newstate, SetColor, srcTrigger);
	}
	else{
#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Mode Off");
#endif
		
		newstate.fadetm = rec.fadetm;
		
		this->get_stripctl()->AddCommand(newstate, Off, srcTrigger);
		//DBG_OUTPUT_PORT.println("Add command");
		//DBG_OUTPUT_PORT.println(this->get_stripctl()->get_commands_size());
	}

}
///ldr -> RGB brigthness
LDRToRGBStrip::LDRToRGBStrip() {

}

void LDRToRGBStrip::handleloopsvc(LDRController* ps, RGBStripController* pd) {
	TriggerFromService< LDRController, RGBStripController>::handleloopsvc(ps, pd);
	LDRState l = ps->get_state();
	RGBState rState;
	rState.ldrValue = l.ldrValue;
	RGBCMD cmd = SetLdrVal;
#ifdef	TRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("LDRToRGBStrip set ");
	DBG_OUTPUT_PORT.println(rState.ldrValue);
#endif	TRIGGER_DEBUG
	if(rState.isLdr && rState.isOn)
		pd->AddCommand(rState, cmd, srcTrigger);
}

//LDR to Realy
LDRToRelay::LDRToRelay() {

}

void LDRToRelay::handleloopsvc(LDRController* ps, RelayController* pd) {
	LDRState l = ps->get_state();

	RelayState prevState=pd->get_state();
	RelayState newState;
	
	RelayCMD cmd = Set;
	if (l.ldrValue > valueOff) { //OFF
		newState.isOn = false;
	}
	else if (l.ldrValue < valueOn) { //ON
		newState.isOn = false;
	}
	if(newState.isOn!=prevState.isOn)
		pd->AddCommand(newState, cmd, srcTrigger);
}
void LDRToRelay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonObject js = json["value"].as<JsonObject>();
	this->valueOff = js["valueOff"];
	this->valueOn = js["valueOn"];
}

RFToRelay::RFToRelay() {
	this->last_tick = 0;
	this->last_token = 0;
	this->delaywait = 300;
}
void RFToRelay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		RFRecord & rec = *(new RFRecord());
		JsonObject json = arr[i];
		
		rec.isOn = arr[i][FPSTR(szisOnText)].as<bool>();
		rec.isswitch = arr[i]["isSwitch"].as<bool>();
		rec.token = arr[i]["token"].as<long>();
		rec.len = arr[i]["len"];;
		rec.protocol = arr[i]["protocol"];
		rec.pulse = arr[i]["pulse"];
		rfs.Add(rec);
	}
}
void RFToRelay::handleloopsvc(RFController* ps, RelayController* pd) {


	RFState rfstate = ps->get_state();
	if (this->last_tick == rfstate.timetick) { ///simple ignore, not new data

		return;
	}

	if (this->delaywait > 0 && this->last_token == rfstate.rftoken && (this->last_tick + this->delaywait) > millis()) {
		//TO DO implement continues pressing
		this->last_tick = rfstate.timetick;
		return;    //ignoring duplicate
	}
#ifdef	RF_TRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("RFToRelay continue with new token");
#endif 
	this->last_tick = rfstate.timetick;
	this->last_token = rfstate.rftoken;
//	return;
	for (int i = 0;i < this->rfs.GetSize();i++) {
		RFRecord& rec = this->rfs.GetAt(i);
		if (rec.token == rfstate.rftoken)
			this->processrecord(rec, pd);
	}



}
void RFToRelay::processrecord(RFRecord& rec, RelayController* pr) {
	RelayState newState=pr->get_state();
#ifdef	RF_TRIGGER_DEBUG
	DBG_OUTPUT_PORT.print ("RFToRelay::processrecord with key");
	DBG_OUTPUT_PORT.println(rec.token);
#endif 
	RelayCMD cmd = Set;
	if (rec.isswitch) {
		cmd = Set;
		newState.isOn = !newState.isOn;
	}
	else {
		newState.isOn = rec.isOn;
	}
	pr->AddCommand(newState, cmd, srcTrigger);
}


/// time to relay dim
TimeToRelayDimTrigger::TimeToRelayDimTrigger() {

	this->pRelayDim = NULL;
}
void TimeToRelayDimTrigger::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		timerecRelayDim & rec = *(new timerecRelayDim());
		JsonObject json = arr[i];
		this->parsetime(json, rec);
		rec.isOn = arr[i][FPSTR(szisOnText)].as<bool>();
		rec.isLdr = arr[i]["isLdr"].as<int>();
		rec.brightness = arr[i]["bg"].as<int>();
		times.Add(rec);
	}
}

void TimeToRelayDimTrigger::dotrigger(timerecRelayDim & rec, Controllers* pctlss) {
	//#ifdef	TRIGGER_DEBUG
		//DBG_OUTPUT_PORT.println("TimeToRGBStripTrigger::dotrigger");
	//#endif
	DBG_OUTPUT_PORT.println("TimeToRelayDimTrigger::dotrigger");
	if (!this->get_relayctl()) {
		CBaseController* pBase = pctlss->GetByName(this->dst.c_str());
		if (pBase == NULL) {
			//DBG_OUTPUT_PORT.println("Destination service not found");
			return;
		}

		RelayDimController *pRelaydim = static_cast<RelayDimController*>(pBase);
		this->set_relayctl(pRelaydim);
	}
	RelayDimState newstate = this->get_relayctl()->get_state();

	if (rec.isOn) {
#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Mode On");
#endif
		//DBG_OUTPUT_PORT.println("Sending command On");

		newstate.isOn = true;

		newstate.brightness = rec.brightness;
		newstate.isLdr = rec.isLdr;

		this->get_relayctl()->AddCommand(newstate, DimSet, srcTrigger);

	}
	else {
#ifdef	TRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("Mode Off");
#endif
		this->get_relayctl()->AddCommand(newstate, DimRelayOff, srcTrigger);
	}

}

DallasToRGBStrip::DallasToRGBStrip() {
	temp_min = 18;
	temp_max = 30;
}
uint32_t tempranges[3][10] = { // color, speed, mode, duration (seconds)  // to do load from json config
	  { 0, 0, 0, 0, 0, 40, 80, 120, 200, 255}, ///red
	  { 0  , 20, 100, 140, 255, 140, 100, 20, 0, 0}, // green
	  { 255, 200, 120, 80, 40, 0, 0, 0, 0, 0} // blue
};
//float temp_min=18;
//float temp_max = 30;
void DallasToRGBStrip::handleloopsvc(DallasController* ps, RGBStripController* pd) {
	TriggerFromService< DallasController, RGBStripController>::handleloopsvc(ps, pd);
	DallasState l = ps->get_state();
	RGBState rState;
	uint32_t color = calcColor(l.temp);
	rState.color = color;
	RGBCMD cmd = SetColor;
#ifdef	DALLSATRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("LDRToRGBStrip set ");
	DBG_OUTPUT_PORT.println(rState.color);
	DBG_OUTPUT_PORT.println(REDVALUE(rState.color));  
	DBG_OUTPUT_PORT.println(GREENVALUE(rState.color));
	DBG_OUTPUT_PORT.println(BLUEVALUE(rState.color));
#endif	//DALLSATRIGGER_DEBUG
	pd->AddCommand(rState, cmd, srcTrigger);
}
uint32_t DallasToRGBStrip::calcColor(float temp) {
	return calcTempColorSimple(temp, temp_min, temp_max);
	uint32_t res = 0;
	int idx = map((uint32_t)temp, (uint32_t)temp_min, (uint32_t)temp_max, 0, 9);
	if (idx == 0) {
		res = Color(tempranges[0][0], tempranges[0][0], tempranges[0][0]);
	}
	else if (idx == 9) {
		res = Color(tempranges[0][9], tempranges[0][9], tempranges[0][9]);
	}
	else {
	
		int idxto = idx+1;
		float degreeperrange = (temp_max - temp_min) / 10.0;
		float rangemin = temp_min+degreeperrange * idx;
		float rangemax = temp_min+degreeperrange * idxto;
		uint32_t red = map((uint32_t)(temp * 100), (uint32_t)(rangemin * 100), (uint32_t)(rangemax * 100), tempranges[0][idx], tempranges[0][idxto]);
		uint32_t green = map((uint32_t)(temp * 100), (uint32_t)(rangemin * 100), (uint32_t)(rangemax * 100), tempranges[1][idx], tempranges[1][idxto]);
		uint32_t blue = map((uint32_t)(temp * 100), (uint32_t)(rangemin * 100), (uint32_t)(rangemax * 100), tempranges[2][idx], tempranges[2][idxto]);
#ifdef	DALLSATRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("idx ");
		DBG_OUTPUT_PORT.println(idx);
		DBG_OUTPUT_PORT.println("idxto ");
		DBG_OUTPUT_PORT.println(idxto);
		DBG_OUTPUT_PORT.println("degreeperrange");
		DBG_OUTPUT_PORT.println(degreeperrange);
		DBG_OUTPUT_PORT.println("range min ");
		DBG_OUTPUT_PORT.println(rangemin);
		DBG_OUTPUT_PORT.println("range max ");
		DBG_OUTPUT_PORT.println(rangemax);
		DBG_OUTPUT_PORT.println("green ");
		DBG_OUTPUT_PORT.println(green);


#endif
		res = Color(red, green, blue);
	}
	
	return res;
}

void DallasToRGBStrip::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	if (arr.size() == 2) {
		this->temp_min = arr[0].as<float>();
		this->temp_max = arr[1].as<float>();
	}
	//DBG_OUTPUT_PORT.println("Dallas tr min ");
	//DBG_OUTPUT_PORT.println(this->temp_min);
	//DBG_OUTPUT_PORT.println("Dallas tr max ");
	//DBG_OUTPUT_PORT.println(this->temp_max);
}

BMEToOled::BMEToOled() {

}
void BMEToOled::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	

}
void BMEToOled::handleloopsvc(BME280Controller* ps, OledController* pd) {
	TriggerFromService< BME280Controller, OledController>::handleloopsvc(ps, pd);
	BMEState l = ps->get_state();
	OledState dispState;
	
	OLEDCMD cmd = OledClear;
	pd->AddCommand(dispState, cmd, srcTrigger);
	cmd = OledDrawText;
	switch (this->mode)
	{
	case all:
		
		dispState.fontsize = 1;
		dispState.text = format_doublestr("Temp %.2f C", l.temp);
		dispState.x = 3;
		dispState.y = 0;
#ifdef	BMETRIGGER_DEBUG
		DBG_OUTPUT_PORT.println("BMEToOled set ");
		DBG_OUTPUT_PORT.println(dispState.text);
#endif	//DALLSATRIGGER_DEBUG
		pd->AddCommand(dispState, cmd, srcTrigger);
		dispState.text = format_doublestr("Hum %.2f %", l.hum);
		dispState.x = 3;
		dispState.y = 10;
		pd->AddCommand(dispState, cmd, srcTrigger);
		dispState.text = format_doublestr("Pres %.2f hPa", l.pres);
		dispState.x = 3;
		dispState.y = 20;
		pd->AddCommand(dispState, cmd, srcTrigger);
		break;
	case temp:
		dispState.fontsize = 3;
		dispState.text = format_doublestr("T %.2f C", l.temp);
		dispState.x = 3;
		dispState.y = 0;
		pd->AddCommand(dispState, cmd, srcTrigger);
		break;
	case hum:
		dispState.fontsize = 3;
		dispState.text = format_doublestr("H %.2f ", l.hum);
		dispState.x = 3;
		dispState.y = 0;
		pd->AddCommand(dispState, cmd, srcTrigger);
		break;
	case pres:
		dispState.fontsize = 3;
		dispState.text = format_doublestr("P %.2f ", l.pres);
		dispState.x = 3;
		dispState.y = 0;
		pd->AddCommand(dispState, cmd, srcTrigger);
		
		break;
	default:
		break;

	}
	if (this->mode == pres) {
		this->mode = all;
	}
	else {
		this->mode=(DMODE)(this->mode+1);
	}
}

String BMEToOled::format_doublestr(const char* fmt, double val) {

	String res;
	char buff[50];
	snprintf(buff, sizeof(buff), fmt, val);
	res = buff;
	return res;
}

BMEToRGBMatrix::BMEToRGBMatrix() {

}
void BMEToRGBMatrix::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();


}
String BMEToRGBMatrix::get_temp_text(double val) {
	return format_str("%.1fC", val);
}
String BMEToRGBMatrix::get_humidity_text(double val) {
	return  format_str("%.0f %%", val);
}
String BMEToRGBMatrix::get_pressure_text(double val) {
	return format_str("%.0fhP", val);
}
void BMEToRGBMatrix::handleloopsvc(BME280Controller* ps, RGBStripController* pd) {
	TriggerFromService< BME280Controller, RGBStripController>::handleloopsvc(ps, pd);

	BMEState l = ps->get_state();
	RGBState rgbState= pd->get_state();

	RGBCMD cmd = SetText;
	rgbState.cmode = current;
	bool isSetColor=false;
	String newtext;
	//this->mode = all;
	switch (this->mode)
	{

	case temp:
	
		newtext = get_temp_text( l.temp);
		rgbState.color = calcTempColorSimple(l.temp, 0.0, 25.0);
		isSetColor = true;
		break;
	case hum:
		newtext = get_humidity_text(l.hum);
		rgbState.color = 0x0000FF;
		isSetColor = true;
		break;
	case pres:
		newtext = get_pressure_text(l.pres);
		rgbState.color = 0x00FFFF;
		isSetColor = true;
		break;
	case all:
		rgbState.color = 0x34e1eb;
		isSetColor = true;
	//case all_color_random:
	case all_color_colorwheel:
		newtext = get_temp_text(l.temp)+" "+ get_humidity_text(l.hum)+ " "+ get_pressure_text(l.pres);
		cmd = SetFloatText; 
//		if(this->mode== all_color_random)
//			rgbState.cmode = randomchar;
		if (this->mode == all_color_colorwheel)
			rgbState.cmode = color_wheel;
		break;

	default:
		break;

	}
	if(isSetColor)
		pd->AddCommand(rgbState, SetColor, srcTrigger);
	memset(rgbState.text, 0, RGB_TEXTLEN);
	strncpy(rgbState.text, newtext.c_str(), RGB_TEXTLEN);
	pd->AddCommand(rgbState, cmd, srcTrigger);
	this->mode = (DMODE)(this->mode + 1);
	if (this->mode == max) {
		this->mode = temp;
	}
}

//thing speak
BMEToThingSpeak::BMEToThingSpeak() {
	t_ch = 0;
	h_ch = 0;
	p_ch = 0;
}
void BMEToThingSpeak::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	
	//t_ch = json["t_ch"].as<uint8_t>();
	//h_ch = json["h_ch"].as<uint8_t>();
	//p_ch = json["p_ch"].as<uint8_t>();
	loadif(t_ch, json, "t_ch");
	loadif(h_ch, json, "h_ch");
	loadif(p_ch, json, "p_ch");
	//DBG_OUTPUT_PORT.println(String("BMEToThingSpeak t_ch:") + String(t_ch) + String(" h_ch:") + String(h_ch) + String(" p_ch:") + String(p_ch));
	t_ch = constrain(t_ch,0, MAX_CHANNELS);
	h_ch = constrain(h_ch,0, MAX_CHANNELS);
	p_ch = constrain(p_ch,0, MAX_CHANNELS);
	//DBG_OUTPUT_PORT.println(String("BMEToThingSpeak t_ch:") + String(t_ch)+String(" h_ch:")+ String(h_ch)+String(" p_ch:")+String(p_ch));
}
void BMEToThingSpeak::handleloopsvc(BME280Controller* ps, ThingSpeakController* pd) {
	TriggerFromService< BME280Controller, ThingSpeakController>::handleloopsvc(ps, pd);
	
	BMEState l = ps->get_state();
	ThingSpeakCMD cmd = TsSend;
	ThingSpeakState newstate=pd->get_last_commandstate();
	if (t_ch>0) 	newstate.data[t_ch-1] = l.temp;
	if (h_ch>0)  newstate.data[h_ch-1] = l.hum;
	if (p_ch>0)  newstate.data[p_ch-1] = l.pres;

	pd->AddCommand(newstate, cmd, srcTrigger);
}

//weatherDisplay

BMEToWeatherDisplay::BMEToWeatherDisplay() {

}
void BMEToWeatherDisplay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);



}
void BMEToWeatherDisplay::handleloopsvc(BME280Controller* ps, WeatherDisplayController* pd) {
	TriggerFromService< BME280Controller, WeatherDisplayController>::handleloopsvc(ps, pd);
	BMEState l = ps->get_state();
	WeatherDisplayCMD cmd = WDSetCurrentData;
	WeatherDisplayState newstate;
	newstate.data. temp = l.temp;
	newstate.data.pressure = l.pres;
	newstate.data.humidity = l.hum;

	pd->AddCommand(newstate, cmd, srcTrigger);
}

TimeToWeatherDisplay::TimeToWeatherDisplay() {

}
void TimeToWeatherDisplay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);



}
void TimeToWeatherDisplay::handleloopsvc(TimeController* ps, WeatherDisplayController* pd) {
	TriggerFromService< TimeController, WeatherDisplayController>::handleloopsvc(ps, pd);
	TimeState ts= ps->get_state();
	WeatherDisplayCMD cmd = WDSetTime;
	WeatherDisplayState newstate;
#ifdef ESP8266
	newstate.now = ts.time_withoffs;
#else
	newstate.now = ts.time_withoffs;
	
#endif

	pd->AddCommand(newstate, cmd, srcTrigger);
}

WeatherForecastToWeatherDisplay::WeatherForecastToWeatherDisplay() {

}
void WeatherForecastToWeatherDisplay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);



}
void WeatherForecastToWeatherDisplay::handleloopsvc(WeatherClientController* ps, WeatherDisplayController* pd) {
	TriggerFromService< WeatherClientController, WeatherDisplayController>::handleloopsvc(ps, pd);

	if (this->last_load < ps->get_last_load()) {
		WeatherDisplayCMD cmd = WDClearForecastData;
		WeatherDisplayState newstate;


		pd->AddCommand(newstate, cmd, srcTrigger);
		cmd = WDFreeze;
		pd->AddCommand(newstate, cmd, srcTrigger);
		cmd = WDAddForecastData;
		for (int i = 0; i < ps->getdata().GetSize(); i++) {
			//DBG_OUTPUT_PORT.println("Trigger run  ");
			//DBG_OUTPUT_PORT.println(i);
			newstate.frecord = ps->getdata()[i];
			pd->AddCommand(newstate, cmd, srcTrigger);
		}
		cmd = WDUnFreeze;
		pd->AddCommand(newstate, cmd, srcTrigger);
		this->last_load = ps->get_last_load();
	}

}

ButtonToWeatherDisplay::ButtonToWeatherDisplay() {

}
void ButtonToWeatherDisplay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);



}
void ButtonToWeatherDisplay::handleloopsvc(ButtonController* ps, WeatherDisplayController* pd) {
	TriggerFromService< ButtonController, WeatherDisplayController>::handleloopsvc(ps, pd);
	ButtonState bs=ps->get_state();
	/*
	if (bs.idx=0 && bs.isPressed && bs.changed_at!= lasttriggered) {
		WeatherDisplayCMD cmd = WDSwitchMode;
		WeatherDisplayState newstate=pd->get_state();
		pd->AddCommand(newstate, cmd, srcTrigger);
		lasttriggered = bs.changed_at;
	}
	*/
	
	long presstime = ps->get_btn_presstime(idx);
	if (ps->get_buttonsstate()[idx] == ispressed && presstime != lasttriggered) {
		WeatherDisplayCMD cmd = WDSwitchMode;
		WeatherDisplayState newstate = pd->get_state();
		pd->AddCommand(newstate, cmd, srcTrigger);
		lasttriggered = presstime;
	}
	
}

LDRToThingSpeak::LDRToThingSpeak() {
	ch = 0;
}
void LDRToThingSpeak::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	loadif(ch, json, "ch");

	ch = constrain(ch, 0, MAX_CHANNELS);


}
void LDRToThingSpeak::handleloopsvc(LDRController* ps, ThingSpeakController* pd) {
	TriggerFromService< LDRController, ThingSpeakController>::handleloopsvc(ps, pd);

	LDRState l = ps->get_state();


	ThingSpeakCMD cmd = TsSend;
	ThingSpeakState newstate = pd->get_last_commandstate();
	if (ch > 0) 	newstate.data[ch - 1] = l.cvalue;


	pd->AddCommand(newstate, cmd, srcTrigger);


}

ButtonToRelay::ButtonToRelay() {

}
void ButtonToRelay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);



}
void ButtonToRelay::handleloopsvc(ButtonController* ps, RelayController* pd) {
	TriggerFromService< ButtonController, RelayController>::handleloopsvc(ps, pd);
	ButtonState bs = ps->get_state();


	long presstime = ps->get_btn_presstime(idx);
	if (ps->get_buttonsstate()[idx] == ispressed && presstime != lasttriggered) {
		RelayState newState = pd->get_state();
		RelayCMD cmd = Set;

			newState.isOn = !newState.isOn;
		pd->AddCommand(newState, cmd, srcTrigger);
		lasttriggered = presstime;
	}

}


RFToMotion::RFToMotion() {
this->last_tick = 0;
this->last_token = 0;
this->delaywait = 300;
}
void RFToMotion::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		RFRecord & rec = *(new RFRecord());
		JsonObject json = arr[i];

		rec.isOn = arr[i][FPSTR(szisOnText)].as<bool>();
		rec.isswitch = arr[i]["isSwitch"].as<bool>();
		rec.token = arr[i]["token"].as<long>();
		rec.len = arr[i]["len"];;
		rec.protocol = arr[i]["protocol"];
		rec.pulse = arr[i]["pulse"];
		rfs.Add(rec);
	}
}
void RFToMotion::handleloopsvc(RFController* ps, MotionController* pd) {


	RFState rfstate = ps->get_state();
	if (this->last_tick == rfstate.timetick) { ///simple ignore, not new data

		return;
	}

	if (this->delaywait > 0 && this->last_token == rfstate.rftoken && (this->last_tick + this->delaywait) > millis()) {
		//TO DO implement continues pressing
		this->last_tick = rfstate.timetick;
		return;    //ignoring duplicate
	}
#ifdef	RF_TRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("RFToRelay continue with new token");
#endif 
	this->last_tick = rfstate.timetick;
	this->last_token = rfstate.rftoken;
	//	return;
	for (int i = 0; i < this->rfs.GetSize(); i++) {
		RFRecord& rec = this->rfs.GetAt(i);
		if (rec.token == rfstate.rftoken)
			this->processrecord(rec, pd);
	}



}
void RFToMotion::processrecord(RFRecord& rec, MotionController* pr) {
	MotionState newState = pr->get_state();
#ifdef	RF_TRIGGER_DEBUG
	DBG_OUTPUT_PORT.print("RFToRelay::processrecord with key");
	DBG_OUTPUT_PORT.println(rec.token);
#endif 
	MotionCMD cmd = MotionSet;
	newState.isTriggered = true;
	newState.tmTrigger = millis();
	pr->AddCommand(newState, cmd, srcTrigger);
}
