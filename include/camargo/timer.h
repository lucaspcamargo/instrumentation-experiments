#ifndef CAMARGO_TIMER_H
#define CAMARGO_TIMER_H

#include "common.h"
#include "utils/ringbuf.h"
#include "driverlib/timer.h"
#include "inc/hw_timer.h"


uint8_t _cTimer_buf[8] = {0,0,0,0,0,0,0,0};
const uint32_t _cTimer_baseArr[] = {TIMER0_BASE, TIMER1_BASE, TIMER2_BASE, TIMER3_BASE,
				    TIMER4_BASE, TIMER5_BASE, TIMER6_BASE, TIMER7_BASE
};

typedef struct CTimer_s {
  
  uint32_t base;
  uint8_t index;
  
} CTimer;

void cTimerInit()
{
  static bool initialized = false;
  
  if(!initialized)
  {
    // init code goes here
    
    initialized = true;
  }
  
}

int cTimerCreate(CTimer * timer, uint32_t duration, void (*handler)())
{
  cTimerInit();
  
  int ret = -1;
  
  for(int i = 0; i < 8; i++)
  {
    if(!_cTimer_buf[i])
    {
      _cTimer_buf[i] = 1;
      
      timer->index = i;
      timer->base = _cTimer_baseArr[i];
      
      SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0 + i);
      TimerConfigure(_cTimer_baseArr[i], TIMER_CFG_PERIODIC);
      TimerLoadSet(_cTimer_baseArr[i], TIMER_A, duration);
      
      TimerIntRegister(_cTimer_baseArr[i], TIMER_A, handler);
      TimerIntEnable(_cTimer_baseArr[i], TIMER_TIMA_TIMEOUT);
      
      TimerEnable(_cTimer_baseArr[i], TIMER_A);
      
      
      ret = 0;
      break;
    }
  }
  
  return ret;
}

int cTimerCreateOneShot(CTimer * timer, uint32_t duration, void (*handler)())
{
  int ret = cTimerCreate(timer, duration, handler);
  if(!ret)
  {
    TimerDisable(timer->base, TIMER_A);
    TimerConfigure(timer->base, TIMER_CFG_ONE_SHOT);
    TimerEnable(timer->base, TIMER_A);
  }
  
  return ret;
}

uint32_t cTimerIntClear(CTimer * timer)
{
  uint32_t status = TimerIntStatus(timer->base, true);
  
  TimerIntClear(timer->base, status);
  
  return status;
}

void cTimerEnable(CTimer * timer)
{
  TimerEnable(timer->base, TIMER_A);
}

void cTimerDisable(CTimer * timer)
{
  TimerDisable(timer->base, TIMER_A);
}

#endif 
