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

	unsigned int i,tmp1, tmp2;

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



	//printf("%d\n", max_rank);

	//判断锅里三张牌可能形成的大牌，放在max_other

	srand((int)time(NULL));

	for (i = 0; i < 1000; i++)

	{		

		tmp1 = rand() % 52;

		tmp2 = rand() % 52;

		if (tmp1 == tmp2 || pokers[tmp1].bool_known == 1 || pokers[tmp2].bool_known ==1)

		{

//			printf("tmp1%d tmp2%d\n", tmp1, tmp2);

			continue;

		}

		tmp_pokers[0] = pokers[tmp1].bit_poker;

		tmp_pokers[1] = pokers[tmp2].bit_poker;

		tmp1 = max_rank_of_poker(tmp_pokers, 5);

		if (tmp1 < max_other)

			max_other = tmp1;

		if (tmp1 > min_other)

			min_other = tmp1;

	}

	max_rank = for_ave;

	//printf("max_other%d min_other%d max_rank%d\n", max_other, min_other, max_rank);	

	

	int ret;

	if (max_rank > min_other)

		ret = 0;

	else if (max_rank <= max_other)

		ret = 100;

	else

	{
		int chazhi = min_other - max_other;
		double bigger_than_me = 1 - pow(1 - (max_rank - max_other) / (float)(chazhi), noFoldNum);
		double less_than_me = 1- pow(1 - (min_other - max_rank) / (float)(chazhi), noFoldNum);
		ret = (less_than_me - bigger_than_me + 1) * 50;
		printf("@@@@@@@@noFoldNum:%d\n", noFoldNum);	
//		printf("@@@@@@@@bigger_than_me-less_than_me-ret：%lf---%lf---%d\n", bigger_than_me, less_than_me, ret);	
	}

	//	ret = (int)((min_other - max_rank) / (float)(min_other - max_other) * 100);
    
	return ret;

}



//转牌圈决策，接受6张牌，前两张为手牌，后4张为公牌

int turn_decision(unsigned int poker[6])

{

	unsigned int i,tmp1, tmp2;



	unsigned int max_other = 9999999;

	unsigned int min_other = 1;

	unsigned int tmp_pokers[6];

	for (i = 0; i < 6; i++)

	{

		tmp_pokers[i] = poker[i];

	}

	int rank = max_rank_of_poker(tmp_pokers, 6);



	//printf("%d\n", max_rank);

	//判断锅里四张牌可能形成的大牌，放在max_other

	srand((int)time(NULL));

	for (i = 0; i < 1000; i++)

	{		

		tmp1 = rand() % 52;

		tmp2 = rand() % 52;

		if (tmp1 == tmp2 || pokers[tmp1].bool_known == 1 || pokers[tmp2].bool_known ==1)

		{

	//		printf("tmp1%d tmp2%d\n", tmp1, tmp2);

			continue;

		}

		tmp_pokers[0] = pokers[tmp1].bit_poker;

		tmp_pokers[1] = pokers[tmp2].bit_poker;

		tmp1 = max_rank_of_poker(tmp_pokers, 6);

		if (tmp1 < max_other)

			max_other = tmp1;

		if (tmp1 > min_other)

			min_other = tmp1;

	}

	//printf("max_other%d min_other%d max_rank%d\n", max_other, min_other, rank);	

	

	int ret;

	if (rank > min_other)

		ret = 0;

	else if (rank <= max_other)

		ret = 100;

	else

	{
	
		int chazhi = min_other - max_other;
		double bigger_than_me = 1 - pow(1 - (rank - max_other) / (float)(chazhi), noFoldNum);

		double less_than_me = 1 - pow(1 - (min_other - rank) / (float)(chazhi), noFoldNum);

		ret = (less_than_me - bigger_than_me + 1) * 50;
		printf("@@@@@@@@noFoldNum:%d\n", noFoldNum);	
//		printf("@@@@@@@@bigger_than_me-less_than_me-ret：%lf---%lf---%d\n", bigger_than_me, less_than_me, ret);	
	}

	return ret;

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

	for (i = 0; i < 1000; i++)

	{		

		tmp1 = rand() % 52;

		tmp2 = rand() % 52;

		if (tmp1 == tmp2 || pokers[tmp1].bool_known == 1 || pokers[tmp2].bool_known ==1)

		{

	//		printf("tmp1%d tmp2%d\n", tmp1, tmp2);

			continue;

		}

		tmp_pokers[0] = pokers[tmp1].bit_poker;

		tmp_pokers[1] = pokers[tmp2].bit_poker;

		tmp1 = max_rank_of_poker(tmp_pokers, 7);

	

		if (tmp1 < max_other)

			max_other = tmp1;

		if (tmp1 > min_other)

			min_other = tmp1;

	}

	//printf("max_other%d min_other%d max_rank%d\n", max_other, min_other, rank);	

	

	int ret;

	if (rank > min_other)

		ret = 0;

	else if (rank <= max_other)

		ret = 100;

	else

	{
		int chazhi = min_other - max_other;
		double bigger_than_me = 1 - pow(1 - (rank - max_other) / (float)(chazhi), noFoldNum);

		double less_than_me = 1- pow(1 - (min_other - rank) / (float)(chazhi), noFoldNum);

		ret = (less_than_me - bigger_than_me + 1) * 50;
		printf("@@@@@@@@noFoldNum:%d\n", noFoldNum);	
//		printf("@@@@@@@@bigger_than_me-less_than_me-ret：%lf---%lf---%d\n", bigger_than_me, less_than_me, ret);	
	}

	return ret;

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
