#ifndef CAMARGO_AD84XX
#define CAMARGO_AD84XX

#include "ssi.h"
#include "common.h"

#define AD84XX_CS_GPIO_PERIPH SYSCTL_PERIPH_GPIOA
#define AD84XX_CS_GPIO_BASE GPIO_PORTA_BASE
#define AD84XX_CS_GPIO_PIN GPIO_PIN_7

#ifndef AD84XX_TRACK_VAL
#define AD84XX_TRACK_VAL 1000
#endif

cSSIConfig _cAD84XX_config = {100000, 10, SSI_FRF_MOTO_MODE_0}; // bitrate whatever, 10 data bits, std spi
bool _cAD84XX_initialized = 0;

int cAD84XXInit()
{
  if(_cAD84XX_initialized) return 0;
  
  if(cSSIInit()) return -1;
  
  SysCtlPeripheralEnable(AD84XX_CS_GPIO_PERIPH);
  GPIOPinTypeGPIOOutput(AD84XX_CS_GPIO_BASE, AD84XX_CS_GPIO_PIN);
  GPIOPinWrite(AD84XX_CS_GPIO_BASE, AD84XX_CS_GPIO_PIN, AD84XX_CS_GPIO_PIN);
  
  _cAD84XX_initialized = true;
  return 0;
}

void cAD84XXSelect(bool select)
{
  GPIOPinWrite(AD84XX_CS_GPIO_BASE, AD84XX_CS_GPIO_PIN, select? 0 : AD84XX_CS_GPIO_PIN);
}

void cAD84XXSetValue(uint8_t unit, uint8_t value)
{ 
  cSSIEnsureConfig(&_cAD84XX_config);
  
  cAD84XXSelect(true);
  cSSIBitDelay();
  cSSIWrite(((unit&0x03)<<8) | value);
  cSSIWait();
  cSSIBitDelay();
  cAD84XXSelect(false);
  
}

#endif