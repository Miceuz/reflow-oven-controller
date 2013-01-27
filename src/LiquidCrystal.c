#include "LiquidCrystal.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define HIGH 0x1
#define LOW  0x0

void send(uint8_t, uint8_t);
void write4bits(uint8_t);
void write8bits(uint8_t);
void pulseEnable();
void _digitalWrite(uint8_t, uint8_t);

uint8_t _rs_pin; // LOW: command.  HIGH: character.
uint8_t _enable_pin; // activated by a HIGH pulse.
uint8_t _data_pins[8];

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;

uint8_t _initialized;

uint8_t _numlines,_currline;

uint8_t _SPIbuff;

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

void LiquidCrystal() {
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  // the SPI expander pinout
  _rs_pin = 2;// 1;
  _enable_pin = 3;//2;
  _data_pins[0] = 4;//6;  // really d4
  _data_pins[1] = 5;//5;  // really d5
  _data_pins[2] = 6;//4;  // really d6
  _data_pins[3] = 7;//3;  // really d7
  
  setSSOutput();
  _SPIbuff = 0;
  
  setupSPI();
  // we can't begin() yet :(
  LCDbegin(16,2);
}

/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
void delayMicroseconds(unsigned int us)
{
	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);
  
#if F_CPU >= 16000000L
	// for the 16 MHz clock on most Arduino boards
  
	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;
  
	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;
  
	// account for the time taken in the preceeding commands.
	us -= 2;
#else
	// for the 8 MHz internal clock on the ATmega168
  
	// for a one- or two-microsecond delay, simply return.  the overhead of
	// the function calls takes more than two microseconds.  can't just
	// subtract two, since us is unsigned; we'd overflow.
	if (--us == 0)
		return;
	if (--us == 0)
		return;
  
	// the following loop takes half of a microsecond (4 cycles)
	// per iteration, so execute it twice for each microsecond of
	// delay requested.
	us <<= 1;
  
	// partially compensate for the time taken by the preceeding commands.
	// we can't subtract any more than this or we'd overflow w/ small delays.
	us--;
#endif
  
	// busy wait
	__asm__ __volatile__ (
                        "1: sbiw %0,1" "\n\t" // 2 cycles
                        "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
                        );
}

void LCDbegin(uint8_t cols, uint8_t lines) {
  _SPIbuff = 0x80; // backlight
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _currline = 0;

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  delayMicroseconds(50000); 
  // Now we pull both RS and R/W low to begin commands
  _digitalWrite(_rs_pin, LOW);
  _digitalWrite(_enable_pin, LOW);
  
  //put the LCD into 4 bit or 8 bit mode
  // this is according to the hitachi HD44780 datasheet
  // figure 24, pg 46

  // we start in 8bit mode, try to set 4 bit mode
  write4bits(0x03);
  delayMicroseconds(4500); // wait min 4.1ms

  // second try
  write4bits(0x03);
  delayMicroseconds(4500); // wait min 4.1ms
  
  // third go!
  write4bits(0x03); 
  delayMicroseconds(150);

  // finally, set to 8-bit interface
  write4bits(0x02); 

  // finally, set # lines, font size, etc.
  LCDcommand(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  LCDdisplayOn();

  // clear it off
  LCDclear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  LCDcommand(LCD_ENTRYMODESET | _displaymode);

}

/********** high level commands, for the user! */
void LCDclear()
{
  LCDcommand(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void LCDhome()
{
  LCDcommand(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void LCDsetCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row > _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  
  LCDcommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCDdisplayOff() {
  _displaycontrol &= ~LCD_DISPLAYON;
  LCDcommand(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCDdisplayOn() {
  _displaycontrol |= LCD_DISPLAYON;
  LCDcommand(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCDcursorOff() {
  _displaycontrol &= ~LCD_CURSORON;
  LCDcommand(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCDcursorOn() {
  _displaycontrol |= LCD_CURSORON;
  LCDcommand(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LCDblinkOff() {
  _displaycontrol &= ~LCD_BLINKON;
  LCDcommand(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCDblinkOn() {
  _displaycontrol |= LCD_BLINKON;
  LCDcommand(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCDscrollDisplayLeft(void) {
  LCDcommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCDscrollDisplayRight(void) {
  LCDcommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCDleftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  LCDcommand(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LCDrightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  LCDcommand(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LCDautoscrollOn(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  LCDcommand(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LCDautoscrollOff(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  LCDcommand(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCDcreateChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  LCDcommand(LCD_SETCGRAMADDR | (location << 3));
  int i;
  for (i = 0; i < 8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void LCDcommand(uint8_t value) {
  send(value, LOW);
}

inline void LCDwrite(uint8_t value) {
  send(value, HIGH);
}

/************ low level data pushing commands **********/

// little wrapper for i/o writes
void  _digitalWrite(uint8_t p, uint8_t d) {
  if (d == HIGH)
    _SPIbuff |= (1 << p);
  else 
    _SPIbuff &= ~(1 << p);

  ssLow();
  //shiftOut(_SPIdata, _SPIclock, MSBFIRST,_SPIbuff);
  transferSPI(_SPIbuff);
  ssHigh();
}

// Allows to set the backlight, if the LCD backpack is used
void setBacklight(uint8_t status) {
  _digitalWrite(7, status); // backlight is on pin 7
}

// write either command or data, with automatic 4/8-bit selection
void send(uint8_t value, uint8_t mode) {
  _digitalWrite(_rs_pin, mode);
  
  write4bits(value>>4);
  write4bits(value);
}

void pulseEnable(void) {
  _digitalWrite(_enable_pin, LOW);
  delayMicroseconds(1);    
  _digitalWrite(_enable_pin, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450ns
  _digitalWrite(_enable_pin, LOW);
  delayMicroseconds(100);   // commands need > 37us to settle
}

void write4bits(uint8_t value) {
  int i;
  for (i = 0; i < 4; i++) {
    _digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  pulseEnable();
}
