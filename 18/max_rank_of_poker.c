#include "max_rank_of_poker.h"


//����1Ϊpoker���飬����5��6��7���ƣ�nע������ 
int max_rank_of_poker(int poker[], int n)
{
	if (n == 5)
		return rank_of_poker(poker);
	int i,j,k;
	int g;
	int max = 9000999; //ѡ����5�ŵ���Сrank 
	int rank_tmp;//ѡ����5�ŵ�rank 
	int tmp[5] = {0,0,0,0,0};
	if (n == 6)
		for (i = 0; i < 6; i++)
		{
			k = 0;
			for (j = 0; j < 6; j++)
			{
				if (j == i)
				{
					 continue;
				}
				tmp[k++] = poker[j];				
			}
			rank_tmp = rank_of_poker(tmp);
			if (rank_tmp < max) 
				max = rank_tmp;
		}
	else
		for(i = 0; i < 6; i++)
		{
			for (j = i + 1; j < 7; j++)
			{
				k = 0;
				for (g = 0; g < 7; g++)
				{
					if (g == i || g == j)
						continue;
					tmp[k++] = poker[g];
				}
				rank_tmp = rank_of_poker(tmp);
				if (rank_tmp < max) 
				max = rank_tmp;
			}
		}
	return max;
}
