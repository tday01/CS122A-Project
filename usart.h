#ifndef USART_H
#define USART_H

#define F_CPU 8000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALE (((F_CPU / (BAUD_RATE * 16UL))) - 1)

void initUSART(unsigned char usartNum)
{
	// USART 0, with interrupts
	if (usartNum != 1) {
		UCSR0B |= (1 << RXEN0)  | (1 << TXEN0) | (1 << RXCIE0);
		UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);

		UBRR0L = BAUD_PRESCALE;
		UBRR0H = (BAUD_PRESCALE >> 8);
	}
	
	// USART 1, no interrupts
	else {
		UCSR1B |= (1 << RXEN1)  | (1 << TXEN1);
		UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11);
		
		UBRR1L = BAUD_PRESCALE;
		UBRR1H = (BAUD_PRESCALE >> 8);
	}
	
	// enable interrupts
	sei();
}

void USART_Send(unsigned char sendMe, unsigned char usartNum)
{
	if (usartNum != 1) {
		while( !(UCSR0A & (1 << UDRE0)) );
		UDR0 = sendMe;
	}
	else {
		while( !(UCSR1A & (1 << UDRE1)) );
		UDR1 = sendMe;
	}
}

unsigned char USART_Receive(unsigned char usartNum)
{
	if (usartNum != 1) {
		while ( !(UCSR0A & (1 << RXC0)) );
		return UDR0;
	}
	else {
		while ( !(UCSR1A & (1 << RXC1)) );
		return UDR1;
	}
}

void USART_Flush(unsigned char usartNum)
{
	static unsigned char temp;
	if (usartNum != 1) {
		while ( UCSR0A & (1 << RXC0) ) { temp = UDR0; }
	}
	else {
		while ( UCSR1A & (1 << RXC1) ) { temp = UDR1; }
	}
}

unsigned char USART_HasTransmitted(unsigned char usartNum)
{
	return (usartNum != 1) ? (UCSR0A & (1 << TXC0)) : (UCSR1A & (1 << TXC1));
}

unsigned char USART_HasReceived(unsigned char usartNum)
{
	return (usartNum != 1) ? (UCSR0A & (1 << RXC0)) : (UCSR1A & (1 << RXC1));
}

unsigned char USART_IsSendReady(unsigned char usartNum)
{
	return (usartNum != 1) ? (UCSR0A & (1 << UDRE0)) : (UCSR1A & (1 << UDRE1));
}

#endif //USART_H
