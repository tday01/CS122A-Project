#include <avr/io.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"

int main(void)
{
	
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	
	initUSART(0);
   
    while (1) 
    {
			//transmit//
			
			static unsigned char x = 1;
			
			if(USART_IsSendReady(0)){
			
			if(~PINA & 0x01){
				if (x < 8)
				{
					x = x << 1;
				}
				else if (x >= 8)
				{
					x = 1;
				}
			
				USART_Send(x, 0);
			}
			_delay_ms(1500);
			
			//long - 32 bits
			//int/short - 16 bits
			
			}
			
						
			//receive//
			if(USART_HasReceived(0))
			{
				PORTC = USART_Receive(0);
				USART_Flush(0);
			}
			
			_delay_ms(1000);
			
    }
}
