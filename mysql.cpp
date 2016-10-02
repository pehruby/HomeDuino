/*
 * mysql.cpp
 *
 * Created: 25.11.2013 20:04:57
 *  Author: hruby
 */ 

#include <Arduino.h>
#include <Dhcp.h>
#include <Dns.h>
#include <EthernetServer.h>
#include <util.h>
#include <EthernetUdp.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <SPI.h>
#include <stdlib.h>
#include <stdio.h>
#include "MySQL.h"

#define PORT 80

#define ENVPHP "homeenvzapis.php"
#define ENERPHP "homeenerzapis.php"
#define CENREAD "homeenerpriceread.php"



String _StartEnvURL;
String _StartEnerURL;
String _Password;
byte *_Host;

MySQL::MySQL(byte *Host, String StartURL, String Pass) {
	int i;
	
	// heslo: x
	_StartURL = StartURL;		// Pocatek URL pro zapis hodnot
	_StartEnerURL= StartURL + String(ENERPHP);
	_StartEnvURL= StartURL + String(ENVPHP);
	_StartPriceURL = StartURL +String(CENREAD);
	_Password = Pass;			// heslo
	_Host = Host;				// IP adresa MySQL serveru
	
	for (i=0;i<MAXPROBETYPENR;i++) {				// inicializace cen za pulz, -1 je nedefinovan8 hodnota
		_pricepulse[i]=-1;
	}
	
}
void MySQL::insertEnvvalue(String Name, String Value ) {				// do tabulky enviro hodnot se vklada jen hodnota (teplota, vlhkost, sila vetru ...)
	
	String URL;											// URL na PHP skript
	String vystup="";
/*	EthernetClient client;
	int tcprespdel=0;
	int rcvchar=0; */
	
	URL = _StartEnvURL + String("?Nazev=") + Name + String("&Hodnota=") + Value + String("&Heslo=") + _Password + String(" HTTP/1.1");
	_phpURL(URL,&vystup);
	Serial.print("Insert vystup:");
	Serial.println(vystup);
	
	
	
}

void MySQL::insertEnervalue(String Name, String Value, String Price ) {				// Do tabulky energii se vklada hodnota (m3, W, ..) a cena
	
	String URL;													// URL na PHP skript
	String vystup="";

	
	URL = _StartEnerURL + String("?Nazev=") + Name + String("&Hodnota=") + Value + String("&Cena=") + Price + String("&Heslo=") + _Password + String(" HTTP/1.1");
	_phpURL(URL,&vystup);
		
	
}

void MySQL::_phpURL(String URL, String *vystup) {
	
	EthernetClient client;
	int tcprespdel=0;
	int rcvchar=0;
	String HTTPHostname="Host: 192.168.167.101";
	
	if (client.connect(_Host,PORT)) {
		client.println(URL);
		client.println(HTTPHostname);
		client.println();
		client.println();
		Serial.println(URL);					// debug
		Serial.println(HTTPHostname);
		Serial.println("MySQL Packet sent");
		
		while(client.connected() && !client.available()) {		//tcp spojeni trva && nejsou k dispozici data ke cteni
			delay(1);
			if (++tcprespdel > MAXTIME) {
				client.stop();									// dlouho neprichazi odpoved, tak spojeni vyresetuju
				Serial.println("Server doesn't respond ... close connection");
			}
		}
		
		Serial.print("Server response (~ms):");
		Serial.println(tcprespdel);
		while (client.available() && (rcvchar++ < 300) ) {							// dokud jsou na vstupu data ke cteni. Pokud je jich moc, je treba to vcas zarazit
			char c = client.read();
			*vystup=*vystup+String(c);
			Serial.print(c);

		}
		Serial.println("\r\n");
		Serial.println(*vystup);
		client.flush();
		client.stop();
		} else {
		Serial.println("Unable to connect to the server");
	}
}

boolean MySQL::naplncenik(int probetype) {					//naplni polozku v enerprice pro danou probetype (IQRF subtype)
	
	#define _MAXTMPSTR 300
	// implementovat dotaz do MySQL databaze pres PHP
	String URL;
	String vystup="";
	
	float vysl1,vysl2;
	char tmpstr[_MAXTMPSTR];
	int i;
	
	URL = _StartPriceURL + String("?Probetype=") + probetype + String("&Column=priceunit") + String("&Heslo=") + _Password + String(" HTTP/1.1");
	_phpURL(URL,&vystup);
	
	if (vystup.charAt(0) == '1') {
		for (i=0;i<(vystup.length()-2) && i<_MAXTMPSTR;i++) {			// && i<_MAXTMPSTR
			tmpstr[i]=vystup.charAt(i+2);
		}
		tmpstr[i]=0;
	}
	vysl1=atof(tmpstr); 

    Serial.print("Priceunitstr:");
    Serial.println(tmpstr);

	vystup="";
	
	URL = _StartPriceURL + String("?Probetype=") + probetype + String("&Column=pulseperunit") + String("&Heslo=") + _Password + String(" HTTP/1.1");
	_phpURL(URL,&vystup);

	if (vystup.charAt(0) == '1') {
		for (i=0;i<(vystup.length()-2) && i<_MAXTMPSTR;i++) {			// && i<_MAXTMPSTR
			tmpstr[i]=vystup.charAt(i+2);
		}
		tmpstr[i]=0;
	}
	vysl2=atof(tmpstr);

    Serial.print("Pulseperunitstr:");
    Serial.println(tmpstr);
	
		
	_pricepulse[probetype]=vysl1/vysl2;
	
/*	Serial.print("Pricepulse:");
	Serial.println(_pricepulse[probetype],4); */
	
	
}

float MySQL::getprice(int probetype,boolean force) {			// vrati cenu za impuls pro danou probetype, dam-li force, vynutim update z MySQL databaze
	
	if (force || (_pricepulse[probetype] = -1)) {
		naplncenik(probetype);
	}
	return _pricepulse[probetype];
	
}
