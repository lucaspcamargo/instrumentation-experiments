#ifndef CAMARGO_ILI9320
#define CAMARGO_ILI9320	

#include "inc/hw_ssi.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"

#include "common.h"
#include "led.h"

#define ILI93200_CTRL_PERIPH SYSCTL_PERIPH_GPIOD
#define ILI93200_CTRL_PORT GPIO_PORTD_BASE

#define ILI93200_CTRL_RD GPIO_PIN_1
#define ILI93200_CTRL_WR GPIO_PIN_2
#define ILI93200_CTRL_RS GPIO_PIN_3
#define ILI93200_CTRL_CS GPIO_PIN_6
#define ILI93200_CTRL_RESET GPIO_PIN_7

#define ILI93200_DATA_PERIPH SYSCTL_PERIPH_GPIOB
#define ILI93200_DATA_PORT GPIO_PORTB_BASE

#define ILI93200_SYS_CLK (80000000ul)
#define FROM_NS(X) ((X)*ILI93200_SYS_CLK/1000000000ul)
//#define DELAY_NS(X) SysCtlDelay(FROM_NS((X))>0? FROM_NS((X)) : 1)
#define DELAY_NS(X) SysCtlDelay(FROM_NS((500)))

#define _cILI9320_CTRL(PIN, VAL) GPIOPinWrite(ILI93200_CTRL_PORT, ILI93200_CTRL_##PIN, (VAL)? 0xff : 0) 
#define _cILI9320_DATA(VAL) GPIOPinWrite(ILI93200_DATA_PORT, PINS_0_7, (VAL)) 
#define _cILI9320_DATA_GET() GPIOPinRead(ILI93200_DATA_PORT, PINS_0_7) 


bool _cILI9320_initialized = 0;


void cILI9320Reset()
{
    _cILI9320_CTRL(RESET, 1);
    SysCtlDelay(ILI93200_SYS_CLK / 1000); //delay 1ms
    _cILI9320_CTRL(RESET, 0);
    SysCtlDelay(ILI93200_SYS_CLK / 100); //delay 10ms
    _cILI9320_CTRL(RESET, 1);
    SysCtlDelay(ILI93200_SYS_CLK / 10); //delay 100ms
}

void cILI9320Write(uint16_t value)
{
  _cILI9320_DATA(value >> 8);
  DELAY_NS(10);
  _cILI9320_CTRL(WR, 0);
  DELAY_NS(50);
  _cILI9320_CTRL(WR, 1);
  DELAY_NS(50);
  _cILI9320_DATA(value & 0xff);
  DELAY_NS(10);
  _cILI9320_CTRL(WR, 0);
  DELAY_NS(50);
  _cILI9320_CTRL(WR, 1);
  
}

void cILI9320WriteReg(uint16_t reg, uint16_t value)
{
  _cILI9320_CTRL(RS, 0);
  DELAY_NS(20);

  cILI9320Write(reg);
  
  DELAY_NS(10);
  _cILI9320_CTRL(RS, 1);
  
  DELAY_NS(50);
  
  cILI9320Write(value);
}

uint16_t cILI9320Read()
{
  uint16_t ret = 0;
  
  GPIODirModeSet(ILI93200_DATA_PORT, PINS_0_7, GPIO_DIR_MODE_IN);
  GPIOPinTypeGPIOInput(ILI93200_DATA_PORT, PINS_0_7); //input, read data now
  
  _cILI9320_CTRL(RD, 0);
  DELAY_NS(50);
  ret = _cILI9320_DATA_GET() << 8;
  DELAY_NS(20);
  _cILI9320_CTRL(RD, 1);
  DELAY_NS(20);
  _cILI9320_CTRL(RD, 0);
  DELAY_NS(50);
  ret |= _cILI9320_DATA_GET();
  DELAY_NS(20);
  _cILI9320_CTRL(RD, 1);
  
  GPIODirModeSet(ILI93200_DATA_PORT, PINS_0_7, GPIO_DIR_MODE_OUT);
  GPIOPinTypeGPIOOutput(ILI93200_DATA_PORT, PINS_0_7); //back to output
  
  return ret;
}

uint16_t cILI9320ReadReg(uint16_t reg)
{
  _cILI9320_CTRL(RS, 0);
  DELAY_NS(20);

  cILI9320Write(reg);
  
  DELAY_NS(10);
  _cILI9320_CTRL(RS, 1);
  
  DELAY_NS(50);
  
  return cILI9320Read();
}


void cILI9320WritePixels(const uint16_t *pixels, int count)
{
  _cILI9320_CTRL(RS, 0);
  DELAY_NS(20);

  cILI9320Write(0x22);
  
  DELAY_NS(10);
  _cILI9320_CTRL(RS, 1);
  
  while(count)
  {
    
    DELAY_NS(50);
    cILI9320Write(*pixels);
    
    ++pixels;
    --count;
  }
}

int cILI9320Init()
{
  if(_cILI9320_initialized) 
    return 0;

  // INITIALIZE PORTS
  
  //control bus
  SysCtlPeripheralEnable(ILI93200_CTRL_PERIPH);
  GPIOPinTypeGPIOOutput(ILI93200_CTRL_PORT, ILI93200_CTRL_RD | ILI93200_CTRL_WR | ILI93200_CTRL_RS | ILI93200_CTRL_CS | ILI93200_CTRL_RESET); //initialize control bus as outputs
  GPIOPinWrite(ILI93200_CTRL_PORT, ILI93200_CTRL_RD | ILI93200_CTRL_WR | ILI93200_CTRL_RS | ILI93200_CTRL_CS | ILI93200_CTRL_RESET, 0xff ); //all ones in control bus, they are active low
  
  // data bus
  SysCtlPeripheralEnable(ILI93200_DATA_PERIPH);
  GPIOPinTypeGPIOOutput(ILI93200_DATA_PORT, PINS_0_7); //initialize data bus as outputs
  GPIOPinWrite(ILI93200_DATA_PORT, PINS_0_7, 0x00); //write all zeroes
  
  
  // INITIAL SEQUENCE

  _cILI9320_CTRL(CS, 0); // always selected 
  cILI9320Reset(); 
  
  // POWER ON SEQUENCE
  
  //...
  
  
  _cILI9320_initialized = 1;
  return 0;
}


#endif