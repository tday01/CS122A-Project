#define F_CPU 8000000UL // Assume uC operates at 8MHz

#include <avr/io.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"
#include <string.h>
//#include <cstring.h>

#include <avr/interrupt.h>

#define xbee 0
#define emic 1

void emic_send_str(char* str)
{
	USART_Send('S', emic);
	
	unsigned int i = 0;
	while(str[i] != '\0')
	{
		while (!USART_IsSendReady(emic));
		USART_Send(str[i], emic);
		i++;
	}
	
	USART_Send('.', emic);
	
	USART_Send('\n', emic);
	
	while(USART_Receive(emic) != ':'){}

}

void emic_init(void)
{
	unsigned char x = 0;
	
	x = USART_Receive(emic);
	
	//if (USART_IsSendReady(emic))
	//{
		
	//if ( ((x == 0xA3) || (x == 0x3A)))
	//{
		
	
	USART_Send('P',emic);
	
	USART_Send('1',emic);
	
	USART_Send('\n',emic);
	
	
	USART_Send('N',emic);
	
	USART_Send('4',emic);
	
	USART_Send('\n',emic);
	
	
	USART_Send('W',emic);
	
	USART_Send('100',emic);
	
	USART_Send('\n',emic);
	
	
	USART_Send('V',emic);
	
	USART_Send('18',emic);
	
	USART_Send('\n',emic);
	
	//}
	
	//}
	
	return 0;
}

int main(void)
{
	
	DDRA = 0x00; PORTA = 0xFF;
	//DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	//DDRD = 0xFF; PORTD = 0x00;
	
	initUSART(xbee);
	USART_Flush(xbee);
	initUSART(emic);
	USART_Flush(emic);
	
	_delay_ms(3000);
	emic_init();
	
	while (1)
	{
		
		static unsigned long x = 65;
		
		static unsigned long y = 0;
		
		// transmit xbee if button press
		if(~PINA & 0x01){
			if (x < 90)
			{
				x += 1;
			}
			else if (x > 90)
			{
				x = 65;
			}
			
			
			USART_Send(x, xbee);
			
			_delay_ms(1000);
		}
		
		
		//receive xbee
		if(USART_HasReceived(xbee))
		{
			y = USART_Receive(xbee);
			USART_Flush(xbee);
			
			_delay_ms(1000);
			
		}
		
		_delay_ms(2000);

		
		USART_Send('S',emic);
		
		USART_Send(y,emic);
		
		USART_Send('\n',emic);

		
	}
		
	}
