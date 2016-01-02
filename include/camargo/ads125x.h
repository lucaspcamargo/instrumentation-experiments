#ifndef CAMARGO_ADS125X
#define CAMARGO_ADS125X

#include "ssi.h"
#include "common.h"
#include "led.h"

#define ADS125X_CS_GPIO_PERIPH SYSCTL_PERIPH_GPIOA
#define ADS125X_CS_GPIO_BASE GPIO_PORTA_BASE
#define ADS125X_CS_GPIO_PIN GPIO_PIN_7

#define ADS125X_DRDY_GPIO_PERIPH SYSCTL_PERIPH_GPIOA
#define ADS125X_DRDY_GPIO_BASE GPIO_PORTA_BASE
#define ADS125X_DRDY_GPIO_PIN GPIO_PIN_3

#define ADS125X_RESET_GPIO_PERIPH SYSCTL_PERIPH_GPIOB
#define ADS125X_RESET_GPIO_BASE GPIO_PORTB_BASE
#define ADS125X_RESET_GPIO_PIN GPIO_PIN_0
#define ADS125X_RESET_PERIOD_INV 100

#define ADS125x_CMD_WAKEUP      0x00
#define ADS125x_CMD_RDATA       0x01
#define ADS125x_CMD_RDATAC      0x03
#define ADS125x_CMD_SDATAC      0x0F
#define ADS125x_CMD_RREG_MASK   0x10
#define ADS125x_CMD_WREG_MASK   0x50
#define ADS125x_CMD_SELFCAL     0xF0
#define ADS125x_CMD_SELFOCAL    0xF1
#define ADS125x_CMD_SELFGCAL    0xF2
#define ADS125x_CMD_SYSOCAL     0xF3
#define ADS125x_CMD_SYSGCAL     0xF4
#define ADS125x_CMD_SYNC        0xFC
#define ADS125x_CMD_STANDBY     0xFD
#define ADS125x_CMD_RESET       0xFE
#define ADS125x_CMD_WAKEUP_2    0xFF


#define ADS125X_REG_STATUS      0x00
#define ADS125X_REG_MUX         0x01
#define ADS125X_REG_ADCON       0x02
#define ADS125X_REG_DRATE       0x03
#define ADS125X_REG_IO          0x04
#define ADS125X_REG_OFC0        0x05
#define ADS125X_REG_OFC1        0x06
#define ADS125X_REG_OFC2        0x07
#define ADS125X_REG_FSC0        0x08
#define ADS125X_REG_FSC1        0x09
#define ADS125X_REG_FSC2        0x0A
 
// GLOBALS
 
cSSIConfig _cADS125X_config = {1000000, 8, SSI_FRF_MOTO_MODE_1}; // bitrate whatever, 10 data bits
bool _cADS125X_initialized = 0;
bool _cADS125X_continuous = 0;
void (*_cADS125X_drdyFn)(void) = 0;

// DECLARATIONS
void cADS125XReset();
void cADS125XSelfCalibrate();
void cADS125XSendCmd(uint8_t cmd);
uint32_t cADS125XDataReady();
void cADS125XWaitDataReady();
void cADS125XWriteReg(uint8_t reg, int n, uint8_t * val);

// IMPLEMENTATION

int cADS125XInit()
{
  if(_cADS125X_initialized) return 0;
  
  if(cSSIInit()) return -1;
  
  SysCtlPeripheralEnable(ADS125X_CS_GPIO_PERIPH);
  GPIOPinTypeGPIOOutput(ADS125X_CS_GPIO_BASE, ADS125X_CS_GPIO_PIN);
  GPIOPinWrite(ADS125X_CS_GPIO_BASE, ADS125X_CS_GPIO_PIN, ADS125X_CS_GPIO_PIN);
  
  SysCtlPeripheralEnable(ADS125X_RESET_GPIO_PERIPH);
  GPIOPinTypeGPIOOutput(ADS125X_RESET_GPIO_BASE, ADS125X_RESET_GPIO_PIN);
  
  if(ADS125X_DRDY_GPIO_PERIPH != ADS125X_CS_GPIO_PERIPH)
    SysCtlPeripheralEnable(ADS125X_DRDY_GPIO_PERIPH);
  GPIOPinTypeGPIOInput(ADS125X_DRDY_GPIO_BASE, ADS125X_DRDY_GPIO_PIN);
  GPIOPadConfigSet(ADS125X_DRDY_GPIO_BASE, ADS125X_DRDY_GPIO_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
  cADS125XReset();
  cADS125XSelfCalibrate();
  
  uint8_t val = 0x82; 
  cADS125XWriteReg(0x03, 1, &val); // 100 SPS
  val = 0x0f;
  cADS125XWriteReg(0x01, 1, &val); // ain 0 aincom dif
  
  _cADS125X_initialized = true;
  return 0;
}

void cADS125XReset()
{
  cLedSet(LED_R);
  GPIOPinWrite(ADS125X_RESET_GPIO_BASE, ADS125X_RESET_GPIO_PIN, 0);
  SysCtlDelay(SysCtlClockGet() / ADS125X_RESET_PERIOD_INV);
  GPIOPinWrite(ADS125X_RESET_GPIO_BASE, ADS125X_RESET_GPIO_PIN, 1);
  SysCtlDelay(SysCtlClockGet() / ADS125X_RESET_PERIOD_INV);
  cLedSet(0);
}

void signal();

void cADS125XSelfCalibrate()
{
  cLedSet(LED_R);
  cADS125XSendCmd(ADS125x_CMD_SELFCAL);
  cADS125XWaitDataReady();
  cLedSet(0);
}


void cADS125XSelect(bool select)
{
  GPIOPinWrite(ADS125X_CS_GPIO_BASE, ADS125X_CS_GPIO_PIN, select? 0 : 0xff);
}

uint32_t cADS125XDataReady()
{
  return GPIOPinRead(ADS125X_DRDY_GPIO_BASE, ADS125X_DRDY_GPIO_PIN) & ADS125X_DRDY_GPIO_PIN? false : true;
}

void cADS125XWaitDataReady()
{
  while(!cADS125XDataReady())
  {
    SysCtlDelay(1);
  }
}

inline void cADS125XPostCmdDelay()
{
  SysCtlDelay(SysCtlClockGet() / 160000 );
}


void cADS125XSendCmd(uint8_t cmd)
{
  cSSIEnsureConfig(&_cADS125X_config);
  cSSIClean();

  cADS125XSelect(true);
  cSSIBitDelay();
  
  cSSIWrite(cmd);
  cSSIWait();
  cSSIBitDelay();
  cADS125XSelect(false);
}

void cADS125XReadReg(uint8_t reg, int n, uint8_t * dest)
{
  cSSIEnsureConfig(&_cADS125X_config);
  cSSIClean();

  cADS125XSelect(true);
  cSSIBitDelay();
  
  cSSIWrite(ADS125x_CMD_RREG_MASK | (reg & 0x0f)); // issu command with start addr 
  cSSIWrite(n & 0x0f); // number of registers to read
  cSSIWait();
  
  uint32_t buf;
  for(int i = 0; i < n; i++)
  {
    cSSIRead(&buf);
    dest[i] = (uint8_t) buf;
  }
  
  cSSIWait();
  cSSIBitDelay();
  cADS125XSelect(false);
}

void cADS125XWriteReg(uint8_t reg, int n, uint8_t * val)
{
  cSSIEnsureConfig(&_cADS125X_config);
  cSSIClean();

  cADS125XSelect(true);
  cSSIBitDelay();
  
  cSSIWrite(ADS125x_CMD_WREG_MASK | (reg & 0x0f)); // issu command with start addr 
  cSSIWrite(n & 0x0f); // number of registers to write
  
  for(int i = 0; i < n; i++)
  {
    cSSIWrite(val[i]);
  }
  
  cSSIWait();
  cSSIBitDelay();
  cADS125XSelect(false);
}


uint32_t cADS125XSample()
{ 
  uint32_t high, mid, low;
  uint32_t trashBin[8];
  
  cSSIEnsureConfig(&_cADS125X_config);
  cSSIClean();
  
  cLedSet(LED_B);
  cADS125XWaitDataReady();
  cLedSet(0);
  
  cADS125XSelect(true);
  cSSIBitDelay();
  
  cLedSet(LED_G);
  cSSIWrite(ADS125x_CMD_RDATA);
  cSSIWait();
  while(SSIDataGetNonBlocking(SSI0_BASE, trashBin)) {
  } // empty fifo
  
  SSIDataPut(CSSI_B, 0);
  SSIDataPut(CSSI_B, 0);
  SSIDataPut(CSSI_B, 0);
  cSSIWait();
  SSIDataGet(CSSI_B, &high);
  SSIDataGet(CSSI_B, &mid);
  SSIDataGet(CSSI_B, &low);
  cLedSet(0);
  
  cSSIBitDelay();
  cADS125XSelect(false);
  
  return ((high & 0xff) << 16) | ((mid & 0xff) << 8) | ((low & 0xff) << 0);
}

#endif