#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "twi.h"
#define TWI_BAUD	100000UL

void twi_init()
{
	//PORTC |= (1 << 0) | (1 << 1);
	TWBR = 32; //((F_CPU / TWI_BAUD) - 16) / 2;
	TWSR = 0;
}

static inline uint8_t twi_ready()
{
	return TWCR & (1 << TWINT);
}

void twi_start()
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!twi_ready());
}

void twi_stop()
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	while (TWCR & (1 << TWSTO));
	_delay_us(100);
}

uint8_t twi_write(uint8_t data)
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!twi_ready());
	return 0;
}

uint8_t twi_read(uint8_t ack)
{
	TWCR = (1 << TWINT) | (ack ? (1 << TWEA) : 0) | (1 << TWEN);
	while (!twi_ready());
	return TWDR;
}

uint8_t twi_scan(uint8_t addr)
{
	twi_start();
	uint8_t ack = twi_write(addr);
	twi_stop();
	return ack;
}

uint8_t twi_write_data(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *ptr)
{
	uint8_t err = 0;
	twi_start();
	twi_write(addr << 1);
	twi_write(reg);
	while (len--){
	twi_write(*ptr);
	++ptr;
	}
	twi_stop();
	return !err;
}

uint8_t twi_read_data(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *ptr)
{
	uint8_t err = 0;
	twi_start();
	twi_write(addr << 1);
	twi_write(reg);

	// Repeated start
	twi_start();
	twi_write((addr << 1) + 1);

	while (len--){
	*ptr++ = twi_read(len);
	}
	twi_stop();
	return !err;
}