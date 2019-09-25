#ifndef SCHEDULER_H
#define SCHEDULER_H

unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a % b;
		if( c == 0 ) { return b; }
		a = b;
		b = c;
	}
	return 0;
}

typedef struct _task{
	signed 	 char state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int); 
} task;

#endif //SCHEDULER_H
