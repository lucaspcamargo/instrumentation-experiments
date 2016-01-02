#ifndef CAMARGO_BUTTONS_H
#define CAMARGO_BUTTONS_H

#include "common.h"
#include "inc/hw_gpio.h"

#define BUTTON_GPIO_PERIPH     SYSCTL_PERIPH_GPIOF
#define BUTTON_GPIO_BASE       GPIO_PORTF_BASE

#define BUTTON_COUNT             2
#define BUTTON_LEFT             GPIO_PIN_4
#define BUTTON_RIGHT            GPIO_PIN_0

#define BUTTONS_ALL             (BUTTON_LEFT | BUTTON_RIGHT)

void cButtonsInit()
{
  SysCtlPeripheralEnable(BUTTON_GPIO_PERIPH);
  
  HWREG(BUTTON_GPIO_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
  HWREG(BUTTON_GPIO_BASE + GPIO_O_CR) |= 0x01;
  HWREG(BUTTON_GPIO_BASE + GPIO_O_LOCK) = 0;
  
  GPIODirModeSet(BUTTON_GPIO_BASE, BUTTONS_ALL, GPIO_DIR_MODE_IN);
  GPIOPadConfigSet(BUTTON_GPIO_BASE, BUTTONS_ALL,
		    GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

bool cButtonPressed(uint8_t button)
{
  return(!GPIOPinRead(BUTTON_GPIO_BASE, button));
}

#endif