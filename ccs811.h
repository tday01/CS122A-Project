#ifndef CCS811_H
#define CCS811_H
#include <stdint.h>

#define STATUS_REG 0x00
#define MEAS_MODE_REG 0x01
#define ALG_RESULT_DATA 0x02
#define ENV_DATA 0x05
#define NTC_REG 0x06
#define THRESHOLDS 0x10
#define BASELINE 0x11
#define HW_ID_REG 0x20
#define FW_BOOT_VER 0x23
#define FW_APP_VER 0x24
#define ERROR_ID_REG 0xE0
#define APP_START_REG 0xF4
#define SW_RESET 0xFF
#define CCS_811_ADDRESS 0x5B
#define GPIO_WAKE 0x5
#define DRIVE_MODE_IDLE 0x0
#define DRIVE_MODE_1SEC 0x10
#define DRIVE_MODE_10SEC 0x20
#define DRIVE_MODE_60SEC 0x30
#define INTERRUPT_DRIVEN 0x80
#define THRESHOLDS_ENABLED 0x40
#define CCS811_DATA_READY 0x08

	struct ccs811_data_t {
		uint16_t eco2;
		uint16_t tvoc;
		uint8_t errorID;
		uint8_t status;
		uint16_t raw;
	};
	
	void ccs811_init();
	uint8_t ccs811_read_status();
	uint8_t ccs811_poll();
	void ccs811_read_data(struct ccs811_data_t *ptr);

#endif // CCS811_h