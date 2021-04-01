#pragma once
#include "Arduino.h"

#include "Array.h"
#include <FS.h>

#if !defined(ESP8266)
#include <SPIFFS.h>
#endif

template<class T>
class CBMEHistory :public CSimpleArray <T>{
public:
	typedef void(*WriteElementFN)(Stream* ps,T elem);
	typedef String(*GetStringElement)( T elem);
	void WriteToStream(Stream* ps) {
		for (int i = 0; i < this->GetSize(); i++) {
			pfn_write_element(ps,this->GetAt(i));
		}
	}
	bool WriteToFile(String fileName) {
		File file = SPIFFS.open(fileName, "w");
		if (!file)
			return false;
		file.print("[");
		for (int i = 0; i < this->GetSize(); i++) {
			String s = pfn_get_string_element(this->GetAt(i));
			file.print(s);
			if (i == (this->GetSize() - 1)) {
				file.print("\r\n");
			}
			else {
				file.print(",\r\n");
			}

		}
		file.print("]");
		file.close();
		return true;
	}
	void SetWriteFn(WriteElementFN fn) { pfn_write_element = fn; };
	void SetGetStringFn(GetStringElement fn) { pfn_get_string_element = fn; };
private:
	WriteElementFN pfn_write_element;
	GetStringElement pfn_get_string_element;
};


