#define PART_TM4C123GH6PM 1

#include "camargo/common.h"
#include "camargo/led.h"
#include "camargo/rainbow.h"

#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_i2c.h"
#include "inc/hw_uart.h"
#include "driverlib/i2c.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"


#define UART_BAUDRATE (460800)

#define OV7670_ADDR 0x21

#define OV7670_CTL_PORT GPIO_PORTB_BASE
#define OV7670_VSYNC_PIN GPIO_PIN_0
#define OV7670_HREF_PIN GPIO_PIN_1
#define OV7670_PCLK_PIN GPIO_PIN_4

#define FB_W 160
#define FB_H 120
#define FB_D 1

#define CLOCK_FREQ 80000000

uint8_t fb[FB_W * FB_H * FB_D];

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

inline uint8_t readI2C0(uint16_t device_address, uint16_t device_register)
{
  
   //clear I2C FIFOs
   HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
   
   //specify that we want to communicate to device address with an intended write to bus
   I2CMasterSlaveAddrSet(I2C0_BASE, device_address, false);

   //the register to be read
   I2CMasterDataPut(I2C0_BASE, device_register);

   //send control byte and register address byte to slave device
   I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

   //wait for MCU to complete send transaction
   while(I2CMasterBusy(I2C0_BASE));

   //read from the specified slave device
   I2CMasterSlaveAddrSet(I2C0_BASE, device_address, true);

   //send control byte and read from the register from the MCU
   I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

   //wait while checking for MCU to complete the transaction
   while(I2CMasterBusy(I2C0_BASE));

   //Get the data from the MCU register and return to caller
   return( I2CMasterDataGet(I2C0_BASE));
 }
 
 inline void writeI2C0(uint16_t device_address, uint16_t device_register, uint8_t device_data)
{
  
   //clear I2C FIFOs
   HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
   
   //specify that we want to communicate to device address with an intended write to bus
   I2CMasterSlaveAddrSet(I2C0_BASE, device_address, false);

   //register to be read
   I2CMasterDataPut(I2C0_BASE, device_register);

   //send control byte and register address byte to slave device
   I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

   //wait for MCU to finish transaction
   while(I2CMasterBusy(I2C0_BASE));

   I2CMasterSlaveAddrSet(I2C0_BASE, device_address, false);

   //specify data to be written to the above mentioned device_register
   I2CMasterDataPut(I2C0_BASE, device_data);

   //wait while checking for MCU to complete the transaction
   I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

   //wait for MCU & device to complete transaction
   while(I2CMasterBusy(I2C0_BASE));
}


void uart0sendHex(uint8_t byte)
{
  static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  
  UARTCharPut(UART0_BASE, hex[byte >> 4]);
  UARTCharPut(UART0_BASE, hex[byte & 0x0f]);
}

void uart0send(uint8_t chr)
{
  UARTCharPut(UART0_BASE, chr);
}

#define uart0putc(x) uart0send((x))

void uart0puts(const char *c)
{
  while(*c)
  {
    uart0putc(*c);
    c++;
  }
}

void uart0ln()
{
  UARTCharPut(UART0_BASE, '\r');
  UARTCharPut(UART0_BASE, '\n');
}

void UARTsetup()
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  UARTConfigSetExpClk(UART0_BASE, CLOCK_FREQ, UART_BAUDRATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  
  uart0ln();  
}


#define OV7670Write(x, y) writeI2C0(OV7670_ADDR, (x), (y))
#define OV7670Read(x) readI2C0(OV7670_ADDR, (x))

void OV7670Setup()
{
  unsigned char tmp;
  
   
  // SETUP CAMERA PINS 
   
  // Control Pins
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, OV7670_VSYNC_PIN | OV7670_HREF_PIN | OV7670_PCLK_PIN );
  
  // Parallel Pins
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 );
  
  
  // SETUP CAMERA CLOCK
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

  GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
  GPIOPinConfigure(GPIO_PB6_M0PWM0);
  SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
  
  PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_STOP);
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 8);
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 3);
  
  PWMGenEnable(PWM0_BASE, PWM_GEN_0);
  PWMOutputState(PWM0_BASE,  PWM_OUT_0_BIT, true);
  
  
  // SETUP CAMERA I2C
  
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
  GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
   
  I2CMasterInitExpClk(I2C0_BASE, CLOCK_FREQ, false);
  I2CMasterEnable(I2C0_BASE);

  SysCtlDelay(CLOCK_FREQ / 10); // 100ms waiting for I2C and camera to stabilize
  
  //clear I2C FIFOs
  HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000; 
  
  // RESET THE CAMERA
  writeI2C0(OV7670_ADDR, 0x12, 0x80);
  SysCtlDelay(CLOCK_FREQ / 100); // 10ms waiting for camera to reset
  
  // READ ID FROM I2C
  uart0puts("OV");
  uint8_t idHigh = readI2C0(OV7670_ADDR, 0x0A);
  uint8_t idLow = readI2C0(OV7670_ADDR, 0x0B);
  uart0sendHex(idHigh);
  uart0sendHex(idLow);
  uart0send('-');
  uint8_t manHigh = readI2C0(OV7670_ADDR, 0x1C);
  uint8_t manLow = readI2C0(OV7670_ADDR, 0x1D);
  uart0sendHex(manHigh);
  uart0sendHex(manLow);
  uart0ln();
  
  // SETUP CAMERA REGS
 
  // CLKRC register: Prescaler = 2
  tmp = OV7670Read(0x11);
  OV7670Write(0x11, (tmp & 0x80) | 0x01);
 
  // DBLV register: PLL = 6
  //tmp = readOV7670(0x6B);
  //writeOV7670(0x6B, (tmp & 0b00111111) | 0b10000000);


  // COM3 register: Enable format scaling
  OV7670Write(0x0C, 0x08);
  uart0sendHex(OV7670Read(0x0C));
  
  // QVGA format
  // COM7 register: Select QVGA format
  OV7670Write(0x12, 0x14);
  uart0sendHex(OV7670Read(0x12));
  uart0ln();
  
  
}

inline void OV7670WaitFrameStart()
{
  while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_VSYNC_PIN)){} // wait for current frame to end (block while low)
  while(GPIOPinRead(OV7670_CTL_PORT, OV7670_VSYNC_PIN)){} // wait for new frame to start (block while high)
}


inline void setup()
{
  
  
  UARTsetup();
  
  OV7670Setup();
  
}

void captureFrame()
{
  uint16_t line = 0;
  uint16_t pixel = 0;
  uint8_t* bufPos = fb;
  
  OV7670WaitFrameStart();
  
  while(line != FB_H)
  {
    while(GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // if high in beginning wait for it to be low
    while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // wait for href to rise
   
    while(pixel != FB_W)
    {

      // this section is a bit slow
      // it misses every other pixel
      // This is fine though
      
      while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_PCLK_PIN)){} // wait pixel clock to rise
      
      while(GPIOPinRead(OV7670_CTL_PORT, OV7670_PCLK_PIN)){} // wait pixel clock to go down if hasn't yet
      while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_PCLK_PIN)){} // wait pixel clock to rise
      
      *bufPos = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7); // read msbs
      
      while(GPIOPinRead(OV7670_CTL_PORT, OV7670_PCLK_PIN)){} // wait pixel clock to go down if hasn't yet
      
      bufPos++; // advance in buffer by one byte
      pixel++;
    }
  
    pixel = 0;
    line++;
    
    // we are going to discard the next line
    while(GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // if high in beginning wait for it to be low
    while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // wait for href to rise
  }
}


void streamFrame()
{
  uint16_t line = 0;
  uint16_t pixel = 0;
  
  uart0puts("\r\nFRAME\r\n");
  while(UARTBusy(UART0_BASE)){} // wait in place
  
  OV7670WaitFrameStart();
  
  while(line != FB_H)
  {
    while(GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // if high in beginning wait for it to be low
    while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // wait for href to rise
   
    while(pixel != FB_W)
    {

      // this section is a bit slow
      // it misses every other pixel
      // This is fine though
      
      while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_PCLK_PIN)){} // wait pixel clock to rise
      
      HWREG(UART0_BASE + UART_O_DR) = HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + ((GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7) << 2))); // read msbs
      
      while(GPIOPinRead(OV7670_CTL_PORT, OV7670_PCLK_PIN)){} // wait pixel clock to go down if hasn't yet
      
      pixel++;
    }
  
    pixel = 0;
    line++;
    
    // we are going to discard the next line
    while(GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // if high in beginning wait for it to be low
    while(!GPIOPinRead(OV7670_CTL_PORT, OV7670_HREF_PIN)){} // wait for href to rise
  }
}

void printFrame()
{
  uart0ln();
  uart0ln(); 
  uart0ln(); 
  uart0ln(); 
  uart0puts("FRAMEA\r\n"); 
  for(uint8_t y = 0; y < FB_H; y+=6)
  {
    for(uint8_t x = 0; x < FB_W; x+=3)
    {
      UARTCharPut(UART0_BASE, (" .':;!+>ictJ5SM#")[fb[(y*FB_W+x)*FB_D]>>4]);
    }
    uart0ln();
  }
}

void sendFrame()
{
  uart0puts("\r\nFRAME\r\n");
  for(uint8_t y = 0; y < FB_H; y++)
  {
    for(uint8_t x = 0; x < FB_W; x++)
    {
      for(uint8_t p = 0; p < FB_D; p++)
	UARTCharPut(UART0_BASE, fb[(y*FB_W+x) + p]);
    }
  }
}

inline void loop()
{
  while(UARTCharsAvail(UART0_BASE))
  {
    char cmd = (char) UARTCharGetNonBlocking(UART0_BASE);
    
    cLedSet(LED_G | LED_R);
    switch(cmd)
    {
      case 'f':
	captureFrame();
	break;
      case 'p':
	printFrame();
	break;
      case 'c':
	while(!UARTCharsAvail(UART0_BASE))
	{
	  captureFrame();
	  printFrame();
	}
	break;
      case 'C':
	while(!UARTCharsAvail(UART0_BASE))
	{
	  captureFrame();
	  sendFrame();
	}
	break;
      case 's':
	while(!UARTCharsAvail(UART0_BASE))
	{
	  streamFrame();
	}
	break;
    }
    cLedSet(LED_G);
  }
}


int main(void) 
{
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

  cLedInit();
  cLedSet(LED_R);

  setup();
  
  cLedSet(LED_G);
  while(1)
  {
      loop();
  }
}
