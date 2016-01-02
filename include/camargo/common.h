#pragma once

#ifndef CAMARGO_NO_INCLUDES

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#endif

#define PINS_0_3 GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
#define PINS_1_4 GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
#define PINS_4_7 GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
#define PINS_0_7 GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7

