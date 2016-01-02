#ifndef CAMARGO_RAINBOW_H
#define CAMARGO_RAINBOW_H

#include "timer.h"
#include "led.h"
#include "color.h"

#ifndef RAINBOW_INTERVAL
#define RAINBOW_INTERVAL 3000
#endif

CTimer _cRainbow_timer;
int _cRainbow_auto = 1;
cColor _cRainbow_ledColor = {20, 10, 0};

void cRainbowIntTimerHandler();

int cRainbowInit()
{
  cLedInit();
  
  if(cTimerCreate(&_cRainbow_timer, RAINBOW_INTERVAL, cRainbowIntTimerHandler))
  {
    // error creating timer
    return -1;
  }
  
  return 0;
}

void cRainbowStart()
{
  cTimerEnable(&_cRainbow_timer);
}

void cRainbowStop()
{
  cTimerDisable(&_cRainbow_timer);
}

void cRainbowSetAutomatic(bool automatic)
{
  _cRainbow_auto = automatic? 1 : 0;
}

void cRainbowSetColor(cColor color)
{
  _cRainbow_ledColor = color;
}

void cRainbowIntTimerHandler()
{
  
  /*uint32_t status = */ cTimerIntClear(&_cRainbow_timer);
  
  static const int P = 1;
  static int counter = 0;
  static float hue = 0.0f;
  
  
  counter += 1;
    
  if(counter >= (P * 255))
  {
    counter = 0;
    
    if(_cRainbow_auto)
    {
      hue += 0.01f;
    
      if(hue >= 1.0f)
      {
	hue -= 1.0f;
      }
      
      cColorHSL2RGB(hue, 1.0f, 0.5f, &_cRainbow_ledColor);
    }
  }
  
  cLedSet( (counter % 256 < (_cRainbow_ledColor.r)? LED_R : 0) + 
  (counter % 256 < (_cRainbow_ledColor.g * P)? LED_G : 0) + 
  (counter % 256 < (_cRainbow_ledColor.b * P)? LED_B : 0) );
}

#endif 
