#ifndef BUILDINFO_H_ 
#include "buildinfo.h" 
#endif 
 
/************************************************************ 
DO NOT MODIFY 
Automatically Generated On -08.10.2013- 22-16- by deploy-success99.bat 
*************************************************************/ 
 
#if defined(BUILDINFO_RAM) 
static const uint16_t BUILD_NUMBER = 2; 
#elif defined(BUILDINFO_EEMEM) 
static const uint16_t BUILD_NUMBER EEMEM = 2; 
#elif defined(BUILDINFO_PROGMEM) 
static const uint16_t BUILD_NUMBER PROGMEM = 2; 
#endif 
 
 
uint16_t GetBuildNumber() 
{ 
    uint16_t val; 
#if defined(BUILDINFO_RAM) 
    val = BUILD_NUMBER; 
#elif defined(BUILDINFO_EEMEM) 
    val = eeprom_read_word( &BUILD_NUMBER ); 
#elif defined(BUILDINFO_PROGMEM) 
    val = pgm_read_word(&BUILD_NUMBER); 
#endif 
    return val; 
} 
