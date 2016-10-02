/*
 * Main.cpp
 *
 * Created: 10/5/2013 4:21:50 PM
 *  Author: hruby
 */


#ifndef SKETCH_H_
#define SKETCH_H_

#ifndef Arduino_h
#include <Arduino.h>
#endif


#ifndef _SPI_H_INCLUDED
#include <SPI.h>
#endif


#ifndef ethernet_h
#include <Ethernet.h>
#endif

#ifndef ethernetudp_h
#include <EthernetUdp.h>
#endif

#ifndef TwoWire_h
#include <Wire.h>
#endif


#ifndef UTILITY_MACROS_H_
#include "Utility\UtilityMacros.h"
#endif

#ifndef BUILDINFO_H_
#include "Utility/Buildinfo.h"
#endif

#ifndef SENSIRION_PROTOCOL_H
#include "sensirion_protocol.h"
#endif

#ifndef NET_H_
#include "net.h"
#endif

#ifndef TEMPHUMSHT_H_
#include "temphumsht.h"
#endif

#ifndef _Nimbits_h
#include "Nimbits.h"
#endif

#ifndef PString_h
#include "PString.h"
#endif

#ifndef _RTCLIB_H_
#include "RTClib.h"
#endif

#ifndef _Time_h_
#include "time.h"
#endif

#ifndef WSTCHIBO_H_
#include "WSTchibo.h"
#endif

#ifndef MYSQL_H_
#include "mysql.h"
#endif

#ifndef IQRF_H_
#include "iqrf.h"
#endif

#ifndef BMP085_H_
#include "bmp085.h"
#endif

/* LED Output */
const uint8_t LED = 13;

#define SS_SD_CARD   4
#define SS_ETHERNET 10
#define SS_HW		53		// hw ss pin on mega
#define RF_IN		22




void SelectEth();
void SelectSD();
int freeRam();
void setup();
void loop();



#endif /* SKETCH_H_ */