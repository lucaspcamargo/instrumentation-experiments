#ifndef CAMARGO_SSI
#define CAMARGO_SSI


#include "inc/hw_ssi.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"

#include "common.h"
#include "led.h"

#define CSSI_B SSI0_BASE
#define CSSI_PERIPH SYSCTL_PERIPH_SSI0

typedef struct cSSIConfig_struct
{
  uint32_t bitrate;
  uint32_t dataWidth;
  uint32_t protocol;
} cSSIConfig;

bool _cSSI_initialized = 0;
cSSIConfig _cSSI_config = {1000000, 8, SSI_FRF_MOTO_MODE_1};

void cSSIIntHandler();

inline void _cSSIApplyConfig()
{
  SSIDisable(CSSI_B);	
  SSIConfigSetExpClk(CSSI_B,SysCtlClockGet(), _cSSI_config.protocol, SSI_MODE_MASTER, 
		     _cSSI_config.bitrate, _cSSI_config.dataWidth);
  SSIEnable(CSSI_B);	
}

int cSSIInit()
{
  if(_cSSI_initialized) 
    return 0;

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinTypeSSI(GPIO_PORTA_BASE,GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_2
#ifdef CSSI_ENABLE_CS
  |GPIO_PIN_3
#endif
  );
  
  GPIOPinConfigure(GPIO_PA2_SSI0CLK);
#ifdef CSSI_ENABLE_CS
  GPIOPinConfigure(GPIO_PA3_SSI0FSS);
#endif
  GPIOPinConfigure(GPIO_PA4_SSI0RX);
  GPIOPinConfigure(GPIO_PA5_SSI0TX);
  
  
  SysCtlPeripheralEnable(CSSI_PERIPH);
  SSIClockSourceSet(CSSI_B, SSI_CLOCK_SYSTEM);
  _cSSIApplyConfig();
  
  // We don't use interrupt
  //SSIIntRegister(CSSI_B, cSSIIntHandler);
  //SSIIntEnable(CSSI_B, SSI_TXFF|SSI_RXFF|SSI_RXTO|SSI_RXOR );
  
  SSIEnable(CSSI_B);	
  
  _cSSI_initialized = 1;
  return 0;
}

void cSSIIntHandler()
{
  uint32_t ui32Status = SSIIntStatus(CSSI_B, true); 
  SSIIntClear( CSSI_B, ui32Status );
}

inline void cSSIWait()
{ 
  while(SSIBusy(CSSI_B))
  {}
}

void cSSIClean()
{
  uint32_t dataRx[8];
  
  while(SSIDataGetNonBlocking(CSSI_B, dataRx))
  {}
  
  cSSIWait();
}

void cSSIEnsureConfig(cSSIConfig * c)
{
  bool changed = false;
  
  if(c->bitrate > 0 && _cSSI_config.bitrate != c->bitrate)
  {
    changed = true;
    _cSSI_config.bitrate = c->bitrate;
  }
  
  if(c->dataWidth > 0 && _cSSI_config.dataWidth != c->dataWidth)
  {
    changed = true;
    _cSSI_config.dataWidth = c->dataWidth;
  }
  
  if(_cSSI_config.protocol != c->protocol)
  {
    changed = true;
    _cSSI_config.protocol = c->protocol;
  }
  
  if(changed)
  {
    cSSIClean();
    _cSSIApplyConfig();
  }
}

void cSSIRead(uint32_t *dest)
{
  uint32_t trashBin[8];
  
  while(SSIDataGetNonBlocking(SSI0_BASE, trashBin)) {
  } // empty fifo

  cLedSet(LED_G);
  SSIDataPut(CSSI_B, 0);
  SSIDataGet(CSSI_B, dest);
  cLedSet(0);
}

void cSSIWrite(uint32_t word)
{
  cLedSet(LED_G | LED_R);
  SSIDataPut(CSSI_B, word);
  cLedSet(0);
}

int cSSIReadFIFO(uint32_t *buf8)
{
  return SSIDataGetNonBlocking(CSSI_B, buf8);
}

int cSSIWriteFIFO(uint32_t word)
{
  return SSIDataPutNonBlocking(CSSI_B, word);
}

void cSSIBitDelay()
{
  //SysCtlDelay(SysCtlClockGet()/_cSSI_config.bitrate);
}


#endif