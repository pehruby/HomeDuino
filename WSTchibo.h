/*
 * WSTchibo.h
 *
 * Created: 27.10.2013 13:53:51
 *  Author: hruby
 */ 


#ifndef WSTCHIBO_H_
#define WSTCHIBO_H_

#define TCHBITU	42

class WSTchibo 
{
	
	public:
		WSTchibo();
		void accept(byte interval,byte level);
		byte* getpacet();
		bool acquired(byte ch);
		byte get_humidity(byte ch);
		uint16_t get_temp_raw(byte ch);
		int get_temp(byte ch);
		bool lowbatt(byte ch);
	
	private:
	
		byte _packet[6];
		byte _finpacket[3][6];
		bool _acquired[3];

};


#endif /* WSTCHIBO_H_ */