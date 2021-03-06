#include "config.h"
#include "Logger.h"

#ifdef LOG_STRING_LOGGER
#ifdef  LOG_SERIAL
#ifdef ESP32
Logger ESPLogger(Serial,3000);
#else
Logger ESPLogger(Serial,0);
#endif
#else
Logger EspLogger;
#endif
#endif

Logger::Logger(size_t size) {
	pextradupl = NULL;
	pSerial = NULL;
	this->logsize = size;
	this->setup();
}
Logger::Logger(Stream& extra, size_t size):Logger(size)

{
	pextradupl = &extra;
	this->logsize = size;
	this->setup();
}

Logger::Logger(HardwareSerial& extra, size_t size) :Logger(size) {
	pSerial = &extra;
	this->logsize = size;
	
}
void Logger::setup() {
	if (this->logsize > 0) {
		slog.reserve(this->logsize);
	}

}
size_t Logger::write(uint8_t ch) {
	
	if (this->logsize > 0) {
		size_t len = this->slog.length();
		if (len >= this->logsize) {  //  cleanup 
			this->slog.remove(0, this->logsize / 2);
		}
		this->slog += (char)ch;
	}
	if (pextradupl)
		pextradupl->write(ch);
	if (pSerial)
		pSerial->write(ch);
	return sizeof(ch);
}
#if defined(ESP8266)
void Logger::begin(unsigned long baud) {
	if (pSerial)
		pSerial->begin(baud);
#else
void Logger::begin(unsigned long baud, uint32_t config , int8_t rxPin, int8_t txPin , bool invert){
	if (pSerial)
		pSerial->begin(baud, config, rxPin, txPin, invert);
#endif 
}