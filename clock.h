#ifndef CLOCK_H
#define CLOCK_H 

void init_clock(void)
{
	cli();

	TCNT0 = 0x00;
	TCCR0B |= 0x05;
 
	TIMSK0  |= _BV(TOIE0);

	sei();
}

#endif //CLOCK_H