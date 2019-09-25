#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "scheduler.h"
#include "timer.h"
//xb
#include <unistd.h>
#include <stdio.h>
#include "usart.h"
//lcd
#include "nokia5110.h"
#include <util/delay.h>
//gas
#include "ccs811.h"
#include "twi.h"
//eeprom
#include <avr/eeprom.h>
//emic
#include "emic.h"
//xbee
#include "xbee.h"
//clock
#include "clock.h"
// cvt
#include "makeString.h"

#define A0 (~PINA & 0x01)
#define A1 (~PINA & 0x02)
#define A2 (~PINA & 0x04)
#define A3 (~PINA & 0x20)
#define A4 (~PINA & 0x40)
#define A5 (~PINA & 0x08)
#define A6 (~PINA & 0x80)

#define xbee 0
#define emic 1

// 6 bytes of data, 1 byte header, 1 byte termination
#define BUFFER_SIZE 6

// ---- Global Vars Begin ---- //

//select thresh value to change
unsigned char threshSelect = 0;

// alarm triggered flag, only gasSensor SM can change
unsigned char soundAlarm = 0;

// alarm test
unsigned char testAlarm = 0;

// alarm enabled/disabled
unsigned char alarmStatus = 0;

// rxBuffer
unsigned char rxBuffer = 0;
unsigned char rxTemp = 0;

// clock
unsigned long minutes = 0;
unsigned long hours = 0;
unsigned long twoSecs = 0;
unsigned long timeout = 0;

// time weighted average of exposure to vocs
unsigned long exposure_TWA;

// tvoc/eco2 values
#define TVOC_LIMIT 5500
#define ECO2_LIMIT 5000
uint16_t tvoc_thresh;
uint16_t eco2_thresh;
#define tvoc_save_inc (eeprom_update_word((uint16_t*)46,(tvoc_thresh+=((uint16_t)0x0032))))
#define eco2_save_inc (eeprom_update_word((uint16_t*)48,(eco2_thresh+=((uint16_t)0x0032))))
#define tvoc_save_dec (eeprom_update_word((uint16_t*)46,(tvoc_thresh-=((uint16_t)0x0032))))
#define eco2_save_dec (eeprom_update_word((uint16_t*)48,(eco2_thresh-=((uint16_t)0x0032))))
// CommsHandler transmit error flag
unsigned char txErrBT = 0;

// gasSensor readings
static struct ccs811_data_t data;

unsigned char rxData = 0;

unsigned char rxFlag = 0;

unsigned char updateFlag = 0;

unsigned char newData[BUFFER_SIZE];

unsigned char pauseMainScreen = 0;

unsigned long exposAccum  = 0;

// received data
unsigned long rxMinutes = 0;
unsigned long rxEposure_TWA = 0;
unsigned long rxTvocs  = 0;

unsigned char rxAlert = 0;


// ---- Global Vars End ---- //

// ---------------- SMs Begin ---------------- //
enum Button_States {Button_start, Button_wait, Button_test, Button_select, Button_reset, Button_requestRX, Button_exposure, Button_inc, Button_dec, Button_arm};
enum CommsHandler_States {CommsHandler_init, CommsHandler_wait, CommsHandler_txUpdate, CommsHandler_analyze, CommsHandler_alarm};
enum ExposureMonitor_States {ExposureMonitor_start, ExposureMonitor_Wait, ExposureMonitor_calculate};
enum LCD_States {LCD_init, LCD_Wait, LCD_Update};
enum GasSensor_States {GasSensor_init, GasSensor_wait, GasSensor_update};

int TickFct_Button(int state){	
	
	switch(state){	//transitions
		
		case Button_start:
		testAlarm = 0;
		state = Button_wait;
		break;
		
		case Button_wait:
		if(!A0 && !A1 && !A2 && A3 && !A4 && !A5 && A6)
		{
			pauseMainScreen = 1;
			state = Button_requestRX;
		}
		else if(A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			emic_send_str("Alarm Test");
			state = Button_test;
		}
		
		else if(!A0 && A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			threshSelect = !threshSelect;
			state = Button_select;
		}
		
		else if(!A0 && !A1 && !A2 && !A3 && !A4 && A5 && !A6)
		{
			alarmStatus = !alarmStatus;
			state = Button_arm;
		}
		
		else if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && A6)
		{
			pauseMainScreen = 1;
			state = Button_exposure;
		}
		
		else if(!A0 && !A1 && A2 && !A3 && !A4 && !A5 && !A6)
		{
			testAlarm = 0;
			soundAlarm = 0;
			alarmStatus = 0;
			PORTD &= 0x7F; // reset vibrate
			eeprom_update_word((uint16_t*)46, (uint16_t)0x02EE);
			eeprom_update_word((uint16_t*)48, (uint16_t)0x028A);
			eeprom_update_word((uint16_t*)50, (uint16_t)0x0000);
			eeprom_update_word((uint16_t*)52, (uint16_t)0x0000);
			eeprom_update_word((uint16_t*)54, (uint16_t)0x0000);
			eeprom_update_word((uint16_t*)56, (uint16_t)0x0000);
			eeprom_update_word((uint16_t*)58, (uint16_t)0x0000);
			minutes = 0;
			hours = 0;
			exposure_TWA = 0;
			twoSecs = 1;
			exposAccum = 0;
			LCD_5110_init();
			LCD_5110_clear();
			LCD_5110_write_string("CS122A Project ", 1);
			LCD_5110_set_position(0, 15);
			LCD_5110_write_string("  Air Quality", 1);
			LCD_5110_set_position(0, 23);
			LCD_5110_write_string("    Sensor", 1);
			LCD_5110_set_position(0, 40);
			LCD_5110_write_string("  By Tim Day", 1);
			LCD_5110_print();
			_delay_ms(1000);
			state = Button_reset;
		}
		
		else if(!A0 && !A1 && !A2 && A3 && !A4 && !A5 && !A6)
		{
			//increment eeprom tvoc or eco2
			if (threshSelect)
			{
				if(tvoc_thresh <= (TVOC_LIMIT - 50))
				{
					tvoc_save_inc;
				}
			}
			else if (!threshSelect)
			{
				if(eco2_thresh <= (ECO2_LIMIT - 50))
				{
					eco2_save_inc;
				}
			}
			state = Button_inc;
		}
		
		else if(!A0 && !A1 && !A2 && !A3 && A4 && !A5 && !A6)
		{
			if (threshSelect)
			{
				if(tvoc_thresh > 50)
				{
					tvoc_save_dec;
				}
			}
			else if (!threshSelect)
			{
				if(eco2_thresh > 50)
				{
					eco2_save_dec;
				}
			}
			state = Button_dec;
		}
		else if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			state = Button_wait;
		}
		else
		{
			//do nothing
		}
		
		break;
		
		case Button_test:
		if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			PORTD |= 0x7F; // vibrate off
			soundAlarm = 0;
			state = Button_wait;
		}
		else
		{
			PORTD |= 0x80; // vibrate on
			state = Button_test;
		}
		break;
		
		case Button_select:
		if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			state = Button_wait;
		}
		else
		{
			state = Button_select;
		}
		break;
		
		case Button_arm:
		if(!A0 && !A1 && !A2 && !A3 && !A4 && A5 && !A6)
		{
			state = Button_arm;
		}
		else
		{
			state = Button_wait;
		}
		break;
		
		case Button_reset:
		if(!A0 && !A1 && A2 && !A3 && !A4 && !A5 && !A6)
		{
			state = Button_reset;
		}
		else
		{
			state = Button_wait;
		}
		break;
		
		case Button_exposure:
		if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{	
			pauseMainScreen = 0;
			state = Button_wait;
		}
		else
		{
			// Exposure data LCD
			LCD_5110_clear();
			LCD_5110_set_position(9, 0);
			LCD_5110_write_string("-Exposure-", 1);
			
			LCD_5110_set_position(15, 13);
			LCD_5110_write_string("TWA:", 1);
			
			if(exposure_TWA <= 0)
			{
				LCD_5110_write_string("0", 1);
			}
			else
			{
				LCD_5110_print_num(exposure_TWA);
			}
		
			LCD_5110_set_position(15, 24);
			LCD_5110_write_string("Tmins:", 1);
			
			(minutes < 1) ? LCD_5110_write_string("0", 1) : LCD_5110_print_num(minutes);
			
			LCD_5110_set_position(15, 34);
			LCD_5110_write_string("Thours:", 1);
			
			(hours < 1) ? LCD_5110_write_string("0", 1) : LCD_5110_print_num(hours);
		
			LCD_5110_print();
			
			state = Button_exposure;
		}
		break;
		
		case Button_inc:
		if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			state = Button_wait;
		}
		else
		{
			state = Button_inc;
		}
		break;
		
		case Button_dec:
		if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			state = Button_wait;
		}
		else
		{	
			state = Button_dec;
		}
		break;
		
		case Button_requestRX:
		if(!A0 && !A1 && !A2 && !A3 && !A4 && !A5 && !A6)
		{
			pauseMainScreen = 0;
			state = Button_wait;
		}
		else
		{
			// request data frame
			xbee_send(0xAA, 0xAA, 0xAA, 0xAA);
			
			LCD_5110_clear();
			LCD_5110_set_position(9, 0);
			LCD_5110_write_string("-RemoteData-", 1);
			
			LCD_5110_set_position(15, 13);
			LCD_5110_write_string("TWA:", 1);
			
			if(rxEposure_TWA <= 0)
			{
				LCD_5110_write_string("0", 1);
			}
			else
			{
				LCD_5110_print_num(rxEposure_TWA);
			}
			
			LCD_5110_set_position(15, 24);
			LCD_5110_write_string("Tmins:", 1);
			
			(rxMinutes < 1) ? LCD_5110_write_string("0", 1) : LCD_5110_print_num(rxMinutes);
			
			LCD_5110_set_position(15, 34);
			LCD_5110_write_string("Vocs:", 1);
			
			(rxTvocs < 1) ? LCD_5110_write_string("0", 1) : LCD_5110_print_num(rxTvocs);
			
			LCD_5110_print();
			
			state = Button_requestRX;
		}
		break;
		
		default:
		state = Button_wait;
		break;
	}
		
	switch(state){	//actions
		
		case Button_wait:
		PORTD &= 0x7F; // vibrate
		testAlarm = 0;
		break;
		
		case Button_test:
		if (!soundAlarm)
		{
			PORTD |= 0x80; // vibrate
			testAlarm = 1;
		}
		
		break;
		
		case Button_select:
		
		break;
		
		case Button_reset:
		
		break;
		
		case Button_inc:
	
		break;
		
		case Button_dec:
		
		break;
		
		case Button_exposure:
		
		break;
		
		case Button_requestRX:
		
		break;
		
		default:
		break;
		
	}
		
	return state;	
}


// CommsHandler SM
int TickFct_CommsHandler(int state) {
	
	unsigned char count = 1;
	unsigned char countBits = 0;
	unsigned char chkSum = 0;
	unsigned char temp = 0;

	switch(state) {
        case CommsHandler_init:
		
            state = CommsHandler_wait;
            break;
        
		case CommsHandler_wait:
            if(soundAlarm || testAlarm)
            {
                state = CommsHandler_alarm;
            }
			else if(rxFlag == 1)
			{
				//rxFlag = 0;
				state = CommsHandler_analyze;
			}
			else if(updateFlag == 1)
			{
				//updateFlag = 0;
				state = CommsHandler_txUpdate;
			}
            else
            {
                state = CommsHandler_wait;
            }
            break;
       
        case CommsHandler_alarm:
            if(!soundAlarm && !testAlarm)
            {
                state = CommsHandler_wait;
            }
            else
            {
                state = CommsHandler_alarm;
            }
            break;
		case CommsHandler_analyze:
			if(rxFlag == 0)
			{
				rxAlert = 0;
				soundAlarm = 0;
				state = CommsHandler_wait;
			}
			break;
		case CommsHandler_txUpdate:
			if(updateFlag == 0)
			{
				state = CommsHandler_wait;
			}
			break;

		default:
            state = CommsHandler_init;
            break;
	}

	switch(state) {
		case CommsHandler_init:

            break;
            
        case CommsHandler_wait:		
			
            break;
            
        case CommsHandler_txUpdate:
			
			updateFlag = 0;

            break;
			
		case CommsHandler_alarm:
			
			xbee_send(0xFF,0xFF,0xFF,0xFF);
			
            break;
		
		case CommsHandler_analyze:
			
			// generate checksum
			for(; count < 4; ++count)
			{
				temp = newData[count];
				
				for(; countBits < 8; ++countBits)
				{
					if(temp & 0x01)
					{
						++chkSum;
					}
					
					temp = temp >> 1;
				}
			}
			
			// validate new checksum with received checksum
			if(newData[4] != chkSum)
			{
				emic_send_str("checksum error");
				// request new data frame
				xbee_send(0xAA, 0xAA, 0xAA, 0xAA);
				
			}
			// check valid frame structure
			else if((newData[0] != 0xFF) || (newData[5] != 0x0A))
			{
				emic_send_str("invalid frame");
				// request new data frame
				xbee_send(0xAA, 0xAA, 0xAA, 0xAA);
			}
			
			if((newData[1] & newData[2] & newData[3]) == 0xff)
			{
				soundAlarm = 1;
				rxAlert = 1;
				//_delay_ms(500);
				newData[1] = 0x00;
			}	
			else if((newData[1] & newData[2] & newData[3]) == 0xAA)
			{
				xbee_send(exposure_TWA, minutes, (char)data.tvoc, 0x00);
			}
			else
			{
			rxEposure_TWA = newData[1];
			rxMinutes = newData[2];
			rxTvocs = newData[3];
			}
		
			// data processed, now close flag
			rxFlag = 0;

			break;
		
		default:
            break;
	}

	return state;
	
}

// ExposureMonitor SM
int TickFct_ExposureMonitor(int state) {
   
	static unsigned long timePassed;
	static unsigned long tmp;
	unsigned char n;
	unsigned char c;
	unsigned char buf[n+1];
    
    switch (state) {
        case ExposureMonitor_start:
		
			exposure_TWA = eeprom_read_word((uint16_t*)50);
			twoSecs = eeprom_read_word((uint16_t*)54);
			exposAccum = eeprom_read_word((uint16_t*)52);
			minutes = eeprom_read_word((uint16_t*)56);
			hours = eeprom_read_word((uint16_t*)58);
            
			state = ExposureMonitor_Wait;
            break;
            
        case ExposureMonitor_Wait:

            state = ExposureMonitor_calculate;
           
            break;
            
        case ExposureMonitor_calculate:
   
			PORTD &= 0x7F; // vibrate off
            state = ExposureMonitor_Wait;
          
            break;
            
        default:
            state = ExposureMonitor_start;
            break;
    }
    
    switch (state) {
        case ExposureMonitor_calculate:
			
			// prevent error from reset
			if(tmp > twoSecs)
			{
				tmp = 0;
			}
			
			timePassed = twoSecs - tmp;
			
			// allow 4 secs after startup for first calculation
			if(twoSecs >= 2)
			{
				exposAccum += (data.tvoc)* (timePassed);
				
				eeprom_update_word((uint16_t*)52,(uint16_t)exposAccum);
				eeprom_update_word((uint16_t*)54,(uint16_t)twoSecs);
				eeprom_update_word((uint16_t*)56,(uint16_t)minutes);
				eeprom_update_word((uint16_t*)58,(uint16_t)hours);
				
				exposure_TWA = exposAccum/(twoSecs);
			}
			
			if(exposure_TWA > 500)
			{
				emic_send_str("Danger T W A threshold violated");
				PORTD |= 0x80; // vibrate on
			}
			
			tmp = twoSecs;
			
            break;
            
        case ExposureMonitor_Wait:
	
            break;
            
        default:
            break;
            
    }
    
    return state;
}

// LCD SM
int TickFct_LCD(int state) {

	switch(state) {
		
		case LCD_init:
			
		LCD_5110_init();
		LCD_5110_clear();
		LCD_5110_write_string("CS122A Project ", 1);
		LCD_5110_set_position(0, 15);
		LCD_5110_write_string("  Air Quality", 1);
		LCD_5110_set_position(0, 23);
		LCD_5110_write_string("    Sensor", 1);
		LCD_5110_set_position(0, 40);
		LCD_5110_write_string("  By Tim Day", 1);
		LCD_5110_print();
		_delay_ms(3000);
		state = LCD_Wait;
		break;
		
		case LCD_Wait:
		PORTA &= 0xEF;
		if(soundAlarm || testAlarm)
		{
			PORTA |= 0x10;
			PORTD |= 0x80; // vibrate on
			state = LCD_Update;
		}
		else if (!A0 && !A1 && A2 && !A3 && !A4)
		{
			state = LCD_init;
		}
		else
		{
			state = LCD_Wait;
		}
		break;
		
		case LCD_Update:
		if(!soundAlarm && !testAlarm)
		{
			PORTD |= 0x7F; // vibrate off
			state = LCD_Wait;
		}
		else
		{
			state = LCD_Update;
		}
		break;

		default:
		state = LCD_init;
		break;
	}

	switch(state) {
		case LCD_init:

		break;
		
		case LCD_Wait:
		
		if(pauseMainScreen == 0)
		{
		LCD_5110_clear();
		LCD_5110_set_position(9, 0);
		alarmStatus ? LCD_5110_write_string("Alarm: ON", 1) : LCD_5110_write_string("Alarm: OFF", 1);

		if(threshSelect)
		{
			LCD_5110_set_position(4, 11);
			LCD_5110_write_string("*", 1);
		}

		LCD_5110_set_position(9, 11);
		LCD_5110_write_string("ThreshT:", 1);
		LCD_5110_print_num(eeprom_read_word((uint16_t*)46));
		
		if(!threshSelect)
		{
			LCD_5110_set_position(4, 19);
			LCD_5110_write_string("*", 1);
		}
		
		LCD_5110_set_position(9, 19);
		LCD_5110_write_string("ThreshC:", 1);
		LCD_5110_print_num(eeprom_read_word((uint16_t*)48));
		LCD_5110_set_position(0, 31);
		LCD_5110_write_string("  TVOCs:", 1);
		if(data.tvoc <= 0){LCD_5110_write_string("detect",1);}
		else {LCD_5110_print_num(data.tvoc);}
		LCD_5110_set_position(0, 39);
		LCD_5110_write_string("  CO2:", 1);
		if(data.eco2 <= 0){LCD_5110_write_string("detect",1);}
		else {LCD_5110_print_num(data.eco2);}
		LCD_5110_print();
		}
		break;
		
		case LCD_Update:
		
		if(pauseMainScreen == 0)
		{
			if(rxAlert)
			{
				LCD_5110_clear();
				LCD_5110_set_position(0, 5);
				LCD_5110_write_string("   Alarm", 1);
				LCD_5110_set_position(0, 13);
				LCD_5110_write_string("      Remote", 1);
			}
			else
			{
			LCD_5110_clear();
			LCD_5110_set_position(0, 5);
			(testAlarm) ? LCD_5110_write_string("   Alarm", 1) : LCD_5110_write_string("  Unsafe", 1);
			
			LCD_5110_set_position(0, 13);
			(testAlarm) ? LCD_5110_write_string("      Test", 1) : LCD_5110_write_string("   Environment", 1);
			}
		
			PORTD |= 0x80; // vibrate on
		
			if(!testAlarm && !rxAlert)
			{
				tvoc_thresh = eeprom_read_word((uint16_t*)46);
				eco2_thresh = eeprom_read_word((uint16_t*)48);
			
				if (data.tvoc > tvoc_thresh)
				{
					PORTD |= 0x80; // vibrate on
					emic_send_str("Warning threshold violated");
					LCD_5110_set_position(0, 31);
					LCD_5110_write_string("->", 1);

				}
				LCD_5110_set_position(12, 31);
				LCD_5110_write_string("TVOCs:", 1);
				if(data.tvoc <= 0){LCD_5110_write_string("~0",1);}
				else {LCD_5110_print_num(data.tvoc);}
				
				
				if(data.eco2 > eco2_thresh)	
				{
					PORTD |= 0x80; // vibrate on
					emic_send_str("Warning threshold violated");
					LCD_5110_set_position(0, 39);
					LCD_5110_write_string("->", 1);
				}
				
				LCD_5110_set_position(13, 39);
				LCD_5110_write_string("CO2:", 1);
				LCD_5110_print_num(data.eco2);
			}
		
		
		LCD_5110_print();

		}
		
		break;
		
		default:
		break;
		
	}
	
	return state;
}

// gasSensor SM
int TickFct_GasSensor(int state) {

	//static unsigned char count = 0;

	switch(state) {
		case GasSensor_init:
		//data.eco2 = 0;
		//data.tvoc = 0;
		tvoc_thresh = eeprom_read_word((uint16_t*)46);
		eco2_thresh = eeprom_read_word((uint16_t*)48);
		ccs811_init();
		
		state = GasSensor_wait;
		break;
		
		case GasSensor_wait:
		if(ccs811_poll() && !soundAlarm && !testAlarm)
		{
			state = GasSensor_update;
		}
		else
		{
			state = GasSensor_wait;
		}
		break;
		
		case GasSensor_update:
		//update data from sensor
		ccs811_read_data(&data);
	
		state = GasSensor_wait;
		break;

		default:
		state = GasSensor_init;
		break;
	}

	switch(state) {
		case GasSensor_init:

		break;
		
		case GasSensor_wait:
		
		tvoc_thresh = eeprom_read_word((uint16_t*)46);
		eco2_thresh = eeprom_read_word((uint16_t*)48);
		
		uint16_t temp_tvoc = data.tvoc;
		uint16_t temp_eco2 = data.eco2;
		
		if ( (tvoc_thresh < temp_tvoc) || (eco2_thresh < temp_eco2) ||
			(TVOC_LIMIT < temp_tvoc || ECO2_LIMIT < temp_eco2) )
			{
				if(alarmStatus)
				{
					PORTD |= 0x80; // vibrate on
					soundAlarm = 1;
				}
			}
		else if (soundAlarm && !A0 && !A1 && A2 && !A3 && !A4)
			{
				PORTD &= 0x7F; // vibrate off
				soundAlarm = 0;
			}

		break;
		
		case GasSensor_update:
		
		break;
		
		default:
		break;
	}

	return state;
}

// ---------------- SMs End ---------------- //

int main(void)
{   
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0XFF; PORTB = 0X00;
	DDRA = 0X00; PORTA = 0xFF;
	
	initUSART(emic);
	_delay_ms(20);
	USART_Flush(emic);
	_delay_ms(20);
	initUSART(xbee);
	_delay_ms(20);
	USART_Flush(xbee);
	_delay_ms(20);
	
	init_emic();
	_delay_ms(20);
	init_xbee();
	_delay_ms(20);
	
	init_clock();
	
	unsigned long int CommsHandler_Task_calc = 500;
    unsigned long int ExposureMonitor_Task_calc = 1000;
	unsigned long int LCD_Task_calc = 250;
	unsigned long int GasSensor_Task_calc = 1000;
	unsigned long int Button_Task_calc = 100;
	
	unsigned long int tmpGCD = 1;
	
	tmpGCD = findGCD(CommsHandler_Task_calc, ExposureMonitor_Task_calc);
	tmpGCD = findGCD(tmpGCD, LCD_Task_calc);
	tmpGCD = findGCD(tmpGCD, GasSensor_Task_calc);
	tmpGCD = findGCD(tmpGCD, Button_Task_calc);

	unsigned long int GCD = tmpGCD;

	unsigned long int CommsHandler_period = CommsHandler_Task_calc/GCD;
    unsigned long int ExposureMonitor_period = ExposureMonitor_Task_calc/GCD;
	unsigned long int LCD_period = LCD_Task_calc/GCD;
	unsigned long int GasSensor_period = GasSensor_Task_calc/GCD;
	unsigned long int Button_period = Button_Task_calc/GCD;

	static task CommsHandler_Task, ExposureMonitor_Task, LCD_Task, GasSensor_Task, Button_Task;
	task *tasks[] = { &CommsHandler_Task, &ExposureMonitor_Task, &LCD_Task, &GasSensor_Task, &Button_Task};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	CommsHandler_Task.state = CommsHandler_init;
	CommsHandler_Task.period = CommsHandler_period;
	CommsHandler_Task.elapsedTime = CommsHandler_period;
	CommsHandler_Task.TickFct = &TickFct_CommsHandler;
	
    ExposureMonitor_Task.state = ExposureMonitor_start;
    ExposureMonitor_Task.period = ExposureMonitor_period;
    ExposureMonitor_Task.elapsedTime = ExposureMonitor_period;
    ExposureMonitor_Task.TickFct = &TickFct_ExposureMonitor;
	
	LCD_Task.state = LCD_init;
	LCD_Task.period = LCD_period;
	LCD_Task.elapsedTime = LCD_period;
	LCD_Task.TickFct = &TickFct_LCD;
	
	GasSensor_Task.state = GasSensor_init;
	GasSensor_Task.period = GasSensor_period;
	GasSensor_Task.elapsedTime = GasSensor_period;
	GasSensor_Task.TickFct = &TickFct_GasSensor;
	
	Button_Task.state = Button_start;
	Button_Task.period = Button_period;
	Button_Task.elapsedTime = Button_period;
	Button_Task.TickFct = &TickFct_Button;

	TimerSet(GCD);
	TimerOn();

	unsigned short i;

	while(1)
	{
		for(i = 0; i < numTasks; i++) {

			if(tasks[i]->elapsedTime == tasks[i]->period) {

				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);

				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 1;
}

// USART receive xbee
ISR(USART0_RX_vect)
{
	//cli();
	
	unsigned char rxTemp = 0;
	static unsigned char countRX = 0;
	static unsigned char txCount = 0; // track x 3 transmissions
	static unsigned char txValid = 0;
	
	rxTemp = UDR0;
	
	newData[countRX] = rxTemp;
	++countRX;
	
	if(countRX > (BUFFER_SIZE-1))
	{
		rxFlag = 1;
		countRX = 0;
	}
	
	//sei();

}

// clock
ISR(TIMER0_OVF_vect)
{
	static long ticksSecs = 0;
	static long ticksHrs = 0;
	static long ticksMins = 0;
	
	ticksSecs++;
	ticksMins++;
	ticksHrs++;
	
	// hours
	if (ticksHrs == 105000)
	{
		hours++;
		ticksHrs = 0;
	}

	// minutes
	if (ticksMins == 1750)
	{
		minutes++;
		ticksMins = 0;
	}
	
	// 2 seconds
	if (ticksSecs == 58)
	{
		twoSecs++;
		ticksSecs = 0;
	}
}




