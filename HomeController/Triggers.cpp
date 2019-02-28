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

//REGISTER_TRIGGER(TimeToRGBStripTrigger)
//REGISTER_TRIGGER(TimeToRelayTrigger)
//REGISTER_TRIGGER(LDRToRelay)
//REGISTER_TRIGGER(LDRToRGBStrip)
//REGISTER_TRIGGER(RFToRelay)


REGISTER_TRIGGER_FACTORY(TimeToRGBStripTrigger)
REGISTER_TRIGGER_FACTORY(TimeToRelayTrigger)
REGISTER_TRIGGER_FACTORY(LDRToRelay)
REGISTER_TRIGGER_FACTORY(LDRToRGBStrip)
REGISTER_TRIGGER_FACTORY(RFToRelay)

void Triggers::setup() {
	this->loadconfig();
	
	//for (int i = 0; i < this->GetSize(); i++)
	//	this->GetAt(i)->setup();

}
void Triggers::loadconfig() {
	String filename = "/triggers.json";
	int capacity = JSON_ARRAY_SIZE(5) + 2 * JSON_OBJECT_SIZE(40) + 262;
	if (SPIFFS.exists(filename)) {
		//file exists, reading and loading
		DBG_OUTPUT_PORT.println("Read triggers configuration: ");
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
}

void Trigger::loadconfig(JsonObject& json) {
	src = json["source"].as<String>();
	dst = json["destination"].as<String>();
	

}
Trigger* Triggers::CreateByType(const char* nametype) {
	TriggerFactory* pFactory = Factories::get_triggerfactory(nametype);
	if (!pFactory) {
		DBG_OUTPUT_PORT.println("TriggerFactory not defined");
		pFactory = Factories::get_triggerfactory(String(nametype)+String("Trigger"));
	}
	if (pFactory) {
		Trigger* pTrigger = pFactory->create();

		pTrigger->type = nametype;
		return pTrigger;
	}
	DBG_OUTPUT_PORT.print("OPS return default trigger");
	DBG_OUTPUT_PORT.println(nametype);
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
	res->type = nametype;
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
			DBG_OUTPUT_PORT.println("Destination service not found");
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
	if (rec.timetype == dailly) {
		stime.trim();
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
	if (rec.timetype == dailly) {
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
		rec.isOn = arr[i]["isOn"].as<bool>();
		rec.color= arr[i]["color"].as<int>();
		rec.brightness = arr[i]["bg"].as<int>();
		rec.wxmode = arr[i]["wxmode"].as<int>();
		rec.isLdr = arr[i]["isLdr"].as<int>();
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
		rec.isOn = arr[i]["isOn"].as<bool>();


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
			DBG_OUTPUT_PORT.println("Destination service not found");
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
			DBG_OUTPUT_PORT.println("Destination service not found");
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
		DBG_OUTPUT_PORT.println("Sending command On");
		
		newstate.isOn = true;
		newstate.isLdr = rec.isLdr;
		newstate.brightness = rec.brightness;
		newstate.color = rec.color;
		
		this->get_stripctl()->AddCommand(newstate, SetRGB, srcTrigger);
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
		this->get_stripctl()->AddCommand(newstate, Off, srcTrigger);
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
	this->delaywait = 300;
}
void RFToRelay::loadconfig(JsonObject& json) {
	Trigger::loadconfig(json);
	JsonArray arr = json["value"].as<JsonArray>();
	for (int i = 0; i < arr.size(); i++) {
		RFRecord & rec = *(new RFRecord());
		JsonObject json = arr[i];
		
		rec.isOn = arr[i]["isOn"].as<bool>();
		rec.isswitch = arr[i]["isswitch"].as<bool>();
		rec.rfkey = arr[i]["rfkey"].as<long>();

		rfs.Add(rec);
	}
}
void RFToRelay::handleloopsvc(RFController* ps, RelayController* pd) {
	

	RFState rfstate = ps->get_state();
	if (this->last_tick == rfstate.timetick)
		return;
	if (this->delaywait && (this->last_tick + this->delaywait) < millis())
		return;    //ignoring duplicate
#ifdef	RF_TRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("RFToRelay detected new value");
#endif 
	this->last_tick = rfstate.timetick;
	for (int i = 0;i < this->rfs.GetSize();i++) {
		RFRecord& rec = this->rfs.GetAt(i);
		if (rec.rfkey == rfstate.rftoken)
			this->processrecord(rec, pd);
	}



}
void RFToRelay::processrecord(RFRecord& rec, RelayController* pr) {
	RelayState newState=pr->get_state();
#ifdef	RF_TRIGGER_DEBUG
	DBG_OUTPUT_PORT.print ("RFToRelay::processrecord with key");
	DBG_OUTPUT_PORT.println(rec.rfkey);
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