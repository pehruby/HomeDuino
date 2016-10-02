/*********************************************
 * vim:sw=8:ts=8:si:et
 * This is the header file the sensirion temperature and
 * humidity sensor. 
 *
 * Based on ideas from the sensirion application note and modified
 * for atmega88/168. 
 * A major part of the code was optimized and as a result the compiled 
 * code size was reduced to 50%.
 *
 * Modifications by: Guido Socher 
 *
 * Note: the sensirion SHTxx sensor does _not_ use the industry standard
 * I2C. The sensirion protocol looks similar but the start/stop
 * sequence is different. You can not use the avr TWI module.
 *
 *********************************************/
//@{
#ifndef SENSIRION_PROTOCOL_H
#define SENSIRION_PROTOCOL_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <avr/io.h>
// #include "board.h"
// je definov�no v board#define F_CPU 12500000UL  // 12.5 MHz
#include <util/delay.h>
// #include "ethmodmeg8.h"

//#ifndef _PODM_PREKL

#define	_FCE_CIDLO_VLHKOSTI
#define	_FCE_ZISKANI_HODNOT		//jeden se mus� vybrat - jestli se bude ��st sekven�n� a zblokuje to na 0,5s
// #define	_FCE_ZISKANI_HODNOT_STAUT	//nebo stavov�m automatem formou testov�n� ukon�en� p�evodu

//#else
//#include "zavislosti.h"
//#endif /* _PODM_PREKL */


/*_____ M A C R O S ________________________________________________________*/
#define CHYBA_START_KOM 0x10
#define CHYBA_TIMEOUT 0x20
#define CHYBA_CRC 0x40
#define	HUMI_TIMEOUT 7		//pokud se bude volat fce ka�d�ch 100ms, bude timeout 600ms

#define MEASURE_TEMP 0x03 //000 0001 1
#define MEASURE_HUMI 0x05 //000 0010 1
#define	INIT_HUM 0x55		//aby se v prvn�m cyklu jen spustilo m��en� a neprov�d�lo se vyhodnocen� 

/*_____ D E F I N I T I O N ________________________________________________*/
extern	unsigned int gHumival_Raw, gTempval_Raw;



// note: hardware connection definitions are in sensirion_protocol.c at
// the beginning of the inensirion_protocol.c file. You need to change
// the coded there if you connect the sensor to other pins.
//

/*_____ D E C L A R A T I O N ______________________________________________*/
extern void s_connectionreset(void);
extern char s_softreset(void);
extern char s_measure(unsigned int *p_value, unsigned char mode);
extern int calc_sth11_temp(unsigned int t);
extern unsigned int rhcalc_int(unsigned int s);
extern unsigned int calc_sth11_humi(unsigned int h, int t);
extern int calc_dewpoint(unsigned int rh,int t);
extern int log10_approx(unsigned char x);
/* debug
extern void s_transstart(void);
extern char s_write_byte(unsigned char value);
extern unsigned char computeCRC8(unsigned char inData, unsigned char seed);
extern unsigned char bitswapbyte(unsigned char byte);
extern unsigned char s_read_byte(unsigned char ack);*/
extern void	ziskat_vlhkost(void);	//z�sk� hodnoty z �idla, z funkce se vysko�� po dokon�en� p�evodu + upraven� v�sledk�
extern void	ziskat_vlhkost_st_automat(void);	//stavov� automat - zah�jen� p�evod, v dal��m cyklu zpracuje a znovu zah�j� p�evod
#endif /* SENSIRION_PROTOCOL_H */
//@}
