#ifndef XBEE_H
#define XBEE_H

#define xbee 0
#define emic 1

void xbee_send(char tx1, char tx2, char tx3, char tx4)
{
	
	unsigned char count = 0;
	unsigned char countBits = 0;
	unsigned char chkSum = 0;
	unsigned char temp = 0;
	
	unsigned char buff[4];
	
	buff[0] = tx1;
	buff[1] = tx2;
	buff[2] = tx3;
	
	// generate checksum
	for(; count < 3; ++count)
	{
		temp = buff[count];
		
		for(; countBits < 8; ++countBits)
		{	
			if(temp & 0x01)
			{
				++chkSum;
			}
			
			temp = temp >> 1;
		}
	}
	
	USART_Send(0xFF,xbee);
	//_delay_ms(50);
	USART_Send(tx1,xbee);
	//_delay_ms(50);
	USART_Send(tx2,xbee);
	//_delay_ms(50);
	USART_Send(tx3,xbee);
	//_delay_ms(50);
	USART_Send(chkSum,xbee);
	//_delay_ms(50);
	USART_Send(0x0A,xbee);
	//_delay_ms(50);
}

void init_xbee()
{
	// TO_DO // customize network
	
	/*
	unsigned char temp = 0;
	
	USART_Send('+', xbee);
	USART_Send('+', xbee);
	USART_Send('+', xbee);
	USART_Send(0x0A, xbee);
	
	temp = USART_Receive(xbee);
	emic_send_str((char)temp);
	
	temp = USART_Receive(xbee);
	emic_send_str((char)temp);
	
	
	USART_Send('M', xbee);
	USART_Send('Y', xbee);
	USART_Send(0x0A, xbee);
	
	unsigned char high = 0;
	unsigned char low = 0;
	
	low = UDR0;
	emic_send_str((char)high);
	
	high = UDR0;
	emic_send_str((char)low);
	*/
}

#endif //XBEE_H
