﻿/*
 * Copyright (c) 2013 Nimbits Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either expressed or implied.  See the License for the specific language governing permissions and limitations under the License.
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
#include "Nimbits.h"
#define BUFFER_SIZE 1024
#define PORT 80
#define CATEGORY 2
#define POINT 1
#define SUBSCRIPTION 5

// const char *GOOGLE = "google.com";
const char *GOOGLE = "phnimb.appspot.com";
byte googleip[] = {173,194,70,141};


const String CLIENT_TYPE_PARAM="&client=arduino";
const String APP_SPOT_DOMAIN = ".appspot.com";
const String PROTOCAL = "HTTP/1.1";
const int WAIT_TIME = 1000;
const char quote = '\"';


String _ownerEmail;
String _instance;
String _accessKey;

Nimbits::Nimbits(String instance, String ownerEmail, String accessKey){
  _instance = instance;
  _ownerEmail = ownerEmail;
  _accessKey = accessKey;
  
  
  /*
  if (instance.length()+APP_SPOT_DOMAIN.length() < MAXURL) {
	  instance.toCharArray(nURL,MAXURL);
	  APP_SPOT_DOMAIN.toCharArray(nURL[instance.length()],MAXURL);
  }
  
  Serial.println(nURL); */


}


String createSimpleJason(char *name, char *parent, int entityType) {


}
String floatToString(double number, uint8_t digits) 
{ 
  String resultString = "";
  // Handle negative numbers
  if (number < 0.0)
  {
    resultString += "-";
    number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;

  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  resultString += int_part;

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    resultString += "."; 

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    resultString += toPrint;
    remainder -= toPrint; 
  } 
  return resultString;
}

void Nimbits::recordtest(char *packet) {
	
	EthernetClient client;
//	char *packet = "Test";
	int tcprespdel=0;
	int rcvchar=0;
	
	if (client.connect(googleip,PORT)) {
		client.print(packet);
		Serial.print(packet);					// debug
		Serial.println("Packet sent");
		
		while(client.connected() && !client.available()) {		//tcp spojeni trva && nejsou k dispozici data ke cteni
			delay(1);
			if (++tcprespdel > MAXTIME) {
				client.stop();									// dlouho neprichazi odpoved, tak spojeni vyresetuju
				Serial.println("Server doesn't respond ... close connection");
			}
		}
		
		Serial.print("Server response (~ms):");
		Serial.println(tcprespdel);
		while (client.available() && (rcvchar++ < 10000) ) {							// dokud jsou na vstupu data ke cteni. Pokud je jich moc, je treba to vcas zarazit
			char c = client.read();
			Serial.print(c);

		}
		Serial.println("\r\n");
		client.flush();
		client.stop();
	} else {
		Serial.println("Unable to connect to the server");
	}
	
}
void Nimbits::recordValue(double value, int decpoint, String note, String pointId) {
  EthernetClient client;

  String json;
  int tcprespdel=0;
  int rcvchar=0;
  #define MAXDEL 500
  char pbuffer[MAXDEL];
  PString paketstr(pbuffer,sizeof(pbuffer));
  
  json =  "{\"d\":\"";
  json +=floatToString(value, decpoint);

  json += "\",\"n\":\""; 
  json +=  note; 
  json +=  "\"}"; 
  String content;
  content = "email=";

  content += _ownerEmail;
  content += "&key=";
  content += _accessKey;
  content += "&json=";
  content += json;
  content += "&id=";
  content +=  pointId;

  Serial.println(content);
  Serial.print("Content length:");
  Serial.println(content.length());

	paketstr.begin();
	paketstr.println("POST /service/v2/value HTTP/1.1");
	paketstr.println("Host: phnimb.appspot.com");
	paketstr.println("Connection: close");
	paketstr.println("User-Agent: Arduino/1.0");
	paketstr.println("Cache-Control: max-age=0");
	paketstr.println("Content-Type: application/x-www-form-urlencoded");
	paketstr.print("Content-Length: ");
	paketstr.println(content.length());
	paketstr.println();
	paketstr.println(content);
  if (client.connect(GOOGLE,PORT)) {
//	if (client.connect(googleip,PORT)) { 
		client.print(paketstr);
		Serial.print(paketstr);					// debug
		Serial.println("Packet sent");
	
		while(client.connected() && !client.available()) {		//tcp spojeni trva && nejsou k dispozici data ke cteni
			delay(1);
			if (++tcprespdel > MAXTIME) {
				client.stop();									// dlouho neprichazi odpoved, tak spojeni vyresetuju
				Serial.println("Server doesn't respond ... close connection");
			}
		}
	
		Serial.print("Server response (~ms):");
		Serial.println(tcprespdel);
		while (client.available()  && (rcvchar++ < 10000)) {							// dokud jsou na vstupu data ke cteni. Pokud je jich moc, je treba to vcas zarazit
			 char c = client.read();
			 Serial.print(c);

		}
		Serial.println("\r\n");
		client.flush();
		client.stop();
  } else {
	  Serial.println("Unable to connect to the server");
  }

}

void Nimbits::addEntityIfMissing(char *key, char *name, char *parent, int entityType, char *settings) {
  EthernetClient client;
  Serial.println("adding");
  String retStr;
  char c;
  // String json;
  String json;
  json =  "{\"name\":\"";
  json += name; 
  json.concat("\",\"description\":\""); 
  json +=   "na"; 
  json += "\",\"entityType\":\""; 
  json +=  String(entityType); 
  json +=  "\",\"parent\":\""; 
  json +=   parent; 
  json += "\",\"owner\":\""; 
  json +=  _ownerEmail;
  json +=  String("\",\"protectionLevel\":\"");
  json +=   "2";
  
  //return json;
  switch (entityType) {
  case 1: 
    // json = createSimpleJason(name, parent, entityType); 

    break;
  case 2: 
    // json = createSimpleJason(name, parent, entityType); 
    break;
  case 5: 
    json +=  "\",\"subscribedEntity\":\""; 
    json +=   parent; 
    json +=  "\",\"notifyMethod\":\""; 
    json +=   "0"; 
    json +=  "\",\"subscriptionType\":\""; 
    json +=   "5"; 
    json +=  "\",\"maxRepeat\":\""; 
    json +=   "30"; 
    json +=  "\",\"enabled\":\""; 
    json +=   "true"; 

    break;
  }
  json += settings;
  json +=  "\"}";
  Serial.println(json);
  String content;
  content = "email=";

  content += _ownerEmail;
  content += "&key=";
  content += _accessKey;
  content += "&json=";
  content += json;
  content += "&action=";
  content += "createmissing";
  Serial.println(content);
  if (client.connect(GOOGLE, PORT)) {
    client.println("POST /service/v2/entity HTTP/1.1");
    client.println("Host:nimbits-02.appspot.com");
    client.println("Connection:close");
    client.println("User-Agent: Arduino/1.0");
    client.println("Cache-Control:max-age=0");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(content.length());
    client.println();
    client.println(content);

    while(client.connected() && !client.available())		//tcp spojeni trva (resp. jsou k dispozici data) && nejsou k dispozici data ke cteni
		delay(1);		
    int contentLength = 0;
    char buffer[BUFFER_SIZE];


    while (client.available() && contentLength++ < BUFFER_SIZE) {
      c = client.read();
      Serial.print(c);
      buffer[contentLength] = c;
    }
    Serial.println("getKeyFromJson");
    Serial.println(sizeof(buffer));
    int i=0;
    char item[] = {
      "\"key\":"          };
    while (i++ < sizeof(buffer) - sizeof(item)) {
      boolean found = false;
      found = false;
      for (int v = 0; v < sizeof(item) -1; v++) {

        if (buffer[i+v] != item[v]) { 
          found = false;
          break;
        }
        found = true;
      }
      if (found) {
        break;
      }


    }

    i = i + sizeof(item)-1;
    int keySize = 0;
    while (i++ < sizeof(buffer)-1) {
      if (buffer[i] == quote) {
        break;
      }
      else {
        key[keySize++] = buffer[i];
      }
    }
    key[keySize] = '\0';
    Serial.println(key);



    client.stop();
  }
  else {
    Serial.println("could not connect");

  }
  delay(1000);
}


//String Nimbits::getTime() {
//  EthernetClient client;

// if (client.connect(GOOGLE, PORT)) {
//  client.print("GET /service/v2/time?");
//writeAuthParamsToClient(client);
//writeHostToClient(client);

// return getResponse(client);


//}




//record a value

//record data

//create point

//create point with parent

//batch update

//delete point

//get children with values


