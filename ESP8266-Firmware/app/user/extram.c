/*
 * Copyright 2016 Piotr Sperka (http://www.piotrsperka.info)
*/

#include "extram.h"
#include "esp_common.h"
#include "spi.h"

void extramInit() {
	gpio16_output_conf();
	gpio16_output_set(1);
}

uint32_t extramRead(uint32_t size, uint32_t address, uint8_t *buffer) {
	uint32_t i = 0;
	spi_take_semaphore();
	gpio16_output_set(0);
	SPIPutChar(0x03);
	SPIPutChar((address>>16)&0xFF);
	SPIPutChar((address>>8)&0xFF);
	SPIPutChar(address&0xFF);
	for(i = 0; i < size; i++) {
		buffer[i] = SPIGetChar();
	}
	gpio16_output_set(1);
	spi_give_semaphore();
	return i;
}

uint32_t extramWrite(uint32_t size, uint32_t address, uint8_t *data) {
	uint32_t i = 0;
	spi_take_semaphore();
	gpio16_output_set(0);
	SPIPutChar(0x02);
	SPIPutChar((address>>16)&0xFF);
	SPIPutChar((address>>8)&0xFF);
	SPIPutChar(address&0xFF);
	for(i = 0; i < size; i++) {
		SPIPutChar(data[i]);
	}
	gpio16_output_set(1);
	spi_give_semaphore();
	return i;
}

