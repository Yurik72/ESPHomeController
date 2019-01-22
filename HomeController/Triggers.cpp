#include <FS.h>
#if !defined(ESP8266)
#include <SPIFFS.h>
#endif
#include "config.h"
#include "TimeController.h"
#include "RGBStripController.h"
#include "LDRController.h"
#include "Triggers.h"
#include "Utilities.h"
#include "RelayController.h"
#include "Controllers.h"



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
}

void Trigger::loadconfig(JsonObject& json) {
	src = json["source"].as<String>();
	dst = json["destination"].as<String>();
	

}
Trigger* Triggers::CreateByType(const char* nametype) {
	Trigger* res = NULL;
	if (strcmp(nametype, "TimeToRGBStrip") == 0) {
		res= new TimeToRGBStripTrigger();
	}
	else if (strcmp(nametype, "LDRToRGBStrip") == 0) {
		res = new LDRToRGBStrip();
	}
	else {
		res = new Trigger();
	}
	res->type = nametype;
	return res;
}
//service base;
template<class SRC, class DST>
TriggerFromService<SRC, DST>::TriggerFromService() {
	this->pDst = NULL;
	this->pSrc = NULL;
}
template<class SRC, class DST>
void TriggerFromService<SRC, DST>::handleloop(CBaseController* pBase, Controllers* pctls) {
#ifdef	TRIGGER_DEBUG
	DBG_OUTPUT_PORT.println("TriggerFromService handleloop");
#endif
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
	DBG_OUTPUT_PORT.println((long)this);
#endif
	if (rec.timetype == dailly) {
		if (rec.timeToTrigger == 0) { //not triggered yet

			rec.timeToTrigger = apply_hours_minutes_fromhhmm(currentTime, rec.time,this->get_timeoffs());
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
			if (!istimetotrigger(rec.lastTriggered, currentTime)) { //already triggered
				//set next time to trigger
				time_t nextday = currentTime + NEXT_DAY_SEC;
				rec.timeToTrigger = apply_hours_minutes_fromhhmm(nextday, rec.time,this->get_timeoffs());
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
	return  (time > currentTime) && ((time - currentTime) < 1200);  //2min
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

		rec.color= arr[i]["color"].as<int>();
		rec.brightness = arr[i]["bg"].as<int>();
		rec.wxmode = arr[i]["wxmode"].as<int>();
		times.Add(rec);
	}
}





void TimeToRGBStripTrigger::dotrigger(timerecRGB & rec, Controllers* pctlss) {
	if (!this->get_stripctl()) {
		CBaseController* pBase = pctlss->GetByName(this->dst.c_str());
		if (pBase == NULL) {
			DBG_OUTPUT_PORT.println("Destination service not found");
			return;
		}

		RGBStripController *pStrip = static_cast<RGBStripController*>(pBase);
		this->set_stripctl(pStrip);
	}
	RGBState newstate = pStrip->get_state();

	if (rec.isOn) {
		newstate.brightness = rec.brightness;
		this->get_stripctl()->AddCommand(newstate, SetBrigthness, srcTrigger);
		newstate.color = rec.color;
		this->get_stripctl()->AddCommand(newstate, SetColor, srcTrigger);
	}
	else{

		this->get_stripctl()->AddCommand(newstate, Off, srcTrigger);
	}

}
///ldr -> RGB brigthness
LDRToRGBStrip::LDRToRGBStrip() {

}

void LDRToRGBStrip::handleloopsvc(LDRController* ps, RGBStripController* pd) {
	LDRState l = ps->get_state();
	RGBState rState;
	rState.ldrValue = l.ldrValue;
	RGBCMD cmd = SetLdrVal;
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