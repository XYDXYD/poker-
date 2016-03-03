#ifndef ROP
#define ROP 
#include <stdio.h>
#include <stdlib.h>
#include "max_rank_of_poker.h"
#endif

typedef struct unknown_poker{
	int bool_known;
	unsigned int bit_poker;
} Pokers; 

//��������ʱ����һ�Σ��յ�������ʱ������bool_knownֵ
void init_pokers();  

//����Ȧ���ߣ�����ǰ����Ϊ���ƣ�������Ϊ���� 
int flop_decision(unsigned int poker[5]);

//ת��Ȧ���ߣ�����6���ƣ�ǰ����Ϊ���ƣ���4��Ϊ����
int turn_decision(unsigned int poker[6]);

//����Ȧ���ߣ�����7���ƣ�ǰ����Ϊ���ƣ���5��Ϊ����
int river_decision(unsigned int poker[7]);

//���ƺ�������
//����ȫ�����Ƿ���֪������ȥ�Ĳ�������֪���Ƶ�����

void update_card(unsigned int x[]);

//
int bit_to_num(unsigned int x);
//
unsigned int pre_flop_decision(unsigned int poker[]);
