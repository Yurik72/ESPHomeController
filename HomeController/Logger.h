#ifndef __LOGGERLOG_H
#define __LOGGERLOG_H

#include <stddef.h>
#include <Print.h>
#include <Stream.h>
#include <WString.h>
#include <Arduino.h>
#include "config.h"



class Logger:public Print {
public:
	Logger(size_t size = 0);
	Logger(Stream& extra,size_t size=0);
	Logger(HardwareSerial& extra, size_t size=0);
#if defined(ESP8266)
	void begin(unsigned long baud);
#else
	void begin(unsigned long baud, uint32_t config = SERIAL_8N1, int8_t rxPin = -1, int8_t txPin = -1, bool invert = false);
#endif
	using Print::write;
	size_t write(uint8_t ch) override;
	const String& LOG() const {
		return slog;
	}
protected:
	void setup();
	String slog;
	Stream* pextradupl;
	HardwareSerial* pSerial;
	size_t logsize;
};
extern Logger ESPLogger;
#endif


