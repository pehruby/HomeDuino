/*
 * iqrf.h
 *
 * Created: 8.12.2013 20:30:23
 *  Author: hruby
 */ 


#ifndef IQRF_H_
#define IQRF_H_


#define MAXCNT 20000		// max pocet cyklu cekani na novy znak
#define PAKETLEN 30			// delka bez preambule
#define IQPREAMBLE 0xa5		// preambule paketu
#define MAXENVENTR	16		// max hodnota cisla channel v sondach
#define LENCHANNAME 5

struct iqrfpacket {
	byte len;			//1
	byte to;			//2 zatim vzdy 255
	byte type;			//3 cislo sondy (asi momentalne bez vyznamu), typ sondy 1=energo,2=enviro,3=EZS
	byte subtype;		//4 
						/* energo (odpovida poli cenik/probetype v SQL) 1-plyn /BK-G3/, 10-elektro
						
						
						*/
	byte channel;		//5 cislo kanalu, jednoznacne urceni sondy v ramci type
	byte seqnr;			//6
	char channame[LENCHANNAME];	//7-11 jednoznacny nazev kanalu
	byte battery;			//12
	byte ctrl;			//13	0=bin (value obsahuje merenou hodnotu), 1=control (value obsahuje ridici hodnotu), 2=text (je platna polozka string),128 (MSB bit8) umoznit nacitani
	byte value;			//14
	char string[15];	//15 - 29
	byte crc;			//30
};

struct enerstruct {			// struktura pro ukladani dat z energosond
	char channame[LENCHANNAME+1];	// +1 abych tam mohl dat 0 jako zakonceni retezce
	byte subtype;			// viz subtype v iqrfpacket	
	int value;				// co kdyz se behem intervalu mezi zapisy do MySQL nacte vic nez 65536 impulsu? !!! asi to nenastane ...
	byte expseq;			// ocekavane sekv.c.
};

struct envstruct {			// struktura pro ukladani dat z envirosond
	char channame[LENCHANNAME+1];	// +1 abych tam mohl dat 0 jako zakonceni retezce
	float value;
	bool zapsano;			// hodnota jiz byla zpracovana/ zapsana do databaze (MySQL)
	bool platna;			// platna polozka, sonda v danem kanalu jiz poslala nejaka data a mam nakopirovano jeji channame (z IQRF paketu)

};

class IQRF {
	
	public:
		IQRF();
		int readbyte(byte znak);
		int zpracujpaket();
		bool acquired();
		char writevalmysql();
		char writeenvmysql();
	//	int readpacket();
	private:
		char cntrozdilsn(char prislo,char cekam);
		char snplus1(char c);		
		int zpracuj_energo();
		int zpracuj_enviro();
		int SNcheck();								// kontrola Seq. Number
		int CRCcheck();								// kontrola CRC
		float TempIQRFtoFloat(char cela, char deset); // prevod teploty z hodnoty poslane z IQRF do typu float
		struct iqrfpacket _aktpaket;				//sem se bude davat aktualne prijimany paket
		struct enerstruct _enerval[MAXENVENTR];		// pole, kam se budou pro jednotlive kanaly nacitat hodnoty/impulzy. Plati u kanalu, ktere maji nacitani povoleno a pro type=energo
		struct envstruct _envval[MAXENVENTR];	// pole, kam se budou pro jednotlive enviro kanaly nacitat hodnoty
		byte _aktdelka;							// pocet prijatych bytu aktualniho paketu
		byte _state;							// stavovy automat
		byte _rozdilsnv;						// rozdil SN aktualniho paketu od ocekavaneho SN
		bool _novypaket;						// v _aktpaket je novy nezpracovany paket
};



#endif /* IQRF_H_ */