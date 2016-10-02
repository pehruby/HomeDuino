/*********************************************
 * vim:sw=8:ts=8:si:et upraveno promùj pøípravek
 * This is the driver code for the sensirion temperature and
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
/*_____ I N C L U D E S ____________________________________________________*/
#include <avr/io.h>
#include "sensirion_protocol.h"
#include <avr/pgmspace.h>



/*_____ M A C R O S ________________________________________________________*/
//adr command r/w
#define STATUS_REG_W 0x06 //000 0011 0
#define STATUS_REG_R 0x07 //000 0011 1

#define RESET 0x1e        //000 1111 0

// physical connection: Arduino PIN 33(DATA),34(SCK)
#define SETSCK1 PORTC|=(1<<PC3)
#define SETSCK0 PORTC&=~(1<<PC3)
#define SCKOUTP DDRC|=(1<<DDC3)
//
#define SETDAT1 PORTC|=(1<<PC4)
#define SETDAT0 PORTC&=~(1<<PC4)
#define GETDATA (PINC&(1<<PINC4))
//
#define DMODEIN DDRC&=~(1<<DDC4)
#define PULLUP1 PORTC|=(1<<PINC4)
#define DMODEOU DDRC|=(1<<DDC4)

//pulswith long
#define S_PULSLONG _delay_us(3.0)
#define S_PULSSHORT _delay_us(1.0)


/*_____ D E F I N I T I O N ________________________________________________*/


unsigned int gHumival_Raw,gTempval_Raw;


//const int logconst[] PROGMEM = {9950,65535-2873+1,0, 3300,78,102, 1850,21,161, 1230,202,202, 860,16,239, 605,200,273, 855,355,308, 615,373,341};	//k,q,q2
const int logconst[] PROGMEM = {995,65536-2873,1, 330,78,102, 185,21,161, 123,202,202, 86,16,239, 121,400,273, 43,101,308, 31,248,340};	//k,q,q2
// Compute the CRC8 value of a data set.
//
//  This function will compute the CRC8 of inData using seed
//  as inital value for the CRC.
//
//  This function was copied from Atmel avr318 example files.
//  It is more suitable for microcontroller than the code example
//  in the sensirion CRC application note.
//
//  inData  One byte of data to compute CRC from.
//
//  seed    The starting value of the CRC.
//
//  return The CRC8 of inData with seed as initial value.
//
//  note   Setting seed to 0 computes the crc8 of the inData.
//
//  note   Constantly passing the return value of this function 
//         As the seed argument computes the CRC8 value of a
//         longer string of data.
//

/*_____ D E C L A R A T I O N ______________________________________________*/


#ifdef _FCE_CIDLO_VLHKOSTI	 

unsigned char computeCRC8(unsigned char inData, unsigned char seed)
{
    unsigned char bitsLeft;
    unsigned char tmp;

    for (bitsLeft = 8; bitsLeft > 0; bitsLeft--)
    {
        tmp = ((seed ^ inData) & 0x01);
        if (tmp == 0)
        {
            seed >>= 1;
        }
        else
        {
            seed ^= 0x18;
            seed >>= 1;
            seed |= 0x80;
        }
        inData >>= 1;
    }
    return seed;    
}

// sensirion has implemented the CRC the wrong way round. We
// need to swap everything.
// bit-swap a byte (bit7->bit0, bit6->bit1 ...)
unsigned char bitswapbyte(unsigned char byte)
{
        unsigned char i=8;
        unsigned char result=0;
        while(i){ 
		result=(result<<1);
                if (1 & byte) {
			result=result | 1;
                }
		i--;
		byte=(byte>>1);
        }
	return(result);
}

// writes a byte on the Sensibus and checks the acknowledge
char s_write_byte(unsigned char value)
{
        unsigned char i=9;
        unsigned char error=0;
        DMODEOU; 
        while(--i){ //shift bit for masking,
                if (0x80 & value) {
                        SETDAT1; //masking value with i , write to SENSI-BUS
                }else{ 
                        SETDAT0;
                }
                SETSCK1; //clk for SENSI-BUS
                S_PULSLONG;
                SETSCK0;
                S_PULSSHORT;
				value=(value<<1);		//pùvodnì se rotovalo i z masky 0x80, ale takhle to vyjde o 10B lépe
//				i=(i>>1);
        }
        DMODEIN; //release DATA-line
        PULLUP1;
        SETSCK1; //clk #9 for ack
        S_PULSLONG;
        if (GETDATA){ //check ack (DATA will be pulled down by SHT11)
                error=1;
        }
        S_PULSSHORT;
        SETSCK0;
        return(error); //error=1 in case of no acknowledge
}

// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
// reversebits=1 caused the bits to be reversed (bit0=bit7, bit1=bit6,...)
unsigned char s_read_byte(unsigned char ack)
{
        unsigned char i=9;
        unsigned char val=0;
        DMODEIN; //release DATA-line
        PULLUP1;
        while(--i){ //shift bit for masking
                SETSCK1; //clk for SENSI-BUS
                S_PULSSHORT;
              val=(val<<1);		//pùvodnì se rotovalo i z masky 0x80, ale takhle to vyjde o 10B lépe
                if (GETDATA){
                        val=(val | 0x01); //read bit	//nejdøím se ète msb, postupnì se to tam dorotuje
                }
                SETSCK0;
                S_PULSSHORT;
//				i=(i>>1); 
        }
        DMODEOU; 
        if (ack){
                //in case of "ack==1" pull down DATA-Line
                SETDAT0;
        }else{
                SETDAT1;
        }
        SETSCK1; //clk #9 for ack
        S_PULSLONG;
        SETSCK0;
        S_PULSSHORT;
        DMODEIN; //release DATA-line
        PULLUP1;
        return (val);
}

// generates a sensirion specific transmission start
// This is the point where sensirion is not I2C standard conform and the
// main reason why the AVR TWI hardware support can not be used.
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
void s_transstart(void)
{
        //Initial state
        SCKOUTP;
        SETSCK0;
        DMODEOU; 
        SETDAT1;
        //
        S_PULSSHORT;
        SETSCK1;
        S_PULSSHORT;
        SETDAT0;
        S_PULSSHORT;
        SETSCK0;
        S_PULSLONG;
        SETSCK1;
        S_PULSSHORT;
        SETDAT1;
        S_PULSSHORT;
        SETSCK0;
        S_PULSSHORT;
        //
        DMODEIN; //release DATA-line
        PULLUP1;
}

// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//      _____________________________________________________         ________
// DATA:                                                     |_______|
//          _    _    _    _    _    _    _    _    _        ___    ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|  |___|   |______
void s_connectionreset(void)
{
        unsigned char i;
        //Initial state
        SCKOUTP;
        SETSCK0;
        DMODEOU; 
        SETDAT1;
        for(i=0;i<9;i++){ //9 SCK cycles
                SETSCK1;
                S_PULSLONG;
                SETSCK0;
                S_PULSLONG;
        }
        s_transstart(); //transmission start
}

// resets the sensor by a softreset
char s_softreset(void)
{
        s_connectionreset(); //reset communication
        //send RESET-command to sensor:
        return (s_write_byte(RESET)); //return=1 in case of no response form the sensor
}
#endif /* _FCE_CIDLO_VLHKOSTI */

#ifdef _FCE_ZISKANI_HODNOT
// makes a measurement (humidity/temperature) with checksum
// p_value returns 2 bytes
// mode: 1=humidity  0=temperature
// return value: 1=write error, 2=crc error, 3=timeout
//
char s_measure(unsigned int *p_value, unsigned char mode)
{
        unsigned char i=0;
        unsigned char msb,lsb;
        unsigned char checksum;
        // the crc8 is computed over the entire communication from command to response data
        unsigned char crc_state=0; 
        *p_value=0;
        s_transstart(); //transmission start
        if(mode){
                mode=MEASURE_HUMI;
        }else{
                mode=MEASURE_TEMP;
        }
        if (s_write_byte(mode)){
                return(1);
        }
        crc_state=computeCRC8(bitswapbyte(mode),crc_state);
        // normal delays: temp i=70, humi i=20
        while(i<240){
                _delay_ms(3.0);
                if (GETDATA==0){
                        i=0;
                        break;
                }
                i++;
        }
        if(i){
                // or timeout 
                return(3);
        }
        msb=s_read_byte(1); //read the first byte (MSB)
        crc_state=computeCRC8(bitswapbyte(msb),crc_state);
        lsb=s_read_byte(1); //read the second byte (LSB)
        *p_value=(msb<<8)|(lsb);
        crc_state=computeCRC8(bitswapbyte(lsb),crc_state);
        checksum =s_read_byte(0); //read checksum
        if (crc_state != checksum ) {
                return(2);
        }
        return(0);
}
#endif /* _FCE_ZISKANI_HODNOT */

#ifdef _FCE_CIDLO_VLHKOSTI
// calculates temperature [C] 
// input : temp [Ticks] (14 bit)
// output: temp [C] times 10 (e.g 253 = 25.3'C)
// Sensor supply voltage: about 3.3V
//
int calc_sth11_temp(unsigned int t)
{
        //t = 10*(t *0.01 - 40.0); - 10x vìtší hodty - platí pro 5V
        //or
        //t = t *0.1 - 400;
        //t/=10;
        //t-=400;
        t=(signed int)(t-(4000+5))/10;		//+5 make a rounding
		return(t);
}

// calculates humidity [%RH]
// The relative humitidy is: -0.0000028*s*s + 0.0405*s - 4.0
// but this is very difficult to compute with integer math. 
// We use a simpler approach. See sht10_Non-Linearity_Compensation_Humidity_Sensors_E.pdf
//
// output: humi [%RH] (=integer value from 0 to 1000)
unsigned int rhcalc_int(unsigned int s)
{
	// s is in the range from 100 to 3340
	unsigned int rh;
	uint8_t k,divval;
	int addval;
	//for s less than 1712: (143*s - 8192)/4096
	//for s greater than 1712: (111*s + 46288)/4096
	//s range: 100<s<3350
        //
	//rh = rel humi * 10
//2 lines
/*	if (s<1712){
                // div by 4:
		rh=(36*s - 2048)/102;
	}else{
                // div by 8:
		rh=(14*s + 5790)/51;
	}
*/	
//3 lines
/*	if (s<=1236){
		rh=(42*s - 3056)/116; //((9*x-602)/25
	}else if (s<=2156){
		rh=(17*s + 2195)/55;	//((31*x)/4+951)/25
	}else{
		rh=((18*s) + 10735)/70;	//((13*x/2+3635))/25
	}
*/
//4 lines
	if (s<=1010){
		k=59;
		addval=-4475;
		divval=155;
		//rh=(59*s - 4475)/155; 
	}else if (s<=1693){
		k=27;
		addval=837;
		divval=82;
		//rh=(27*s + 837)/82;	
	}else if (s<=2399){
		k=20;
		addval=5333;
		divval=69;
		//rh=(20*s + 5333)/69;	
	}else {
		k=16;
		addval=11089;
		divval=64;
		//rh=((16*s) + 11089)/64;
	}
	rh=(k*s + addval)/divval;
	
// round up as we will cut the last digit to full percent
//	rh+=5;
//	rh/=10;
        //
	if (rh>980){
		rh=1000;
	}
	return(rh);
}

// calculates humidity [%RH] with temperature compensation
// input : humi [Ticks] (12 bit), temperature in 'C * 100 (e.g 253 for 25.3'C)
// output: humi [%RH] (=integer value from 0 to 1000)
unsigned int calc_sth11_humi(unsigned int h, int t)
{
        unsigned int rh;
        int temp;
		//unsigned int dtepå;
		rh=rhcalc_int(h);
        // now calc. Temperature compensated humidity [%RH]
        // the correct formula is:
        // rh_true=rh+(t/10-25)*(0.01+0.00008*(sensor_val)); 
		// you can approximate rh=sensor_val*k+q -> sensor_val=(rh-q)/k, k=0.0314 q=1,9209 
		//rh_true=rh+(t/10-25)*(0.01+0.00008*(rh-q)/k);
		//rh_true=rh+(t/10-25)*((0.01-q/k)+0.00008*rh/k);
		//rh_true=rh+(t/10-25)*(0.009846+0.0025477*rh); rh is <0,100>
		//rh_true=rh+(t/10-25)*(0.009846+0.0025477*rh/10); rh is <0,1000>, 0.00025477 ~1/4096, 
		//rh_true=rh+(t/10-25)*(4096*0.009846+rh)/4096;
		//rh_true=rh+(t/10-25)*((40+rh)/16)/256;
		t=t-250;
		temp=((signed int)(t)*(unsigned int)((40+rh)/16))>>8;
		if (t<0){	
			temp=temp | 0xff00;		//compiller made 0xffff >>8 = 0x00ff, it is wrong result in signed number
			temp=temp+1;			//compensation of rounding negative number
		}
		rh=(unsigned int)(rh+temp);
		return(rh);
}
/*
// this is an approximation of 100*log10(x) and does not need the math
// library. The error is less than 5% in most cases.
// compared to the real log10 function for 2<x<100.
// input: x=2<x<100 
// output: 100*log10(x) 
// Idea by Guido Socher
int log10_approx(unsigned char x)
{
	int l,log;
	if (x==1){
		return(0);
	}
	if (x<8){
		return(11*x+11);
	}
	//
	log=980-980/x;
	log/=10;
	if (x<31){
		l=19*x;
		l=l/10;
		log+=l-4;
	}else{
		l=67*x;
		l=l/100;
		if (x>51 && x<81){
			log+=l +42;
		}else{
			log+=l +39;
		}
	}
	if (log>200) log=200;
	return(log);
}
*/
// this is an approximation of 100*ln(x) and does not need the math
// library. The error is less than +-2 of value in most cases.
// compared to the real log function for 2<x<100.
// input: x=2<x<1000  log_apr(1000) is equal to ln(100) 
// output: 100*ln(x/10) 
int log_apr(unsigned int x)
{
	unsigned int k;//,q,q2;
	uint8_t index,temp;
	if (x==1){
		return(0);
	}
	if (x<50){
//		k=9950;
//		q=65535-2873;
//		q2=0;
//		return((9950*x-2873)/256);
		index=0;
	}else{
	if (x<110){
		index = 3;
//		k=3300;
//		q=78;
//		q2=102;
//		return((3300*x+78)/256+102);
	}else{
	if (x<170){
		index = 6;
//		k=1850;
//		q=21;
//		q2=161;
//		return((1850*x+21)/256+161);
	}else{
	if (x<250){
		index = 9;
//		k=1230;
//		q=202;
//		q2=202;
//		return((1230*x+202)/256+202);
	}else{
	if (x<350){
		index = 12;
//		k=860;
//		q=16;
//		q2=239;
//		return((860*x+16)/256+239);
	}else{
	if (x<500){
		index = 143;//15;	15 || 0x80;
//		k=121;
//		q=400;
//		q2=273;
//		return(((121*x+400)/2)/256+273);
	}else{
	if (x<700){
		index = 18;
//		k=430;
//		q=101;
//		q2=308;
//		return((430*x+101)/256+308);
	}else{
		index = 21;
//		k=310;
//		q=248;
//		q2=340;
//		return((310*x+248)/256+340);
	}//x<70
	}//x<50
	}//x<35
	}//x<25
	}//x<17
	}//x<11
	}//x<5

	temp=index&0x7f;	//keep only value
	k=x*pgm_read_word(&logconst[temp])+pgm_read_word(&logconst[temp+1]);
	if (index & 0x80)	k>>=1; //one line need divide by 512, information is saved in bit 7  
	return((k/256)+pgm_read_word(&logconst[temp+2]));
}

// calculates dew point
// input: humidity [in %RH times 10], temperature [in C times 10]
// output: dew point [in C times 10]
int calc_dewpoint(unsigned int rh,int t)
{ 
        // we use integer math
//		x=(log_apr(rh)-(100*ln(100)))-((88*t)/((243-t/10)/2));
//      DP=(48620/(220-(x/8)))*x/10
		int k,tmp;
        uint16_t utmp;
        k=log_apr(rh)-459;	
        //(1762/2)/10 ~ 88
        tmp=(unsigned int)(88*t);  // maximum valid temperature is (2^16)/88=744 -> 74.4'C 
        utmp=(unsigned int)(243+t/10)/2;
/*        tmp=(unsigned int)(44*t);   //  max temperature is 148.9'C, but computing error is a little larger  
        utmp=(unsigned int)(243+t/10)/4;*/
        if (t<0)	tmp=~tmp+1;		//convert negative number to positive
		utmp=(unsigned int)tmp/(unsigned int)utmp;
		if (t<0) k=k-(signed int)utmp;	//add negative number
		else k=k+(signed int)utmp;		//add positive number
        utmp=(unsigned int)(220-(k>>3)); //1762/8 ~ 220
	    utmp= (unsigned int)48620/ utmp;	//24310
		tmp=(unsigned int)utmp*(signed int)k;
		tmp=tmp>>4;	//2x larger constant, 8x smaller divider 
	if (tmp<0){
		tmp-=5;//tmp-=51;
	}else{
		tmp+=5;//tmp+=51;
	}
        return (tmp/10);
}
#endif /* _FCE_CIDLO_VLHKOSTI */

#ifdef _FCE_ZISKANI_HODNOT
//fce provede sekvenèní ètení z èidla, vypoète se korekce zmìøených údajù, výpoèet rosného bodu
//a umístìní dat do bufferu LCD 
/*
void	ziskat_vlhkost(void){
static time_t adtime1=0;
static uint8_t s_conreset_cnt=0;  // do not reset the sensirion connection everytime
unsigned char error;
if (((sys_time()-adtime1) & 0xff)>210) {//každé 2 sekundy - pro snížení vlastního zahøívání - pracuje jen 10% èasu - 14bit pøevod trvá 210ms
					adtime1=sys_time();
char str[10];
	    int dew,temp;
        unsigned char rh,j=0;
		if (s_conreset_cnt>9){
                s_connectionreset();
                s_conreset_cnt=0;
        }
        s_conreset_cnt++;		//aby se zvyšovalo každý cyklus
        error=s_measure( &gTempval_Raw,0); //measure temperature
        if (error==0){
                error=s_measure( &gHumival_Raw,1); //measure humidity
        }
        temp=calc_sth11_temp(gTempval_Raw);
        itoa(temp,str,10);
		AddDot(str, 1);
		j=PutsToBuf("t:", line1buf,0); //convlib.c
		j=PutsToBuf(str, line1buf,--j);	//convlib.c
		j=PutsToBuf("'C", line1buf,--j); //convlib.c
//		rh=rhcalc_int(gHumival_Raw);
//		itoa(rh,str,10);
//		PutsToBuf(str, line1buf,10);

		rh=calc_sth11_humi(gHumival_Raw,temp);
		itoa(rh,str,10);
		AddDot(str, 1);
		j=PutsToBuf("R:", line1buf,j); //convlib.c
		j=PutsToBuf(str, line1buf,--j);
		j=PutsToBuf("%D:", line1buf,--j); //convlib.c		
//		rh=rh/10;
		//(unsigned char) 
		dew=calc_dewpoint(rh,temp);
		itoa(dew,str,10);
		AddDot(str, 1);
		j=PutsToBuf(str, line1buf,--j);		
		PutsToBuf("'C", line1buf,--j); //convlib.c
}
}
*/

#endif /* _FCE_ZISKANI_HODNOT */


/*F***************************************************************************
* NAME: sensirion_protocol
*-----------------------------------------------------------------------------
* PARAMS:
*
* return:
*----------------------------------------------------------------------------
* PURPOSE: 
*   Read raw data from sensor and calculate truth value
*----------------------------------------------------------------------------
* EXAMPLE:
*----------------------------------------------------------------------------
* NOTE: 
*----------------------------------------------------------------------------
* REQUIREMENTS:
******************************************************************************/

#ifdef _FCE_ZISKANI_HODNOT_STAUT
//fce provede ètení z èidla, v jednom stavu teplota a v druhém stavu vlhkost, vypoète se korekce zmìøených údajù, výpoèet rosného bodu
//a umístìní dat do bufferu LCD 
void	ziskat_vlhkost_st_automat(void){ //každé 2 sekundy - pro snížení vlastního zahøívání - pracuje jen 10% èasu - 14bit pøevod trvá 210ms
	static time_t adtime1=0; 
	if (((sys_time()-adtime1) & 0xff)>210) {
		static	uint8_t	timeout=1;		//udává dobu, do kdy musí èidlo dokonèit pøevod,6 jako 600ms 	
		static uint8_t	stav_aut = INIT_HUM; //MEASURE_HUMI; //MEASURE_TEMP 
		uint8_t	error = 0;
		adtime1=sys_time();

		if (--timeout==0){	//jen proto, aby se volala instrukce dec, ta rovonu nastaví flag	
                s_connectionreset();
				timeout=HUMI_TIMEOUT;		//nastavit nový timeout
                if	(stav_aut == INIT_HUM){
					stav_aut = MEASURE_TEMP;
				//spustí se mìøení, nastaví se další stav, neprovede se ètení dat a jejich zpracování
					s_transstart(); //transmission start
					if (s_write_byte(stav_aut)){
						error = CHYBA_START_KOM;
					}
				}else error = CHYBA_TIMEOUT;
        }
		//zde se bude testovat, jestli byl dokonèen pøevod
//		if (timeout==0)	error = CHYBA_TIMEOUT;	//pokud vypršel èas na pøevod, je to chyba
		if ( (error != 0) || (GETDATA==0) ){	//když je chyba, musí se zaèit mìøit druhá hodnota, když není chyba a je dokonèeno mìøení, taky se musí spustit další mìøení 
//		if ( (timeout==0) || (GETDATA==0) ){	//když je chyba, musí se zaèit mìøit druhá hodnota, když není chyba a je dokonèeno mìøení, taky se musí spustit další mìøení 
			uint8_t crc_state=0;
			uint8_t	temp;
			uint16_t	raw_data;
			char str[10];
			timeout=HUMI_TIMEOUT;		//nastavit nový timeout
			crc_state=computeCRC8(bitswapbyte(stav_aut),crc_state);
        	temp=s_read_byte(1); //read the first byte (MSB)
        	raw_data=temp<<8;
        	crc_state=computeCRC8(bitswapbyte(temp),crc_state);
        	temp=s_read_byte(1); //read the second byte (LSB)
        	raw_data |=temp;
			crc_state=computeCRC8(bitswapbyte(temp),crc_state);
        	temp =s_read_byte(0); //read checksum
        	if (crc_state != temp ) {
                error = CHYBA_CRC;
        	}
			switch(stav_aut){
				case MEASURE_HUMI:{
					uint8_t j;
					//pøeète se hodnota z èidla, výpoèet, uložení do bufferu
					//v pøípadì chyby se uloží na displ info o chybì
					//nastaví se nový stav automatu
					int16_t temp_int,temp_int2;	//nerad, ale musím volat ještì jednou korekci teploty, abych nemusel mít další promìnnou static
					stav_aut = MEASURE_TEMP;
					s_transstart(); //transmission start
					if (s_write_byte(stav_aut)){	//sice by to chtìlo zmìnit - aby tento error se projevil až za následujícím errorem 
						error = CHYBA_START_KOM;	//protože se dává zobrazí info o chybì u špatného senzoru, ale to by zbyteènì nerostl kód
					}								//pak další test na error, poté vypsat do místa druhého èidla, pøípadnì doplnit vypsání kódu erroru
					if (error != 0){
						PutsToBuf("R:ER", line1buf,9); //convlib.c
						break;
					}

					gHumival_Raw=raw_data;
					temp_int=calc_sth11_temp(gTempval_Raw);//sice to veme èas, ale jinak by musela být promìnná static, kde by se hodnota pøedávala
					temp_int2=calc_sth11_humi(raw_data,temp_int);
					itoa(temp_int2,str,10);
					AddDot(str, 1);
					j=PutsToBuf("R:", line1buf,8); //convlib.c
					j=PutsToBuf(str, line1buf,--j);
					j=PutsToBuf("%D:", line1buf,--j); //convlib.c		
//					temp_int2=temp_int2/10;
					temp_int2=calc_dewpoint(temp_int2,temp_int);
					itoa(temp_int2,str,10);
					AddDot(str, 1);
					j=PutsToBuf(str, line1buf,--j);		
					PutsToBuf("'C", line1buf,--j); //convlib.c
				break;
				}
				case MEASURE_TEMP:{
					uint8_t j;
					int16_t tepl_temp;
					//pøeète se hodnota z èidla, uložení do bufferu
					//...
					//nastaví se druhý stav automatu
					stav_aut = MEASURE_HUMI;
					s_transstart(); //transmission start
					if (s_write_byte(stav_aut)){	//sice by to chtìlo zmìnit - aby tento error se projevil až za následujícím errorem 
						error = CHYBA_START_KOM;	//protože se dává zobrazí info o chybì u špatného senzoru, ale to by zbyteènì nerostl kód
					}								//pak další test na error, poté vypsat do místa druhého èidla, pøípadnì doplnit vypsání kódu erroru
					if (error != 0){
						PutsToBuf("t:ER", line1buf,0); //convlib.c
						break;
					}
					gTempval_Raw=raw_data;
					tepl_temp=calc_sth11_temp(raw_data);
					itoa(tepl_temp,str,10);
					j=PutsToBuf("t:", line1buf,0); //convlib.c
					AddDot(str, 1);
					j=PutsToBuf(str, line1buf,--j);	//convlib.c
					j=PutsToBuf("'", line1buf,--j); //convlib.c //konèí j=8
				break;
				}
		
		}
		}

}
}
#endif /* _FCE_ZISKANI_HODNOT_STAUT */
