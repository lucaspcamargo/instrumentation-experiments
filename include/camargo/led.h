#pragma once

#include "common.h"
#include "inc/hw_gpio.h"

#define LED_GPIO_PERIPH     SYSCTL_PERIPH_GPIOF
#define LED_GPIO_BASE       GPIO_PORTF_BASE

#define LED_R             GPIO_PIN_1
#define LED_G             GPIO_PIN_3
#define LED_B             GPIO_PIN_2

#define LEDS_ALL             (LED_R | LED_G | LED_B)

inline void cLedInit()
{
  SysCtlPeripheralEnable(LED_GPIO_PERIPH);
 
  GPIODirModeSet(LED_GPIO_BASE, LEDS_ALL, GPIO_DIR_MODE_OUT);
  GPIOPadConfigSet(LED_GPIO_BASE, LEDS_ALL, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
}

inline void cLedSet(uint8_t colors)
{
  GPIOPinWrite(LED_GPIO_BASE, LEDS_ALL, colors );
}