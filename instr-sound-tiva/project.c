#define PART_TM4C1230C3PM 1

#include "camargo/common.h"
#include "camargo/led.h"
#include "camargo/color.h"
#include "camargo/rainbow.h"

#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/pin_map.h"

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void ADC0Seq1Handler();
void UART0Handler();
void TimerMainIntHandler();

uint32_t adcData[4];

CTimer tmrMain;

// 115200 baud, 8b, Np, 1s
// 11520 bytes/s
// 8 bit per sample
// about 10 KHz
// let's use that!
#define TX_FREQ 22050
#define BAUDRATE 921600

int main(void)
{
  
  SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
		  SYSCTL_OSC_MAIN);
  
  IntMasterEnable();
  
  cLedInit();
  cLedSet(LED_R);
  
  // UART CONFIG
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), BAUDRATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  
  UARTIntRegister( UART0_BASE, UART0Handler );
  UARTIntEnable( UART0_BASE, UART_INT_RX | UART_INT_RT );
  
  // ADC CONFIG
  
  cLedSet(LED_G|LED_R);
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
  
  ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH4|ADC_CTL_IE|ADC_CTL_END);
  ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
  ADCSequenceEnable(ADC0_BASE, 1);
  
  ADCHardwareOversampleConfigure(ADC0_BASE, 1);
  
  // ADC INTERRUPT
  
  IntEnable(INT_ADC0SS1);
  ADCIntRegister(ADC0_BASE, 1, ADC0Seq1Handler);
  ADCIntEnable(ADC0_BASE, 1);
  
  IntMasterEnable();
  
  // SAMPLING TIMER
  
  cTimerCreate(&tmrMain, SysCtlClockGet() / TX_FREQ, TimerMainIntHandler);
  
  // MAIN LOOPS
  
  cLedSet(LED_G);
  
  
  while(1)
  {
    // do nothing :)
  }
}

void ADC0Seq1Handler()
{
  ADCIntClear(ADC0_BASE, 1);
  
  uint32_t count = ADCSequenceDataGet(ADC0_BASE, 1, adcData);
  
  if(count && !UARTBusy(UART0_BASE))
  {
    UARTCharPutNonBlocking(UART0_BASE, adcData[0] >> 4);
  }
  
  cLedSet(LED_B | LED_G);
}

void UART0Handler()
{
  uint32_t status = UARTIntStatus(UART0_BASE, true); 
  
  UARTIntClear( UART0_BASE, status );
  
  if(status & UART_INT_RX)
  {
    while(UARTCharsAvail(UART0_BASE)) 
    {
      uint8_t in = (uint8_t) UARTCharGetNonBlocking(UART0_BASE);
    }
  }
  
  if(status & UART_INT_TX)
  {
    // do nothing here
  }
}

void TimerMainIntHandler()
{
  cTimerIntClear(&tmrMain);
  
  if(!ADCBusy(ADC0_BASE))
  {
    ADCProcessorTrigger(ADC0_BASE, 1);
    cLedSet(LED_B);
  }
  else
  {
    cLedSet(LED_R|LED_G);  
  }
}
