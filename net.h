/*
 * net.h
 *
 * Created: 6.10.2013 11:01:48
 *  Author: hruby
 */ 


#ifndef NET_H_
#define NET_H_

#ifndef _SPI_H_INCLUDED
#include <SPI.h>
#endif


#ifndef ethernet_h
#include <Ethernet.h>
#endif

#ifndef ethernetudp_h
#include <EthernetUdp.h>
#endif


void netinit(void);
unsigned long sendNTPpacket(IPAddress& address);
int getNTPTime(unsigned long *lepoch);

#endif /* NET_H_ */