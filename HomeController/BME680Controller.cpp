#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "BaseController.h"
#include "BME680Controller.h"
#include <Wire.h>

#include "Zanshin_BME680.h" 

//REGISTER_CONTROLLER(BME280Controller)
#ifndef ESP8266 
REGISTER_CONTROLLER_FACTORY(BME680Controller)
#endif

//BME280ControllerFactory* ff = new BME280ControllerFactory();

const size_t bufferSize = JSON_OBJECT_SIZE(20);

BME680Controller::BME680Controller() {
	this->i2caddr = 0x77;
	this->uselegacy = true;
	this->pbme = NULL;
	this->isinit = false;
	this->temp_corr = 0.0;
	this->hum_corr = 0.0;
#ifdef	ENABLE_NATIVE_HAP
	this->ishap=true;
	this->hapservice_temp=NULL;
	this->hapservice_hum=NULL;
	this->hapservice_press=NULL;

	this->hap_temp=NULL;
	this->hap_hum=NULL;
	this->hap_press=NULL;
#endif
}
String  BME680Controller::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;
	root[FPSTR(sztemp)] = this->get_state().temp;
	root[FPSTR(szhum)] = this->get_state().hum;
	root[FPSTR(szpres)] = this->get_state().pres;
	String json;

	serializeJson(root, json);

	return json;
}
bool  BME680Controller::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	BME680State newState;
	newState.isOn = root[FPSTR(szisOnText)];
	newState.temp = root[FPSTR(sztemp)];
	newState.hum = root[FPSTR(szhum)];
	newState.pres = root[FPSTR(szpres)];
	this->AddCommand(newState, BMESet, src);
	//this->set_state(newState);
	return true;

}
void BME680Controller::loadconfig(JsonObject& json) {
	i2caddr = json[FPSTR(szi2caddr)];
	uselegacy = json["uselegacy"];
	loadif(temp_corr, json, "temp_corr");
	loadif(hum_corr, json, "hum_corr");
}
void BME680Controller::getdefaultconfig(JsonObject& json) {
	json[FPSTR(szi2caddr)]= i2caddr;
	json["uselegacy"]= uselegacy;
	json[FPSTR(szservice)] = "BME280Controller";
	json[FPSTR(szname)] = "BME";
	json["temp_corr"] = 0.0;
	json["hum_corr"] = 0.0;
	BME680::getdefaultconfig(json);
}
#define BME_SCK 13
#define BME_SDI_MISO 2
#define BME_SDO_MOSI 5
#define BME_CS 14
void  BME680Controller::setup() {
	DBG_OUTPUT_PORT.println("BME680Controller setup");
	if (this->uselegacy) {
		DBG_OUTPUT_PORT.println("Init Adafruit_BME680");
		this->pbme = new BME680_Class();
		//this->pbme=new Adafruit_BME280 (BME_CS, BME_SDO_MOSI, BME_SDI_MISO, BME_SCK); // software SPI
		if (!this->pbme->begin(I2C_STANDARD_MODE)) { // Start BME680 using I2C, use first device found
			DBG_OUTPUT_PORT.println("-  Unable to find BME680. ");
			//delay(5000);
		}  // of loop until device is located

		else {
			this->isinit = true; 
				this->pbme->setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
				this->pbme->setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
				this->pbme->setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
				//Serial.print(F("- Setting IIR filter to a value of 4 samples\n"));
				this->pbme->setIIRFilter(IIR4);  // Use enumerated type values
			//Serial.print(F("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n"));  // "°C" symbols
			this->pbme->setGas(320, 150);  // 320°c for 150 milliseconds
		}
	}
	else {
		//DBG_OUTPUT_PORT.println("Init Direct read");
		//DBG_OUTPUT_PORT.println("On address");
		DBG_OUTPUT_PORT.println(this->i2caddr);
			
		Wire.begin();
		//soft reset chip
		write8(0xE0, 0xB6);
		delay(100);
	}
	
}
void BME680Controller::write8(byte reg, byte value) {

	Wire.beginTransmission((uint8_t)this->i2caddr);
	Wire.write((uint8_t)reg);
	Wire.write((uint8_t)value);
	Wire.endTransmission();
}
void BME680Controller::run() {
	if (this->commands.GetSize() == 0) {
		command newcmd;
		newcmd.mode = BMEMeasure;
		this->meassure(newcmd.state);
#ifdef  BMECONTROLLER_DEBUG
		DBG_OUTPUT_PORT.print("BME280Controller->");
		DBG_OUTPUT_PORT.print("Temperature in Celsius : ");
		DBG_OUTPUT_PORT.print(newcmd.state.temp);
		DBG_OUTPUT_PORT.print("Pressure : ");
		DBG_OUTPUT_PORT.print(newcmd.state.pres);
		DBG_OUTPUT_PORT.print("Relative Humidity : ");
		DBG_OUTPUT_PORT.print(newcmd.state.hum);
		DBG_OUTPUT_PORT.println(" RH");
#endif //  LDRCONTROLLER_DEBUG

		//this->commands.Add(newcmd);
		this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
	command cmd;

	while (commands.Dequeue(&cmd)) {
		if (this->baseprocesscommands(cmd))
			continue;
		BME680State newState = cmd.state;

		this->set_state(newState);
	}
	BME680::run();
}


bool BME680Controller::onpublishmqtt(String& endkey, String& payload) {
	endkey = szStatusText;
	payload = String(this->get_state().isOn ? 1 : 0);
	return true;
}
void BME680Controller::onmqqtmessage(String topic, String payload) {
	
}

void BME680Controller::meassure(BME680State& state) {
	static int32_t  temp, humidity, pressure, gas;
	if (this->uselegacy) {
		if (this->pbme && this->isinit) {
			this->pbme->getSensorData(temp, humidity, pressure, gas);
			state.temp = constrain(temp / 100.0F,-30,100);
			state.pres = constrain(pressure / 100.0F,0,200);
			state.hum = constrain(humidity / 1000.0F, 0, 100);
			state.gas = gas / 100.0F;
		}
	}
	else {
		
		this->directmeassure(state);
	}
	state.temp += temp_corr;
	state.hum += hum_corr;
}

void BME680Controller::directmeassure(BME680State& state) {
	unsigned int b1[24];
	unsigned int data[8];
	unsigned int dig_H1 = 0;
	for (int i = 0; i < 24; i++)
	{
		// Start I2C Transmission
		Wire.beginTransmission(this->i2caddr);
		// Select data register
		Wire.write((136 + i));
		// Stop I2C Transmission
		Wire.endTransmission();

		// Request 1 byte of data
		Wire.requestFrom(this->i2caddr, (uint8_t)1);

		// Read 24 bytes of data
		if (Wire.available() == 1)
		{
			b1[i] = Wire.read();
		}
		delay(10);
	}

	// Convert the data
	// temp coefficients
	unsigned int dig_T1 = (b1[0] & 0xff) + ((b1[1] & 0xff) * 256);
	int dig_T2 = b1[2] + (b1[3] * 256);
	int dig_T3 = b1[4] + (b1[5] * 256);

	// pressure coefficients
	unsigned int dig_P1 = (b1[6] & 0xff) + ((b1[7] & 0xff) * 256);
	int dig_P2 = b1[8] + (b1[9] * 256);
	int dig_P3 = b1[10] + (b1[11] * 256);
	int dig_P4 = b1[12] + (b1[13] * 256);
	int dig_P5 = b1[14] + (b1[15] * 256);
	int dig_P6 = b1[16] + (b1[17] * 256);
	int dig_P7 = b1[18] + (b1[19] * 256);
	int dig_P8 = b1[20] + (b1[21] * 256);
	int dig_P9 = b1[22] + (b1[23] * 256);

	// Start I2C Transmission
	Wire.beginTransmission(this->i2caddr);
	// Select data register
	Wire.write(161);
	// Stop I2C Transmission
	Wire.endTransmission();

	// Request 1 byte of data
	Wire.requestFrom(this->i2caddr, (uint8_t)1);

	// Read 1 byte of data
	if (Wire.available() == 1)
	{
		dig_H1 = Wire.read();
	}

	for (int i = 0; i < 7; i++)
	{
		// Start I2C Transmission
		Wire.beginTransmission(this->i2caddr);
		// Select data register
		Wire.write((225 + i));
		// Stop I2C Transmission
		Wire.endTransmission();

		// Request 1 byte of data
		Wire.requestFrom(this->i2caddr, (uint8_t)1);

		// Read 7 bytes of data
		if (Wire.available() == 1)
		{
			b1[i] = Wire.read();
		}
	}

	// Convert the data
	// humidity coefficients
	int dig_H2 = b1[0] + (b1[1] * 256);
	unsigned int dig_H3 = b1[2] & 0xFF;
	int dig_H4 = (b1[3] * 16) + (b1[4] & 0xF);
	int dig_H5 = (b1[4] / 16) + (b1[5] * 16);
	int dig_H6 = b1[6];

	// Start I2C Transmission
	Wire.beginTransmission(this->i2caddr);
	// Select control humidity register
	Wire.write(0xF2);
	// Humidity over sampling rate = 1
	Wire.write(0x01);
	// Stop I2C Transmission
	Wire.endTransmission();

	// Start I2C Transmission
	Wire.beginTransmission(this->i2caddr);
	// Select control measurement register
	Wire.write(0xF4);
	// Normal mode, temp and pressure over sampling rate = 1
	Wire.write(0x27);
	// Stop I2C Transmission
	Wire.endTransmission();

	// Start I2C Transmission
	Wire.beginTransmission(this->i2caddr);
	// Select config register
	Wire.write(0xF5);
	// Stand_by time = 1000ms
	Wire.write(0xA0);
	// Stop I2C Transmission
	Wire.endTransmission();

	for (int i = 0; i < 8; i++)
	{
		// Start I2C Transmission
		Wire.beginTransmission(this->i2caddr);
		// Select data register
		Wire.write((247 + i));
		// Stop I2C Transmission
		Wire.endTransmission();

		// Request 1 byte of data
		Wire.requestFrom(this->i2caddr, (uint8_t)1);

		// Read 8 bytes of data
		if (Wire.available() == 1)
		{
			data[i] = Wire.read();
		}
		
	}

	// Convert pressure and temperature data to 19-bits
	long adc_p = (((long)(data[0] & 0xFF) * 65536) + ((long)(data[1] & 0xFF) * 256) + (long)(data[2] & 0xF0)) / 16;
	long adc_t = (((long)(data[3] & 0xFF) * 65536) + ((long)(data[4] & 0xFF) * 256) + (long)(data[5] & 0xF0)) / 16;
	// Convert the humidity data
	long adc_h = ((long)(data[6] & 0xFF) * 256 + (long)(data[7] & 0xFF));

	// Temperature offset calculations
	double var1 = (((double)adc_t) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
	double var2 = ((((double)adc_t) / 131072.0 - ((double)dig_T1) / 8192.0) *
		(((double)adc_t) / 131072.0 - ((double)dig_T1) / 8192.0)) * ((double)dig_T3);
	double t_fine = (long)(var1 + var2);
	double cTemp = (var1 + var2) / 5120.0;
	double fTemp = cTemp * 1.8 + 32;

	// Pressure offset calculations
	var1 = ((double)t_fine / 2.0) - 64000.0;
	var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
	var2 = var2 + var1 * ((double)dig_P5) * 2.0;
	var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
	var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
	double p = 1048576.0 - (double)adc_p;
	p = (p - (var2 / 4096.0)) * 6250.0 / var1;
	var1 = ((double)dig_P9) * p * p / 2147483648.0;
	var2 = p * ((double)dig_P8) / 32768.0;
	double pressure = (p + (var1 + var2 + ((double)dig_P7)) / 16.0) / 100;

	// Humidity offset calculations
	double var_H = (((double)t_fine) - 76800.0);
	var_H = (adc_h - (dig_H4 * 64.0 + dig_H5 / 16384.0 * var_H)) * (dig_H2 / 65536.0 * (1.0 + dig_H6 / 67108864.0 * var_H * (1.0 + dig_H3 / 67108864.0 * var_H)));
	double humidity = var_H * (1.0 - dig_H1 * var_H / 524288.0);

	if (humidity > 100.0)
	{
		humidity = 100.0;
	}
	else if (humidity < 0.0)
	{
		humidity = 0.0;
	}
	// Output data to serial monitor
#ifdef  BMECONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("Direct measure");
	DBG_OUTPUT_PORT.print("Temperature in Celsius : ");
	DBG_OUTPUT_PORT.print(cTemp);
	DBG_OUTPUT_PORT.println(" C");
	DBG_OUTPUT_PORT.print("Temperature in Fahrenheit : ");
	DBG_OUTPUT_PORT.print(fTemp);
	DBG_OUTPUT_PORT.println(" F");
	DBG_OUTPUT_PORT.print("Pressure : ");
	DBG_OUTPUT_PORT.print(pressure);
	DBG_OUTPUT_PORT.println(" hPa");
	DBG_OUTPUT_PORT.print("Relative Humidity : ");
	DBG_OUTPUT_PORT.print(humidity);
	DBG_OUTPUT_PORT.println(" RH");
#endif
	cTemp = isnan(cTemp) ? 0.0 : cTemp;
	pressure = isnan(pressure) ? 0.0 : pressure;
	humidity = isnan(humidity) ? 0.0 : humidity;
	state.temp =round( constrain(cTemp,-30.0,100.0)*100)/100;
	state.pres = round(constrain(pressure,0.0,2000.0)*100)/100;
	state.hum =round(constrain( humidity,0.0,100.0)*100)/100;
	

}



#ifdef	ENABLE_NATIVE_HAP

void BME680Controller::setup_hap_service(){

#ifdef  BMECONTROLLER_DEBUG
	DBG_OUTPUT_PORT.println("BME280Controller::setup_hap_service()");
#endif

	if(!ishap)
		return;
	if(this->accessory_type>1){
#ifdef  BMECONTROLLER_DEBUG
			DBG_OUTPUT_PORT.println("BME280Controller adding as new accessory");
#endif
			//hap_add_temp_hum_as_accessory(this->accessory_type,this->get_name(),&this->hapservice_temp,&this->hapservice_hum);
			this->hapservice_temp = hap_add_temp_as_accessory(this->accessory_type, this->get_name());
			this->hapservice_hum = hap_add_hum_as_accessory(this->accessory_type, this->get_name());
		}
	else{
		String tempName = this->get_name();
		tempName += "temp";
		String humName=  this->get_name();
		humName += "hum";
		this->hapservice_temp = hap_add_temperature_service(tempName.c_str());
		this->hapservice_hum=hap_add_humidity_service(humName.c_str());
	}
	if(this->hapservice_temp)
		this->hap_temp=homekit_service_characteristic_by_type(this->hapservice_temp, HOMEKIT_CHARACTERISTIC_CURRENT_TEMPERATURE);
	if(this->hapservice_hum)
		this->hap_hum=homekit_service_characteristic_by_type(this->hapservice_hum, HOMEKIT_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY);

	DBG_OUTPUT_PORT.println(String("hap_temp")+String(int(hap_temp)));
	DBG_OUTPUT_PORT.println(String("hap_hum") + String(int(hap_hum)));


}
void BME680Controller::notify_hap(){
	if(this->ishap && this->hapservice_temp && this->hap_temp && this->isinit){
		BME680State newState=this->get_state();
		if(this->hap_temp->value.float_value!=newState.temp){
			this->hap_temp->value.float_value=newState.temp;
				  homekit_characteristic_notify(this->hap_temp,this->hap_temp->value);
		}
	}
	if(this->ishap && this->hapservice_hum && this->hap_hum && this->isinit){
		BME680State newState=this->get_state();
		if(this->hap_hum->value.float_value!=newState.hum){
			this->hap_hum->value.float_value = (float)newState.hum;
				  homekit_characteristic_notify(this->hap_hum,this->hap_hum->value);
		}
	}

}


void BME680Controller::hap_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){


	if(!context){
		return;
	};

}
#endif

