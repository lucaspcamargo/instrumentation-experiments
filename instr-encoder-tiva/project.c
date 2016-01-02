#define PART_TM4C123GH6PM 1

#define BAUDRATE 115200
#define USE_QEI 0

#include "camargo/common.h"
#include "camargo/led.h"

#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"
#include "inc/hw_timer.h"
#include "inc/hw_gpio.h"

#if USE_QEI
#include "driverlib/qei.h"
#endif

#define GPIO_ENC_CLK GPIO_PIN_7
#define GPIO_ENC_DT GPIO_PIN_6


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void GPIOHandler(void);
void UART0Handler();
void tmrDebouncerIntHandler(void);
void QEI0Handler(void);

int debounceFlag = 0;
uint32_t debouncerPeriod;

inline void setup()
{ 
  
  // UART CONFIG
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), BAUDRATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  
  UARTIntRegister( UART0_BASE, UART0Handler );
  UARTIntEnable( UART0_BASE, UART_INT_RX | UART_INT_RT );
  
  UARTCharPutNonBlocking(UART0_BASE, 'I');   
  
  // PULSE COUNTING CONFIG
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; //In Tiva include this is the same as "_DD" in older versions (0x4C4F434B)
  HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
  HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;
  
#if USE_QEI
  
  GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_7 | GPIO_PIN_6, GPIO_DIR_MODE_IN);
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
  GPIOPinConfigure(GPIO_PD6_PHA0);
  GPIOPinConfigure(GPIO_PD7_PHB0);
  GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
  
  QEIDisable(QEI0_BASE);
  
  QEIIntDisable(QEI0_BASE, QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);
  QEIConfigure(QEI0_BASE, QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_NO_RESET | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP, 1000);
  QEIPositionSet(QEI0_BASE, 500);
  
  QEIEnable(QEI0_BASE);
  
#else
  
  
  GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_ENC_CLK | GPIO_ENC_DT);
  
  // TIMER DEBOUNCE CFG
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
  TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
  debouncerPeriod = SysCtlClockGet() / 4000; // 0.25ms
  TimerLoadSet(TIMER1_BASE, TIMER_A, debouncerPeriod-1);
  
  TimerIntRegister(TIMER1_BASE, TIMER_A, tmrDebouncerIntHandler );
  TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
  
  TimerEnable(TIMER1_BASE, TIMER_A);

#endif
  
  // MOTOR PWM CONFIG
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  
  GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
  GPIOPinConfigure(GPIO_PB6_M0PWM0);
  SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
  
  PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_STOP);
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 1024);
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 1);
  
  PWMGenEnable(PWM0_BASE, PWM_GEN_0);
  PWMOutputState(PWM0_BASE,  PWM_OUT_0_BIT, true);
  
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0);

  
}

#if !USE_QEI
/* returns change in encoder state (-1,0,1) */
int8_t read_encoder(uint8_t newState)
{
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;               //remember previous state
  old_AB |= newState & 0x03;  //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}
#endif

void loop()
{ 
#if USE_QEI
  
  uint32_t pos = QEIPositionGet(QEI0_BASE);
  QEIPositionSet(QEI0_BASE, 500);
  
  if(pos != 500)
  {
    int diff = pos-500;
    
    while(diff)
    {
      UARTCharPut(UART0_BASE, diff > 0? 'R' : 'L');
      cLedSet(diff < 0? LED_B : LED_R | LED_G);
      
      diff += diff>0? -1 : 1;
    }
    
  }
  
#else
  static uint8_t prevState = 0;
  uint8_t newState = ( (GPIOPinRead(GPIO_PORTD_BASE, GPIO_ENC_DT)? 0x00 : 0x01 )  | (GPIOPinRead(GPIO_PORTD_BASE, GPIO_ENC_CLK)? 0x00 : 0x02 ));
  
  if(prevState != newState)
  {
    TimerDisable(TIMER1_BASE, TIMER_A);
    TimerLoadSet(TIMER1_BASE, TIMER_A, debouncerPeriod-1);
    TimerEnable(TIMER1_BASE, TIMER_A);
    debounceFlag = 1;
  }
  else{
    if(!debounceFlag)
    {
      int8_t stateChange = read_encoder(newState);
      if( stateChange ) {
	UARTCharPutNonBlocking(UART0_BASE, stateChange < 0? 'L' : 'R');
	cLedSet(stateChange < 0? LED_B : LED_R | LED_G);
      }
    }
  }
  
  prevState = newState;

#endif
  
  while(UARTCharsAvail(UART0_BASE)) 
    {
      uint8_t in = (uint8_t) UARTCharGet(UART0_BASE);
      //UARTCharPutNonBlocking(UART0_BASE, in); //echo  
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, (uint32_t) 1 + (in/255.0)* 1023 );
    }
}


int main(void)
{
  
  SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

  cLedInit();
  cLedSet(LED_R);

  setup();
  
  cLedSet(LED_G);
  while(1)
  {
    loop();
  }
}



void UART0Handler()
{
  uint32_t status = UARTIntStatus(UART0_BASE, true); 
  
  UARTIntClear( UART0_BASE, status );
  
  if(status & UART_INT_RX)
  {
    // do nothing here too
    
  }
  
  if(status & UART_INT_TX)
  {
    // do nothing here
  }
}

void tmrDebouncerIntHandler(void)
{
  TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
  
  debounceFlag = 0;
  
}
