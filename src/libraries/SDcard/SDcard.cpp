#ifdef __cplusplus
extern "C" {
#endif
  #include <inttypes.h>
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include "ff.h"
  #include "diskio.h"
  #include "mmc.h"
#ifdef __cplusplus
}
#endif
#include "WProgram.h" 
#include "wiring.h"
#include "../digitalWriteFast/digitalWriteFast.h"
#include "SDcard.h"


//-------------------- Constructor --------------------


SDcard::SDcard(void)
{
  return;
}


//-------------------- Public --------------------


void SDcard::init(uint8_t clock_div)
{
  //init pins
#ifdef SD_PWR_PIN
  pinMode(SD_PWR_PIN,  OUTPUT);
  digitalWrite(SD_PWR_PIN, LOW); //power on
#endif
  pinMode(SD_CS_PIN,  OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH); //deselect
  pinMode(SD_MISO_PIN, INPUT);
  digitalWrite(SD_MISO_PIN, HIGH); //pull-up
  pinMode(SD_MOSI_PIN, OUTPUT);
  pinMode(SD_CLK_PIN, OUTPUT);

#if !defined(SD_SOFTWARE_SPI)
  //SS has to be output or input with pull-up
# if (defined(__AVR_ATmega1280__) || \
      defined(__AVR_ATmega1281__) || \
      defined(__AVR_ATmega2560__) || \
      defined(__AVR_ATmega2561__))     //--- Arduino Mega ---
#  define SD_SS_PORTBIT (0) //PB0
# else                                 //--- Arduino Uno ---
#  define SD_SS_PORTBIT (2) //PB2
# endif
  if(!(DDRB & (1<<SD_SS_PORTBIT))) //SS is input
  {
      PORTB |= (1<<SD_SS_PORTBIT); //pull-up on
  }

  //init hardware spi
  switch(clock_div)
  {
    case 2:
      SPCR = (1<<SPE)|(1<<MSTR); //enable SPI, Master, clk=Fcpu/4
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/2
      break;
    case 4:
      SPCR = (1<<SPE)|(1<<MSTR); //enable SPI, Master, clk=Fcpu/4
      SPSR = (0<<SPI2X); //clk*2 = off
      break;
    case 8:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //enable SPI, Master, clk=Fcpu/16
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/8
      break;
    case 16:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //enable SPI, Master, clk=Fcpu/16
      SPSR = (0<<SPI2X); //clk*2 = off
      break;
    case 32:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1); //enable SPI, Master, clk=Fcpu/64
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/32
      break;
  }
#endif

  return;
}


void SDcard::service(void)
{
  disk_timerproc();

  return;
}


uint8_t SDcard::mount(void)
{
  if(disk_initialize(0) & STA_NOINIT)
  {
    return 1;
  }

  if(f_mount(0, &fatfs) == FR_OK)
  {
    return 0;
  }

  return 2;
}


void SDcard::unmount(void)
{
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  return;
}

