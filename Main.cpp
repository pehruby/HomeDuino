/*
 * Main.cpp
 *
 * Created: 10/5/2013 4:21:50 PM
 *  Author: hruby
 */



#ifndef SKETCH_H_
#include "Sketch.h"
#endif

String floatToString(double number, uint8_t digits);

String instance = "phnimb";
char owner[] = "petr.hruby@gmail.com";
String readWriteKey = "testkey";

static volatile unsigned char tsec=0, tmin=0, thod=0,tday=0;		// citani po sekundach TIMER1A
static volatile uint16_t waitnextsec=0,t50us=0,interval;
static volatile byte lastlevel, got_interval=0;

// String StartURLwr = "GET http://192.168.167.5/mericidum/zapis.php/";
//String StartMyEnvURLwr = "GET http://192.168.167.5/mericidum/homeenvzapis.php/";
//String StartMyEnerURLwr = "GET http://192.168.167.5/mericidum/homeenerzapis.php/";
String StartMyURLwr = "GET http://192.168.167.5/mericidum/";
byte sqlip[] = {192, 168, 167, 5};


// MySQL Databaze(sqlip,StartURLwr,"x");
MySQL MyDatabase(sqlip,StartMyURLwr,"x");
// MySQL MyEnerDatabase(sqlip,StartMyEnerURLwr,"x");

unsigned long epoch=0;

RTC_DS1307 RTC;

Nimbits mynimbit(instance, owner, readWriteKey);

Temp_Hum_SHT cidloSHT;

DateTime dtActual;

WSTchibo cidloTchibo;

IQRF cidloIQRF;

bmp085 Tlak;




int freeRam() {
	extern int __heap_start,*__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int) __brkval);
}


ISR(TIMER1_COMPA_vect)
{
	static byte count=0,was_hi=0;
	
	
	// "cron" 
	t50us++;
	if (t50us == 20000) {				//ubehla 1s (50x20000us)
	
		tsec++;
		if (tsec == 60) {
			tmin++;
			tsec=0;
			if (tmin == 60) {
				thod++;
				tmin=0;
				if (thod==24) {
					thod=0;
				}
			}
		}
		waitnextsec=0;
		t50us=0;
	}
	
	
	// obsluha 433Mhz prijimace
	
	if (digitalRead(RF_IN) == HIGH) {		//nyni je stav 1
		if (was_hi) {						// v minulem cyklu byl stav 1
			count++;
		} else{								// v minulem cyklu byl stav 0
			interval=count;
			got_interval=1;
			lastlevel=0;
			count=0;
			was_hi=1;						// info pro pristi cyklus, ze byl stav 1
		}
		
	} else {								// nyni je stav 0
		if (was_hi) {						// v minulem cyklu byl stav 1
			interval=count;
			got_interval=1;
			lastlevel=1;
			count=0;
			was_hi=0;						// info pro pristi cyklus, ze byl stav 0
		} else {
			count++;
		}
		
	}
	
}


void SelectEth() {
	// deselect SD, pin 4 output, HIGH
	pinMode(SS_SD_CARD,OUTPUT);
	digitalWrite(SS_SD_CARD,HIGH);
	
	digitalWrite(SS_ETHERNET, LOW);
	
}

void SelectSD() {
	
	pinMode(SS_ETHERNET,OUTPUT);
	digitalWrite(SS_ETHERNET,HIGH);
	
	digitalWrite(SS_SD_CARD,LOW);
	
}

 void setup()
 {
	Serial.begin(115200);
	Serial1.begin(9600);
	pinMode(SS_HW,OUTPUT);		//pin 53 on Mega - output
	pinMode(RF_IN,INPUT);
	pinMode(LED,OUTPUT);
	SelectEth();
	netinit();

	Wire.begin();				// I2C init
	
	RTC.begin();
	
	if (! RTC.isrunning()) {
		Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		//RTC.adjust(DateTime(__DATE__, __TIME__));
		getNTPTime(&epoch);				// inicializace RTC pomoci NTP
		if (epoch>0) {
			RTC.adjust(epoch);
			Serial.println("RTC aktualizovano pomoci NTP");
		}
	}
	Tlak.Calibration();
	Serial.println("Setup2");
	
	MyDatabase.getprice(1,false);
		
	TCCR1A=0;
	TCCR1B=0;				// clear TCCR1

	/* nastaveni ridiciho registru B pro citac 1 */
//	TCCR1B |= (1 << CS10);	TCCR1B |= (1 << CS12); // prescaler 1024
//	TCCR1B |= (1 << CS12);	// prescaler 256
	TCCR1B |= (1 << CS11);	// prescaler 8
	TCCR1B |= (1 << WGM12);	// CTC mode

//	OCR1A=15650;			// output compare register A pro citac 1 na 1000ms pri prescaler 1024
//	OCR1A=62500;			// OCR A pro citac 1 na 1000ms pri prescaler 256
//	OCR1A=3;				// OCR A pro citac 1 na 50us pri prescaler 256
	OCR1A=100;				// OCR A pro citac 1 na 50us pri prescaler 8
	TIMSK1 |= (1 << OCIE1A);		// timer 1A output compare interrupt enable
//	TIMSK1 |= (1 << OCIE1B);		// timer 1B output compare interrupt enable

	sei();		// enable interrupt
 }

 void loop()
 {
	 byte *paket;
	 byte channel=0;				//na cidlu je to Ch1,Ch2,Ch3 - zde 0,1,2
	 char serznak;
	 int i;
	 int t;
	 String hodnota;
	 float akttlak;
	 
	 
	if ((tsec == 30) && (tmin == 0) && ! waitnextsec) {
		if (getNTPTime(&epoch) == 0) {
			RTC.adjust(epoch);
			Serial.println("Synchronizovan cas pres NTP");
		} else {
			Serial.println("Synchronizace pres NTP se nezdarila");
		}
		
		
		
		// Databaze.insertEnvvalue("Test","2");
		waitnextsec++;

	} 
	if ((tsec == 30) && (tmin%3 == 1) && !waitnextsec) {
		cidloIQRF.writevalmysql();		// zapis nastradane energo hodnoty do MySQL
		cidloIQRF.writeenvmysql();		// zapis nastradane enviro hodnoty
	}
	
//	 char *TestStr="POST /service/v2/value HTTP/1.1\r\nHost: phnimb.appspot.com\r\nConnection: close\r\nUser-Agent: Arduino/1.0\r\nCache-Control: max-age=0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 94\r\n\r\nemail=petr.hruby@gmail.com&key=testkey&json={\"d\":\"25.2\",\"n\":\"\"}&id=petr.hruby@gmail.com/DPTemp\r\n";
	if ((tsec == 55) && (tmin%12 ==0) && !waitnextsec) {
//	if (tsec == 0) {	
//		mynimbit.recordtest(TestStr);
		 // getNTPTime(&epoch);
		 // Serial.println("Test");
		 // Serial.println(epoch);
	 	 Serial.println("===============New measurement============");
		 cidloSHT.TGetSHTValues();											// Co kdyz se nic nenacte ?  !!!!!
		 cidloSHT.PrintConsoleAll();
		 Serial.println("============== 1st packet==temp===========");
		 Serial.print("Free RAM:");
		 Serial.println(freeRam());
		 mynimbit.recordValue((float)cidloSHT.temp/10,1,"","petr.hruby@gmail.com/DPTemp");
		 MyDatabase.insertEnvvalue("MegaTemp",floatToString((float)cidloSHT.temp/10,1));
		 mynimbit.recordValue((float)cidloSHT.rh/10,1,"","petr.hruby@gmail.com/DPHumi");
		 MyDatabase.insertEnvvalue("MegaHumi",floatToString((float)cidloSHT.rh/10,1));
		 mynimbit.recordValue((float)cidloSHT.dew/10,1,"","petr.hruby@gmail.com/DPDewPoint");
		 MyDatabase.insertEnvvalue("MegaDew",floatToString((float)cidloSHT.dew/10,1));
		 if (cidloTchibo.acquired(channel)) {
			mynimbit.recordValue((float)cidloTchibo.get_temp(channel)/10,1,"","petr.hruby@gmail.com/DPTempOut");
			MyDatabase.insertEnvvalue("OutTemp",floatToString((float)cidloTchibo.get_temp(channel)/10,1));
			mynimbit.recordValue(cidloTchibo.get_humidity(channel),0,"","petr.hruby@gmail.com/DPHumiOut");
			MyDatabase.insertEnvvalue("OutHumi",floatToString(cidloTchibo.get_humidity(channel),0));
			if (cidloTchibo.lowbatt(channel)) {
				digitalWrite(LED,HIGH);
				Serial.println("Battery low");
				} else {
				digitalWrite(LED,LOW);
				Serial.println("Battery OK");
			}	
		 }
		 
		 // Tlak z BMP-085
		 akttlak=(float)Tlak.GetPressureFin();
		 akttlak/=100;
		 akttlak+=KOMPENZACE_VYSKY;
		 MyDatabase.insertEnvvalue("Press",floatToString(akttlak,1));
		 
		 
//		 mynimbit.recordValue((float)cidloSHT.dew/10,1,"","petr.hruby@gmail.com/DPDewPoint");
//		 if (cidloTchibo.acquired())
//		 if ((t=cidloTchibo.get_temp()) != -9999)
//			mynimbit.recordValue((float)cidloTchibo.get_temp()/10,1,"","petr.hruby@gmail.com/DPTempOut");
		 waitnextsec++;
		 
		 
	}
/*	if ((tsec == 30) && (tmin%12 ==0) && !waitnextsec) {
		//	if (tsec == 0) {
		//		mynimbit.recordtest(TestStr);
		// getNTPTime(&epoch);
		// Serial.println("Test");
		// Serial.println(epoch);
		Serial.println("===============New measurement============");
		cidloSHT.TGetSHTValues();
		cidloSHT.PrintConsoleAll();
		mynimbit.recordValue((float)cidloSHT.dew/10,1,"","petr.hruby@gmail.com/DPDewPoint");
		if (cidloTchibo.acquired(channel))
			mynimbit.recordValue((float)cidloTchibo.get_temp(channel)/10,1,"","petr.hruby@gmail.com/DPTempOut");
			mynimbit.recordValue(cidloTchibo.get_humidity(channel),0,"","petr.hruby@gmail.com/DPHumiOut");
			if (cidloTchibo.lowbatt(channel)) {
				digitalWrite(LED,HIGH);
			} else {
				digitalWrite(LED,LOW);
			}
		waitnextsec++;
		
		
	} */
	if (tsec%10 == 0 && !waitnextsec) {			// to waitnextsec muze byt problematicke
		dtActual=RTC.now();
		dtActual.ConsolePrint();
		waitnextsec++;
		
	}

	if (got_interval) {							// byl dokoncen prijem jedne "urovne", tj. prave se zmenila uroven prijateho signalu. Zpracujeme predchozi uroven vcetne jeji delky v 50us jednotkach
		cidloTchibo.accept(interval,lastlevel);	
		got_interval=0;
	}	 
	
/*	if (cidloTchibo.acquired()) {
		paket=cidloTchibo.getpacet();
		for(i=0;i<6;i++) {
			Serial.print("0x");
			Serial.print(paket[i], HEX);
			Serial.print("/");
			Serial.print(paket[i], DEC);
			Serial.print(" ");
		}
		Serial.print("Humidity:");
		Serial.print(cidloTchibo.get_humidity());
		Serial.print(" Raw temperature:");
		Serial.println(cidloTchibo.get_temp_raw());
		Serial.print(" Real temperature:");
		t=cidloTchibo.get_temp();
		Serial.print(t);	
		Serial.println();
	}
*/
	// zpracovani bytu/paketu z IQRF
	if (Serial1.available()) {			
		serznak=Serial1.read();
		// Serial.println(serznak,HEX);
		cidloIQRF.readbyte(serznak);
	}
	if (cidloIQRF.acquired()) {
		cidloIQRF.zpracujpaket();
	}

 }