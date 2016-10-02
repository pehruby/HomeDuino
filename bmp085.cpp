/*
 * bmp085.cpp
 *
 * Created: 19.4.2014 19:49:24
 *  Author: hruby
 */ 

#include <Arduino.h>
#include <stdlib.h>
#include <stdio.h>
#include <Wire.h>
#include "bmp085.h"

const unsigned char OSS = 3;  // Oversampling Setting, presnost 0-3(nejvetsi)

// Put this line at the top of your program


// Read 1 byte from the BMP085 at 'address'
char bmp085::Read(unsigned char address)
{
	// unsigned char data;
	
	Wire.beginTransmission(BMP085_ADDRESS);
	Wire.write(address);
	Wire.endTransmission();
	
	Wire.requestFrom(BMP085_ADDRESS, 1);
	while(!Wire.available())
	;
	
	return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085::ReadInt(unsigned char address)
{
	unsigned char msb, lsb;
	
	Wire.beginTransmission(BMP085_ADDRESS);
	Wire.write(address);
	Wire.endTransmission();
	
	Wire.requestFrom(BMP085_ADDRESS, 2);
	while(Wire.available()<2)
	;
	msb = Wire.read();
	lsb = Wire.read();
	
	return (int) msb<<8 | lsb;
}
void bmp085::Calibration()
{
	
	Serial.println("Kalibrace");
	ac1 = ReadInt(0xAA);
	ac2 = ReadInt(0xAC);
	ac3 = ReadInt(0xAE);
	ac4 = ReadInt(0xB0);
	ac5	= ReadInt(0xB2);
	ac6 = ReadInt(0xB4);
	b1 = ReadInt(0xB6);
	b2 = ReadInt(0xB8);
	mb = ReadInt(0xBA);
	mc = ReadInt(0xBC);
	md = ReadInt(0xBE);
}

// Read the uncompensated temperature value
unsigned int bmp085::ReadUT()
{
	unsigned int ut;
	
	// Write 0x2E into Register 0xF4
	// This requests a temperature reading
	Wire.beginTransmission(BMP085_ADDRESS);
	Wire.write(0xF4);
	Wire.write(0x2E);
	Wire.endTransmission();
	
	// Wait at least 4.5ms
	delay(5);
	
	// Read two bytes from registers 0xF6 and 0xF7
	ut = ReadInt(0xF6);
	return ut;
}


// Read the uncompensated pressure value
unsigned long bmp085::ReadUP()
{
	unsigned char msb, lsb, xlsb;
	unsigned long up = 0;
	
	// Write 0x34+(OSS<<6) into register 0xF4
	// Request a pressure reading w/ oversampling setting
	Wire.beginTransmission(BMP085_ADDRESS);
	Wire.write(0xF4);
	Wire.write(0x34 + (OSS<<6));
	Wire.endTransmission();
	
	// Wait for conversion, delay time dependent on OSS
	delay(2 + (3<<OSS));
	
	// Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
	Wire.beginTransmission(BMP085_ADDRESS);
	Wire.write(0xF6);
	Wire.endTransmission();
	Wire.requestFrom(BMP085_ADDRESS, 3);
	
	// Wait for data to become available
	while(Wire.available() < 3)
	;
	msb = Wire.read();
	lsb = Wire.read();
	xlsb = Wire.read();
	
	up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
	
	return up;
}

short bmp085::GetTemperature(unsigned int ut)
{
	long x1, x2;
	
	x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
	x2 = ((long)mc << 11)/(x1 + md);
	b5 = x1 + x2;

	return ((b5 + 8)>>4);
}


// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085::GetPressure(unsigned long up)
{
	long x1, x2, x3, b3, b6, p;
	unsigned long b4, b7;
	
	b6 = b5 - 4000;
	// Calculate B3
	x1 = (b2 * (b6 * b6)>>12)>>11;
	x2 = (ac2 * b6)>>11;
	x3 = x1 + x2;
	b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
	
	// Calculate B4
	x1 = (ac3 * b6)>>13;
	x2 = (b1 * ((b6 * b6)>>12))>>16;
	x3 = ((x1 + x2) + 2)>>2;
	b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
	
	b7 = ((unsigned long)(up - b3) * (50000>>OSS));
	if (b7 < 0x80000000)
	p = (b7<<1)/b4;
	else
	p = (b7/b4)<<1;
	
	x1 = (p>>8) * (p>>8);
	x1 = (x1 * 3038)>>16;
	x2 = (-7357 * p)>>16;
	p += (x1 + x2 + 3791)>>4;
	
	return p;
}


long bmp085::GetPressureFin() {
	
	long pressure;
	short temperature;
	
	temperature = GetTemperature(ReadUT());
	
	Serial.print("Teplota x 10");
	Serial.println(temperature);
	
	pressure = GetPressure(ReadUP());
	
	Serial.print("Tlak x 10:");
	Serial.println(pressure);
	
	
	return pressure;
}




