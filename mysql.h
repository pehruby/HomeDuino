/*
 * mysql.h
 *
 * Created: 25.11.2013 21:38:32
 *  Author: hruby
 */ 


#ifndef MYSQL_H_
#define MYSQL_H_

#define MAXTIME 1000

#define MAXPROBETYPENR	20			// max hodnota, ktera se muze objevis v subtype poli v IQRF paketu




class MySQL {
	
	private:
		String _StartURL;
		String _StartEnerURL;
		String _StartEnvURL;
		String _StartPriceURL;
		String _Password;
		byte *_Host;
		float _pricepulse[MAXPROBETYPENR];			// cena za impuls pro dany subtype (v poli IQRF paketu)
		
		void _phpURL(String URL,String *vystup);
		boolean naplncenik(int probetype);
	
	public:
		MySQL(byte *Host, String StartURL, String Pass);
		void insertEnvvalue(String Name, String Value );
		void insertEnervalue(String Name, String Value, String Price );
		float getprice(int probetype,boolean force);
		
	
};



#endif /* MYSQL_H_ */