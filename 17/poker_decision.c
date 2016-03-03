#include "poker_decision.h"
#include "tarrays.h"
#include <time.h>
#include <math.h>

extern int noFoldNum;
extern Pokers pokers[52];

int bit_to_num(unsigned int x)
{
	x = x >> 15;
	x &= 0xfffffffe;
	int ret = 0;
	do{
		ret++;
		x /= 2;
	} while (x > 1);
	return ret;
}


void init_pokers() //程序启动时调用一次，收到两张牌时更新其bool_known值 

{					//收到翻牌信息时更新其bool_known值 

	int i,j=0;

	char char_tmp[13] = {'2','3','4','5','6','7','8','9','1','J','Q','K','A'};

	for (i = 0; i < 13; i++)

	{
		pokers[j].bool_known = 0;
		pokers[j++].bit_poker = msg_to_bit("HEARTS", char_tmp[i]);
		pokers[j].bool_known = 0;
		pokers[j++].bit_poker = msg_to_bit("SPADES", char_tmp[i]);
		pokers[j].bool_known = 0;
		pokers[j++].bit_poker = msg_to_bit("CLUBS", char_tmp[i]);
		pokers[j].bool_known = 0;
		pokers[j++].bit_poker = msg_to_bit("DIAMONDS", char_tmp[i]);

	}

}
//int pre_flop_decision(unsigned int poker[])只是自己的两张牌
unsigned int pre_flop_decision(unsigned int poker[])
{
	unsigned int a = poker[0];
	unsigned int b = poker[1];
	unsigned int combine = 0;
	int flag = 0;//0表示同花
	int index = 0;
	int ret = 0;
	flag = a & b & 0x0000f000;
	
	a = a >> 16;
	b = b >> 16;
	if (a > b)
	{
		a ^= b;
		b ^= a;
		a ^= b;
	}
	a = a << 16;
	combine = 0;
	combine = a | b;

	if(flag == 0)//feitonghua 91
	{
		for(index = 0; index < 91; index++)
		{
			 if(feitonghua[index][0] == combine)
				ret = feitonghua[index][1];
		}
	}
	else//tonghua 78
	{
		for(index = 0; index < 78; index++)
		{
			 if(tonghua[index][0] == combine)
				ret = tonghua[index][1];
		}
		
	}
	return ret;
}



//翻牌圈决策，参数前两项为手牌，后三项为公牌 

int flop_decision(unsigned int poker[])

{

	unsigned int i,tmp1, tmp2, tmp3;

	unsigned int max_rank = 9999999;

	unsigned int for_ave = 0;

	unsigned int max_other = 9999999;

	unsigned int min_other = 1;

	unsigned int tmp_pokers[6];

	int counter = 0;

	for (i = 0; i < 5; i++)

	{

		tmp_pokers[i] = poker[i];

	}

	for (i = 0; i < 52; i++)

	{

		if (pokers[i].bool_known)

		{

			continue;

		}

		tmp_pokers[5] = pokers[i].bit_poker;

		int rank = max_rank_of_poker(tmp_pokers, 6);

		for_ave += rank;

		counter++;

	}

	for_ave /= counter;

	max_rank = for_ave;

	//printf("%d\n", max_rank);

	//判断锅里三张牌可能形成的大牌，放在max_other

	srand((int)time(NULL));
	int bigger = 0;
	counter = 0;
	for (i = 0; i < 1000; i++)

	{		

		tmp1 = rand() % 52;

		tmp2 = rand() % 52;

		tmp3 = rand() % 52;

		if (tmp1 == tmp2|| tmp2 == tmp3 || tmp1 == tmp3 || pokers[tmp1].bool_known == 1 || pokers[tmp2].bool_known ==1 || pokers[tmp3].bool_known == 1)

		{

//			printf("tmp1%d tmp2%d\n", tmp1, tmp2);
			i--;
			continue;

		}
		counter++;
		tmp_pokers[0] = pokers[tmp1].bit_poker;

		tmp_pokers[1] = pokers[tmp2].bit_poker;

		tmp_pokers[5] = pokers[tmp3].bit_poker;

		tmp1 = max_rank_of_poker(tmp_pokers, 6);

		if (tmp1 < max_rank)
			bigger++;

	}
	printf("%d\n", bigger);

	return (1 - (1 - pow((1 - (bigger / 1000.)), noFoldNum))) * 100;

}



//转牌圈决策，接受6张牌，前两张为手牌，后4张为公牌

int turn_decision(unsigned int poker[6])

{

	unsigned int i,tmp1, tmp2, tmp3;



	unsigned int max_other = 9999999;

	unsigned int min_other = 1;

	unsigned int tmp_pokers[7];

	for (i = 0; i < 6; i++)

	{

		tmp_pokers[i] = poker[i];

	}

	int rank = max_rank_of_poker(tmp_pokers, 6);



	//printf("%d\n", max_rank);

	//判断锅里四张牌可能形成的大牌，放在max_other

	srand((int)time(NULL));
	int bigger = 0;
	for (i = 0; i < 1000; i++)

	{		

		tmp1 = rand() % 52;

		tmp2 = rand() % 52;

		tmp3 = rand() % 52;

		if (tmp1 == tmp2 || tmp2 == tmp3 || tmp1 == tmp3 || pokers[tmp1].bool_known == 1 || pokers[tmp2].bool_known ==1 || pokers[tmp3].bool_known == 1)

		{

	//		printf("tmp1%d tmp2%d\n", tmp1, tmp2);
			i--;
			continue;

		}

		tmp_pokers[0] = pokers[tmp1].bit_poker;

		tmp_pokers[1] = pokers[tmp2].bit_poker;
		tmp_pokers[6] = pokers[tmp3].bit_poker;
		tmp1 = max_rank_of_poker(tmp_pokers, 7);

		if (tmp1 < rank)
			bigger++;

	}
	printf("%d\n", bigger);

	return (1 - (1 - pow((1 - (bigger / 1000.)), noFoldNum))) * 100;
}



//河牌圈决策，接受7张牌，前两张为手牌，后5张为公牌

int river_decision(unsigned int poker[7])

{

	unsigned int i,tmp1, tmp2;



	unsigned int max_other = 9999999;

	unsigned int min_other = 1;

	unsigned int tmp_pokers[7];

	for (i = 0; i < 7; i++)

	{

		tmp_pokers[i] = poker[i];

	}

	int rank = max_rank_of_poker(tmp_pokers, 7);



	//printf("%d\n", max_rank);

	//判断锅里五张牌可能形成的大牌，放在max_other

	srand((int)time(NULL));
	int bigger = 0;
	for (i = 0; i < 1000; i++)

	{		

		tmp1 = rand() % 52;

		tmp2 = rand() % 52;

		if (tmp1 == tmp2 || pokers[tmp1].bool_known == 1 || pokers[tmp2].bool_known ==1)

		{

	//		printf("tmp1%d tmp2%d\n", tmp1, tmp2);
			i--;
			continue;

		}

		tmp_pokers[0] = pokers[tmp1].bit_poker;

		tmp_pokers[1] = pokers[tmp2].bit_poker;

		tmp1 = max_rank_of_poker(tmp_pokers, 7);

	
		if (tmp1 < rank)
			bigger++;

	}
	return (1 - (1 - pow((1 - (bigger / 1000.)), noFoldNum))) * 100;

}


//更新全局牌是否已知，传进去的参数是已知的牌的数组
void update_card(unsigned int x[]) 
{
	int i;
	for (i = 0; i < 52; i++)
	{
		if (pokers[i].bit_poker == x[0] || pokers[i].bit_poker == x[1] ||
				pokers[i].bit_poker == x[2] || pokers[i].bit_poker == x[3] ||
				pokers[i].bit_poker == x[4] || pokers[i].bit_poker == x[5] || pokers[i].bit_poker == x[6])
		{
			//printf("%u\n", pokers[i].bit_poker);
			pokers[i].bool_known = 1;
		}
	}
}


/*
int main(int argc, char *argv[]) {

	unsigned int x[7] = {0,0,0,0,0,0,0};

	int i;

	init_pokers();

	x[0] = msg_to_bit("HEARTS", 'k');

	

	x[1] = msg_to_bit("SPADES", 'k');

	x[2] = msg_to_bit("CLUBS", '7');

//	x[6] = msg_to_bit("HEARTS", 'k');

	x[3] = msg_to_bit("DIAMONDS", '3');

	x[4] = msg_to_bit("HEARTS", '3');

//	x[5] = msg_to_bit("CLUBS", 'K');

	for (i = 0; i < 52; i++)

	{

		if (pokers[i].bit_poker == x[0] || pokers[i].bit_poker == x[1] ||

				pokers[i].bit_poker == x[2] || pokers[i].bit_poker == x[3] ||

				pokers[i].bit_poker == x[4] || pokers[i].bit_poker == x[5] || pokers[i].bit_poker == x[6])

		{

			printf("%u\n", pokers[i].bit_poker);

			pokers[i].bool_known = 1;

		}

	}

//	x[5] = msg_to_bit("DIAMONDS", '3');

	printf("%d\n",flop_decision(x));

//	int tmp = max_rank_of_poker(x,6);

//	printf("%d\n",tmp);

	return 0;

}
*/
