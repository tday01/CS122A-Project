#ifndef EMIC_H
#define EMIC_H

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

void init_emic()
{
	/*
	USART_Send('P',emic); //parser
	
	USART_Send('1',emic);
	
	USART_Send('\n',emic);
	
	USART_Send('N',emic); //voice
	
	USART_Send('1',emic);
	
	USART_Send('\n',emic);
	*/
	USART_Send('V',emic); //volume max:18
	
	USART_Send('1',emic);
	
	USART_Send('8',emic);
	
	USART_Send('\n',emic);
	
	/*
	USART_Send('W',emic); // words per minute min:75
	
	USART_Send('1',emic);
	
	USART_Send('5',emic);
	
	USART_Send('0',emic);
	
	USART_Send('\n',emic);
	*/
}

#endif //EMIC_H