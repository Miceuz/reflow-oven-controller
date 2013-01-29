#include <inttypes.h>
#ifndef _spi_h
#define spi_h
/**
 * Set Slave Select pin to output 
 */
void setSSOutput();
/**
 * Setup SPI
 */
void setupSPI();
/**
 * Transfer byte via SPI
 */
uint8_t transferSPI(uint8_t b);

void ssLow();
void ssHigh();

#endif