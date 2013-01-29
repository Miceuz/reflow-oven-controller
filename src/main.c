#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "LiquidCrystal.h"

static int uart_putchar(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static int lcd_putchar(char c, FILE *stream);
static FILE lcd = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

static int uart_putchar(char c, FILE *stream) {
  if (c == '\n') {
    uart_putchar('\r', stream);
  }
  loop_until_bit_is_set(UCSRA, UDRE);
  UDR = c;
  return 0;
}

static int lcd_putchar(char c, FILE *stream) {
  LCDwrite(c);
  return 0;
}

void uart_init(void) {
  #define BAUD 9600
  #include <util/setbaud.h>
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  
#if USE_2X
  UCSRA |= _BV(U2X);
#else
  UCSRA &= ~(_BV(U2X));
#endif
  
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0); /* 8-bit data */ 
  UCSRB = _BV(RXEN) | _BV(TXEN);   /* Enable RX and TX */
}

int main (void) {
  DDRC |= _BV(PC3);
  uart_init();
  stdout = &mystdout;

  LiquidCrystal();
  LCDbegin(8, 2);
  
//  fputs("GRIBAS", &lcd);
  fprintf(&lcd, "GRIB");
  uint16_t c = 0;
  while(1) {
    PORTC |= _BV(PC3);
    _delay_ms(500);
    PORTC &= ~_BV(PC3);
    _delay_ms(500);
    puts("Hello, world!\n");
    LCDsetCursor(0,1);
    fprintf(&lcd, "%d", c++);
  }
}
