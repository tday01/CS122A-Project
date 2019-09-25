#include "ccs811.h"
#include "twi.h"
#include <stdio.h>

void ccs811_init()
{
	uint8_t i2c_buff[8];
	
	twi_init();
	twi_read_data(CCS_811_ADDRESS, STATUS_REG, 1, i2c_buff);
	
	twi_write_data(CCS_811_ADDRESS, APP_START_REG, 0, i2c_buff);
	
	i2c_buff[0] = DRIVE_MODE_1SEC;
	
	twi_write_data(CCS_811_ADDRESS, MEAS_MODE_REG, 1, i2c_buff);
	
}


static inline uint8_t ccs811_read(uint8_t addr)
{
	uint8_t data;
	return twi_read_data(CCS_811_ADDRESS, addr, 1, &data) ? data : 0;
}

uint8_t ccs811_read_status()
{
	return ccs811_read(STATUS_REG);
}

uint8_t ccs811_poll()
{
	return ccs811_read_status() & CCS811_DATA_READY;
}

static inline uint16_t swap_u16(uint16_t data)
{
	return ((data & 0xff) << 8) | ((data >> 8) & 0xff);
}

void ccs811_read_data(struct ccs811_data_t *ptr)
{
	twi_read_data(CCS_811_ADDRESS, ALG_RESULT_DATA, sizeof(struct ccs811_data_t), (void *)ptr);
	ptr->eco2 = swap_u16(ptr->eco2);
	ptr->tvoc = swap_u16(ptr->tvoc);
	ptr->raw = swap_u16(ptr->raw);
}