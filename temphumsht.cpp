/*
 * temphumsht.cpp
 *
 * Created: 6.10.2013 16:01:03
 *  Author: hruby
 */ 

#ifndef TEMPHUMSHT_H_
#include "temphumsht.h"
#endif

	
Temp_Hum_SHT::Temp_Hum_SHT(void) {
	dew =0;
	temp=0;
	rh=0;
	
}

int Temp_Hum_SHT::TGetSHTValues(void) {
	
	int error;
	
	error=s_measure( &rawtemp,0); //zmer teplotu
	if (error==0){
		error=s_measure( &rawhum,1); //zmer vlhkost

	}
	temp=calc_sth11_temp(rawtemp);
	rh=calc_sth11_humi(rawhum,temp);
	dew=calc_dewpoint(rh,temp);		
	
	return error;
}

void Temp_Hum_SHT::PrintConsoleAll(void) {
	Serial.print("Rawtemp:");
	Serial.println(rawtemp);
	Serial.print("Rawhum:");
	Serial.println(rawhum);
	Serial.print("Teplota:");
	Serial.print(temp/10);
	Serial.print(".");
	Serial.println(temp%10);
	Serial.print("Vlhkost:");
	Serial.print(rh/10);
	Serial.print(".");
	Serial.println(rh%10);
	Serial.print("Rosny bod:");
	Serial.print(dew/10);
	Serial.print(".");
	Serial.println(rh%10);
}



