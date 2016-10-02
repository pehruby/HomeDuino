/*
 * net.cpp
 *
 * Created: 6.10.2013 11:02:13
 *  Author: hruby
 */ 

#ifndef NET_H_
#include "net.h"
#endif

byte mac[] = {
0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };

byte myip[] = {192, 168, 167, 101};
byte mymask[] = {255, 255, 255,0};
// byte dnsip[] = {8,8,8,8};
byte dnsip[] = {93,153,117,1};
byte defgw[] = {192,168,167,1};
// byte server[] = { 173, 194, 35, 179 }; // Google

IPAddress timeServer(217, 31, 202, 100); // ntp.nic.cz NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets



// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;


unsigned int localPort = 8888;      // local port to listen for UDP packets


EthernetClient client;

void netinit(void) {
	
	Ethernet.begin(mac,myip,dnsip,defgw,mymask);
	Udp.begin(localPort);

}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer,NTP_PACKET_SIZE);
	Udp.endPacket();
}

int getNTPTime(unsigned long *lepoch) {
	
	sendNTPpacket(timeServer); // send an NTP packet to a time server
	bool UDPReceived = false;
	unsigned int UDPwaittime =0;

	// wait to see if a reply is available
	// !!!!!!!!!!!!!!Predelat !!!!!!!!!!!!!!!!!!!
//	delay(1000);
	while (!UDPReceived && UDPwaittime < 200) {	
		if ( Udp.parsePacket() ) {
			// We've received a packet, read the data from it
			UDPReceived = true;
			Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

			//the timestamp starts at byte 40 of the received packet and is four bytes,
			// or two words, long. First, esxtract the two words:

			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
			// combine the four bytes (two words) into a long integer
			// this is NTP time (seconds since Jan 1 1900):
			unsigned long secsSince1900 = highWord << 16 | lowWord;
			// Serial.print("Seconds since Jan 1 1900 = " );
			//Serial.println(secsSince1900);

			// now convert NTP time into everyday time:
			// Serial.print("Unix time = ");
			// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
			const unsigned long seventyYears = 2208988800UL;
			// subtract seventy years:
			Serial.println(*lepoch);
			*lepoch = secsSince1900 - seventyYears;
			// print Unix time:
			Serial.println(*lepoch);
		} else {
			delay(5);
			UDPwaittime++;
		}
	

	} //while
	
	if (!UDPReceived) {
		return -1;
	}
	
	return 0;
}
