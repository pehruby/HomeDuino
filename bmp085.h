/*
 * bmp085.h
 *
 * Created: 19.4.2014 19:53:07
 *  Author: hruby
 */ 


#ifndef BMP085_H_
#define BMP085_H_


#define BMP085_ADDRESS 0x77  // I2C address of BMP085
#define KOMPENZACE_VYSKY 26,5


class bmp085 {
	
	public:
		void Calibration();
		long GetPressure(unsigned long up);
		short GetTemperature(unsigned int ut);
		long GetPressureFin();
		
	private:
		char Read(unsigned char address);
		int ReadInt(unsigned char address);
		unsigned int ReadUT();
		unsigned long ReadUP();
		// Calibration values
		int ac1;
		int ac2;
		int ac3;
		unsigned int ac4;
		unsigned int ac5;
		unsigned int ac6;
		int b1;
		int b2;
		int mb;
		int mc;
		int md;

		// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
		// so ...Temperature(...) must be called before ...Pressure(...).
		long b5;

		short temperature;
		long pressure;

		// Use these for altitude conversions
		const float p0 = 101325;     // Pressure at sea level (Pa)
		float altitude;
	
	};

#endif /* BMP085_H_ */