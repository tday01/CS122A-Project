#ifndef TWI_H
#define TWI_H

#include <stdint.h>

	void twi_init();
	void twi_start();
	void twi_stop();
	uint8_t twi_write(uint8_t data);
	uint8_t twi_read(uint8_t ack);
	uint8_t twi_scan(uint8_t addr);
	uint8_t twi_write_data(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *ptr);
	uint8_t twi_read_data(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *ptr);

#endif // TWI_H