#ifndef BIT_H
#define BIT_H

unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

unsigned char GetBit(unsigned char port, unsigned char number) 
{
	return ( port & (0x01 << number) );
}

#endif //BIT_H
