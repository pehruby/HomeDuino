/*
 * iqrf.cpp
 *
 * Created: 8.12.2013 20:30:06
 *  Author: hruby
 */ 

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "iqrf.h"
#include "mysql.h"
#include "Nimbits.h"		// pro floatToString, predelat !!!

extern MySQL MyDatabase;				// je definovano v Main.cpp
// extern MySQL MyEnerDatabase;

IQRF::IQRF() {
	
	int i;
	for (i=0;i<MAXENVENTR;i++) {	
		//energo
		_enerval[i].value =0;			//vynuluj hodnoty v poli
		_enerval[i].expseq =0;
		// enviro
		_envval[i].platna=false;		// na zacatku neni zadna hodnota platna
		_envval[i].zapsano=true;		// na zacatku neni treba zadnou hodnotu zapisovat do MySQL
		
		_enerval[i].channame[0]=0;		//prazdny nazev kanalu
		_enerval[i].channame[LENCHANNAME]=0;		//prazdny nazev kanalu
	}
	
};



int IQRF::readbyte(byte hodnota) {
	
	char* aktbytepaket = reinterpret_cast<char *>(&_aktpaket);		// ze struct udelam pole znaku;
	
	switch (_state) {
		
		case 0:
			if (_aktdelka == 0 && hodnota == IQPREAMBLE) {	
				_state = 1;										// prijata preambule
			}
			break;
		case 1:
			if (_aktdelka == 0 && hodnota == PAKETLEN) {
				_state = 2;										// prijat 1.byte - delka paketu
				aktbytepaket[_aktdelka++]=hodnota;
			}
			break;
		case 2: 
			aktbytepaket[_aktdelka++]=hodnota;				// prijimame zbyla pakety
			if (_aktdelka == PAKETLEN) {						// nacten cely paket
				_novypaket=true;
				_state=0;
			}
			break;
	}
	
	
	return 0;
	
}


int IQRF::zpracujpaket() {
	
	int i;
	char* aktbytepaket = reinterpret_cast<char *>(&_aktpaket);
	
	
	
	_rozdilsnv=0;
	
	Serial.println("--- Zpracovany paket ---");
	for (i=0;i<PAKETLEN;i++) {
		Serial.print(aktbytepaket[i],HEX);
		Serial.print(",");
	}
	Serial.println("");
	Serial.println("--- ---------------- ---"); 
	if (CRCcheck() !=0) {
		Serial.println("Chyba CRC");
		_aktdelka=0;
		_novypaket=false;
		return -1;
	}

	if ((_aktpaket.ctrl & 0x7f) == 0) {			// je to bin hodnota ?
		SNcheck();								// prekontroluj SN
	}
	
	// jaky je stav baterie
	
	if (_aktpaket.battery < 7) {
		Serial.print("Slaba baterie:");
		Serial.println(_aktpaket.battery);			// 2.25 V + _aktpaket.battery × 0.1 V
	}
	

	// zpracovani prijate zpravy
	
	
	switch (_aktpaket.type) {
		
		case 1:							//energo
			zpracuj_energo();
			break;
		
		case 2:							//enviro
		
			zpracuj_enviro();
			break;
		
		case 3:							//EZS
		
			break;
	}
	
	
	// tohle az nakonec
	_aktdelka=0;
	_novypaket=false;
	
	return 0;

}

int IQRF::SNcheck() {
	
	Serial.print("Exp.SN:");
	Serial.println(_enerval[_aktpaket.channel].expseq);
	Serial.print("Rcv.SN:");
	Serial.println(_aktpaket.seqnr);

	if ((_aktpaket.seqnr == _enerval[_aktpaket.channel].expseq) || (_enerval[_aktpaket.channel].expseq == 0)) {		// prisla ocekavana hodnota SN nebo jsme po restartu a nevime co cekat
		_enerval[_aktpaket.channel].expseq=snplus1(_aktpaket.seqnr);				// SN++
		} else {
		if (_aktpaket.seqnr == 0) {			// SN prichoziho je 0, tj. sonda je po restartu
			_enerval[_aktpaket.channel].expseq =1;		// priste by melo byt SN=1
			
			} else {							// prislo chybne SN;
			Serial.print("Chybne SN. Rozdil:");
			_rozdilsnv=cntrozdilsn(_aktpaket.seqnr,_enerval[_aktpaket.channel].expseq);		// rozdil mezi ocekavanym a prijatym SN
			Serial.println(_rozdilsnv,DEC);
			_enerval[_aktpaket.channel].expseq=snplus1(_aktpaket.seqnr);				// SN++ aktualne prijateho paketu
		}
		
	}	
	
	return 0;
}

int IQRF::CRCcheck() {								// kontrola "CRC"
	
	int i;
	char* aktbytepaket = reinterpret_cast<char *>(&_aktpaket);
	char csum=0;
	
	for (i=0;i<PAKETLEN-1;i++) {
		csum+=aktbytepaket[i];
	}
	
	Serial.print("Spocitane CRC:");
	Serial.println(csum,HEX);
	if (csum != _aktpaket.crc) {
		return -1;
	}
	
	return 0;
}

int IQRF::zpracuj_energo() {
	
	int i;
	
	switch (_aktpaket.ctrl & 0x7f) {
	
		case 0:				// bin
			if (_aktpaket.ctrl & 0x80) {		//umozni nacitani hodnot
		
				if (_aktpaket.channel<MAXENVENTR) {		// aby se nam to veslo do staticky definovaneho pole
					if (_enerval[_aktpaket.channel].value == 0) {
						for (i=0;i<LENCHANNAME;i++) {
							if (_aktpaket.channame[i] != 0x20 ) {				// neni to mezera
								_enerval[_aktpaket.channel].channame[i]=_aktpaket.channame[i];		//prekopiruj jmeno
							} else {
								_enerval[_aktpaket.channel].channame[i]=0;					// misto mezery zapis konec retezce
							}
						}
						_envval[_aktpaket.channel].channame[LENCHANNAME]=0;					// na konec 0
						_enerval[_aktpaket.channel].subtype=_aktpaket.subtype;					// prekopiruj subtype
					}
					Serial.print("Channel name:");
					Serial.print(_enerval[_aktpaket.channel].channame);
					Serial.println("---");
					_enerval[_aktpaket.channel].value=_enerval[_aktpaket.channel].value+_aktpaket.value+_aktpaket.value*_rozdilsnv;				// pripocicej doslou hodnotu value a pripoci opravu za ztracene pakety !!! je otazka, jestli je to dobry napad
					Serial.print("Value:");
					Serial.println(_enerval[_aktpaket.channel].value);
			
				}
		
			} else {
				// hodnotu je treba hned zapsat do databaze - Dodelat
			}
	
			break;
	
		case 1:				// control
			switch (_aktpaket.value) {
		
				case 0:		//keepalive
				Serial.println(_enerval[_aktpaket.channel].channame);	
				Serial.println("Keepalive");
		
				break;
		
				case 1:		//battery - tohle jsem nakonec udelal jinak
		
				break;
		
				case 2:		//1 byte - delka zpravy 1 byte - zatim neimplementovano
		
				break;
		
				case 3:		// reset
		
				break;
			}
			break;
	
		case 2:
	
			break;
	}	
	
	return 0;
}

int IQRF::zpracuj_enviro() {
	
	int i;
	
	switch (_aktpaket.ctrl & 0x7f) {
	
	
		case 3:								// tepl. senzor IQRF
			if (_aktpaket.channel<MAXENVENTR) {		// aby se nam to veslo do staticky definovaneho pole
				if (!_envval[_aktpaket.channel].platna) {
					for (i=0;i<LENCHANNAME;i++) {
						if (_aktpaket.channame[i] != 0x20 ) {				// neni to mezera
							_envval[_aktpaket.channel].channame[i]=_aktpaket.channame[i];		//prekopiruj jmeno
							} else {
							_envval[_aktpaket.channel].channame[i]=0;					// misto mezery zapis konec retezce
						}
					}
					_envval[_aktpaket.channel].channame[LENCHANNAME]=0;					// na konec 0
					_envval[_aktpaket.channel].platna=true;					//polozka pole je platna, obsahuje jmeno
					Serial.print("Enviro channel name:");
					Serial.print(_envval[_aktpaket.channel].channame);
					Serial.println("---");
				}
				_envval[_aktpaket.channel].value=TempIQRFtoFloat(_aktpaket.value,_aktpaket.string[0]);
				_envval[_aktpaket.channel].zapsano=false;
				}
			
			
	}
	
	
	return 0;
	
}

float IQRF::TempIQRFtoFloat(char cela, char deset) {
	
	int znamenko=1;
	float fteplota;
	
	Serial.print("Temp cela:");
	Serial.println(cela,DEC);
	Serial.print("Temp deset:");
	Serial.println(deset,DEC);
	
	if (cela & 0x80) {					// zaporna hodnota teploty                              
		cela  = ~cela;
		cela++;
		znamenko=-1;
	}
	fteplota=znamenko*((float)cela+0.0625*(float)deset);
	Serial.print("Temp vysl:");
//	Serial.println((double)fteplota,2);
	
	return fteplota;
	
}

bool IQRF::acquired() {
	return _novypaket;
}

char IQRF::snplus1(char c) {
	if (c == 255) {
		return 1;
	}
	return ++c;
}

char IQRF::cntrozdilsn(char prislo,char cekam) {
	if (prislo >= cekam) {
		return prislo-cekam;
	}
	return 255-cekam+prislo;
}

char IQRF::writevalmysql() {			// zapis nastradanych hodnot do MySQL databaze
	
	int i;
	float cenazapuls;
	float price=0;
	int chyba = 0;
	
	Serial.println("Zapis nastradanych energo hodnot do MySQL");
	for (i=0;i<MAXENVENTR;i++) {
		if (_enerval[i].value > 0) {
			if ((cenazapuls=MyDatabase.getprice(_enerval[i].subtype,false)) != -1) {
				price=_enerval[i].value*cenazapuls;
				MyDatabase.insertEnervalue((String)_enerval[i].channame,floatToString(_enerval[i].value,0),floatToString(price,4));		// !!! chtelo by to test, jestli se zapis podaril 
			} else {
				Serial.print("Chyba: neni cena za puls pro sondu:");
				Serial.println(i);
				chyba=-1;
			}
			_enerval[i].value=0;			// nastradane hodnoty smazu v kazdem pripade
		}
	}
	
	
	return chyba;
}


char IQRF::writeenvmysql() {
	
	int i;
	char chyba=0;
	
	Serial.println("Zapis nastradanych enviro hodnot do MySQL");
	for (i=0;i<MAXENVENTR;i++) {
		if (_envval[i].platna && !_envval[i].zapsano) {			// platna nezapsana hodnota
			MyDatabase.insertEnvvalue((String)_envval[i].channame,floatToString(_envval[i].value,1));	// ten pocet desetinnych mist budu muset vyresit lepe !!! podle ctrl policka v IQRF paketu?
			_envval[i].platna=false;
			_envval[i].zapsano=true;
		}
	
	}
	
	return chyba;
}

/*
int IQRF::readpacket() {

	// prvni byte paketu (preambule) uz je nacteny drive, ted by mel byt prvni byte delka
		
	bool nacteno=false;		// nacten cely paket
	bool timeout=false;		// timeout pricekani na dalsi byte paketu
	bool lenerr=false;		// nesouhlasi ocekavana delka paketu
	byte polozka=0;
	byte len;
	int i=0;
	int cnt=0;
	struct iqrfpacket rcvpaket;
	
	while (!nacteno && !timeout && !lenerr) {				// dokud se nenacte cely paket nebo nevyprsi "timeout"
		if (Serial1.available()) {
		//	serznak=Serial1.read();
		//	Serial.println(serznak,HEX);
			(byte *)rcvpaket[i++]=Serial1.read();
			cnt=0;
			if ((i == 1) && rcvpaket.len != PAKETLEN) {
				lenerr = true;
			}
			if (i==rcvpaket.len) {
				nacteno = true;
			}
		} else {
			if (cnt++ == MAXCNT) {
				timeout = true;
			}
		}	
	}
	
	
}
*/

