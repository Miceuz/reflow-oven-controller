#include <avr/io.h>
#include "spi.h"

void setSSOutput() {
  DDRB |= _BV(PB2);
  PORTB |= _BV(PB2);
}

void setupSPI() {
  //  // Set direction register for SCK and MOSI pin.
  //  // MISO pin automatically overrides to INPUT.
  //  // When the SS pin is set as OUTPUT, it can be used as
  //  // a general purpose output port (it doesn't influence
  //  // SPI operations).
  //  
  DDRB |= _BV(PB5);
  PORTB |= _BV(PB5);

  DDRB |= _BV(PB3);
  PORTB |= _BV(PB3);
  //  // Warning: if the SS pin ever becomes a LOW INPUT then SPI 
  //  // automatically switches to Slave, so the data direction of 
  //  // the SS pin MUST be kept as OUTPUT.
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);  
}

uint8_t transferSPI(uint8_t b) {
  SPDR = b;
  while (!(SPSR & _BV(SPIF)))
    ;
  return SPDR;  
}

void ssLow(){
  PORTB &=~ _BV(PB2);
}
void ssHigh(){
  PORTB |= _BV(PB2);
}
