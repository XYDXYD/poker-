#include "msg_to_bit.h"

//color用消息里面的字符串，point为字符2 3 4 5 6 7 8 9 1 j q k，10用1代替 
unsigned int msg_to_bit(char color[], char point)
{
	unsigned int tmp = 0;
	int value;
	int primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 };
	if (strcmp(color, "SPADES") == 0)
		tmp = tmp | 0x1000;
	else if (strcmp(color, "HEARTS") == 0)
		tmp = tmp | 0x2000;
	else if (strcmp(color, "CLUBS") == 0)
		tmp = tmp | 0x8000;
	else if (strcmp(color, "DIAMONDS") == 0)
		tmp = tmp | 0x4000;
		
	if (point == 'J' || point == 'j') value = 9;
	else if (point == 'Q' || point == 'q') value = 10;
	else if (point == 'K' || point == 'k') value = 11;
	else if (point == 'A' || point == 'a') value = 12;
	else if (point == '1') value = 8;
	else 
	{
		value = point - '2';
	}
	
	tmp = tmp | primes[value];
	tmp = tmp | (value << 8);
	tmp = tmp | (1 << (16 + value));
	return tmp;
}
