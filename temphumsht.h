/*
 * temphumsht.h
 *
 * Created: 6.10.2013 16:34:42
 *  Author: hruby
 */ 


#ifndef TEMPHUMSHT_H_
#define TEMPHUMSHT_H_

#ifndef SENSIRION_PROTOCOL_H
#include "sensirion_protocol.h"
#endif

#ifndef Arduino_h
#include <Arduino.h>
#endif

class Temp_Hum_SHT {
	
	private:
	unsigned int rawhum, rawtemp;
	public:
	int dew, temp, rh;		//vse x10
	
	
	Temp_Hum_SHT(void);
	int TGetSHTValues(void);
	void PrintConsoleAll(void);
	
};




#endif /* TEMPHUMSHT_H_ */