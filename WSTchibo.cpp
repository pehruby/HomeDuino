/*
 * WSTchibo.cpp
 *
 * Created: 27.10.2013 13:50:27
 *  Author: hruby
 *
 * Weather sensor Tchibo
 */ 


/*

Doplnit o podminku/vyber kanalu 1,2,3

*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "WSTchibo.h"

WSTchibo::WSTchibo() {
	
	_acquired[0] = _acquired[1] = _acquired[2] = false;
}

void WSTchibo::accept(byte interval, byte level) {
	byte sample;		// bin0=0, bin1=1, init=2, oddelovac =4
//	static byte lastsample=0xFF;
	static byte state=0;
	static byte prijatebity;
	static bool cekamoddel;
	static bool opakovani=false;
    byte chid;			//cislo kanalu aktualniho paketu;
	byte i;
	
	/*state		0 - nic
				1 - prvni oddelovac
				2 - prvni init
				3 - druhy oddelovac
				4 - druhy init
				5 - prenos 42 bitu pocinaje oddelovacem

	*/
	
/*	if (state == 5 ) {
		Serial.print("Interval:");
		Serial.println(interval);
		Serial.print("Level:");
		Serial.println(level);
	} */
	if (level) {					// uroven "1"
		if (interval >= 9 && interval <= 15) {
			sample=4;				// je to oddelovac
		} else {
			state=0;				// neni to nic
			return;
		}
	} else {						// uroven "0"
		if (interval >= 30 && interval <= 46) {
			sample=0;
		} else {
			if (interval >= 68 && interval <= 86) {
				sample=1;
			} else {
				if (interval >= 140 && interval <= 168) {
					sample=2;
				} else {
					state=0;
					return;
				}
		
			}
		}
	}
	
	
	switch (state) {
		case 0:
			if (sample == 4) {			// prijat 1. oddelovac, prejdi do state 1
				state=1;
				prijatebity=0;
				cekamoddel=true;
				opakovani=false;
			}
			break;
		case 1:
			if (sample == 2) {				// prijat 1.init, prejdi do state 2
				prijatebity=0;
				state=2;
			} else {
				state=0;					// 1.init neprisel, zpet do state 0
			}
			break;
		case 2:
			if (sample == 4) {				//prijat 2. oddelovac
				state=3;
			} else {
				state=0;
			}
			break;
		case 3:
			if (sample == 2) {				// prijat 2.init
				// _acquired=false;
				_packet[0] = _packet[1] = _packet[2] = _packet[3] = _packet[4] = _packet[5] = 0;
				state=4;
			} else {
				state=0;
			}
			break;
		case 4:					// byla prijate cela preambule a ted budeme zpracovavat 42 bitu
//			Serial.println("Preambule prijata");
			if (cekamoddel) {					// mel by prijit oddelovac
				if (sample == 4) {				// je tu oodelovac
					cekamoddel=false;			// priste bude nejaka hodnota
				} else {
					state=0;					// error
//					Serial.println("Err 1");
					break;
				}
			} else {							// mel by prijit nejaky bit
				if (sample == 1) {				// je to hodnota 1
					prijatebity++;
					_packet[(prijatebity+5)/8] |= (1<<((TCHBITU-prijatebity) % 8));
					cekamoddel=true;			//
				} else {		
					if (sample == 0) {			// je to hodnota 0
						prijatebity++;
						cekamoddel=true;		// priste bude oddelovac
					} else {
						state=0;				//error
//						Serial.println("Err2");
//						Serial.println(sample);
						break;
					}
				}
				if (prijatebity == TCHBITU) {
					state = 5;
					
					chid=(_packet[2]&0x3f)>>4;
					//Serial.println(chid);
					
					for (i=0;i<6;i++) {
						if (!opakovani) {		// je to prvni paket zpravy
							_finpacket[chid][i]=_packet[i];
						} else {				// neni to prvni paket
							if (_finpacket[chid][i] != _packet[i]) {		//porovnej to s prvnim
								_acquired[chid]=false;					// neco nesouhlasi, asi doslo k chybe pri prenosu
								Serial.println("Chyba pri prenosu z Tchibo cidla");
							}
						}
					}
					if (!opakovani)
						_acquired[chid]=true;				// prijal jsem prvni paket a zatim ho budu povazovat za spravny. Co kdyz neni spravne prijaty? PRILEZITOSTNE PREDELAT

/*					Serial.println("Prijato");
					Serial.println(_packet[0],HEX);
					Serial.println(_packet[1],HEX);
					Serial.println(_packet[2],HEX);
					Serial.println(_packet[3],HEX);
					Serial.println(_packet[4],HEX);
					Serial.println(_packet[5],HEX); */
				} 
			}
			break;
		case 5:					//kdyz prijde po prijeti 32 bitu oddelovac, prejdi do stavu 1
			if (sample == 4) {
				state = 1;
				opakovani=true;	//pristi paket bude opakovani stavajiciho
			} else {
				state = 0;
				opakovani=false;	// uz jsme na konci prijmu sekvence paketu
			}
			break;
		
	}
/*	if (state > 1) {
//		Serial.print("State:");
//		Serial.println(state);
		if (state == 4) {
			Serial.print("Prijate bity:");
			Serial.println(prijatebity);
		} 
	} */
}

byte* WSTchibo::getpacet() {
	return _packet;
}

bool WSTchibo::acquired(byte ch) {
	bool temp = _acquired[ch];
	_acquired[ch] = false;
	return temp;
}

byte WSTchibo::get_humidity(byte ch) {
	byte humi=_finpacket[ch][4];
	
	humi = (humi>>4) | (humi<<4);	//4xROL
	return humi;
}

uint16_t WSTchibo::get_temp_raw(byte ch) {
	uint16_t temp;
	
	temp = (_finpacket[ch][3]>>4) | (_finpacket[ch][3]<<4);
	temp=(temp<<4)& 0x0FF0;
	temp = temp | (_finpacket[ch][2] & 0x0F);
	return temp;
	
}

int WSTchibo::get_temp(byte ch) {
	uint16_t rawtemp;
//	int realtemp10;
	float realtemp=0;
	float ka=-678.275909;
	float kb=0.5558508;
	
	rawtemp=get_temp_raw(ch);
	realtemp=rawtemp*kb;
	realtemp+=ka;
//	realtemp10;	
/*	if (_acquired)
		return (int)realtemp;
	else
		return -9999;
	
*/	
	return (int)realtemp;
}	

bool WSTchibo::lowbatt(byte ch) {
	return (bool)(_finpacket[ch][5]>>6);
}


