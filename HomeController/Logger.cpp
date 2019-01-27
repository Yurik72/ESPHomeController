#include "Logger.h"

#ifdef LOG_STRING_LOGGER
#ifdef  LOG_SERIAL
Logger ESPLogger(Serial);
#else
Logger EspLogger;
#endif
#endif

Logger::Logger(size_t size) {
	pextradupl = NULL;
	pSerial = NULL;
	this->logsize = size;
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
	
	size_t len=this->slog.length();
	if (len >= this->logsize) {  //  cleanup 
		 this->slog.remove(0, this->logsize / 2);
	}
	this->slog+= (char)ch;

	if (pextradupl)
		pextradupl->write(ch);
	if (pSerial)
		pSerial->write(ch);
	return sizeof(ch);
}
void Logger::begin(unsigned long baud, uint32_t config , int8_t rxPin, int8_t txPin , bool invert){
	if (pSerial)
		pSerial->begin(baud, config, rxPin, txPin, invert);
}