#ifndef makeString_H
#define makeString_H

char* makeStr(unsigned long num)
{
	unsigned char n = 0;//length = 0;
	unsigned char c = 0;//tmp = 0;
	
	n = snprintf(NULL, 0, "%lu", num);
	
	unsigned char buf[n+1];
	
	c = snprintf(buf, n+1, "%lu", num);
	
	char *ptr = &buf;
	
	return ptr;
}

#endif //MAKESTRING_H
