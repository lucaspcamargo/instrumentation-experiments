#ifndef CAMARGO_SEGDISPLAY_H
#define CAMARGO_SEGDISPLAY_H

#include "common.h"

#define DISPLAY_M_PERIPH SYSCTL_PERIPH_GPIOE
#define DISPLAY_M_PORT_BASE GPIO_PORTE_BASE
#define DISPLAY_M_PINS PINS_1_4
#define DISPLAY_M_SEP1_PIN GPIO_PIN_5
#define DISPLAY_M_PIN_BASE 2
#define DISPLAY_D_PERIPH SYSCTL_PERIPH_GPIOB
#define DISPLAY_D_PORT_BASE GPIO_PORTB_BASE


const uint8_t DISPLAY_ALPHA[] = {
	0b11000000, //0
        0b11111001, //1
        0b10100100, //2
        0b10110000, //3
        0b10011001, //4
        0b10010010, //5
        0b10000010, //6
        0b11111000, //7
        0b10000000, //8
        0b10010000, //9
        0b10001000, //a
        0b10000011, //b
        0b11000110, //c
        0b10100001, //d
        0b10000110, //e
        0b10001110, //f
	0b01000000, //0.
        0b01111001, //1.
        0b00100100, //2.
        0b00110000, //3.
        0b00011001, //4.
        0b00010010, //5.
        0b00000010, //6.
        0b01111000, //7.
        0b00000000, //8.
        0b00010000, //9.
        0b00001000, //a.
        0b00000011, //b.
        0b01000110, //c.
        0b00100001, //d.
        0b00000110, //e.
        0b00001110, //f.
        0b11111111  //blank (0x20)
    };

void cDisplayInit()
{
  
  SysCtlPeripheralEnable(DISPLAY_M_PERIPH);
  SysCtlPeripheralEnable(DISPLAY_D_PERIPH);
  
  GPIOPinTypeGPIOOutput(DISPLAY_M_PORT_BASE, DISPLAY_M_PINS|DISPLAY_M_SEP1_PIN);
  GPIOPadConfigSet(DISPLAY_M_PORT_BASE, DISPLAY_M_PINS|DISPLAY_M_SEP1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
  
  GPIOPinTypeGPIOOutput(DISPLAY_D_PORT_BASE, PINS_0_7);
  GPIOPadConfigSet(DISPLAY_D_PORT_BASE, PINS_0_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
  
  GPIOPinWrite(DISPLAY_M_PORT_BASE, DISPLAY_M_PINS, 0x00);
  GPIOPinWrite(DISPLAY_D_PORT_BASE, PINS_0_7, 0x00);

}

void cDisplayMultiplex(const uint8_t *newData)
{
  static int currDigit = 0;
  static const uint8_t *displayData = 0;
  
  if(newData)
    displayData = newData;
  
  GPIOPinWrite(GPIO_PORTB_BASE, 0xff, displayData[currDigit]);

  GPIOPinWrite(DISPLAY_M_PORT_BASE, DISPLAY_M_PINS, DISPLAY_M_PIN_BASE << currDigit);

  SysCtlDelay(100);
  
  GPIOPinWrite(DISPLAY_M_PORT_BASE, DISPLAY_M_PINS, 0);

  SysCtlDelay(1);
  
  currDigit++;
  currDigit %= 4;
}

void cDisplaySetSeparator(int on)
{
  GPIOPinWrite(DISPLAY_M_PORT_BASE, DISPLAY_M_SEP1_PIN, on? DISPLAY_M_SEP1_PIN : DISPLAY_M_SEP1_PIN);
}


#endif