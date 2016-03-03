#include "arrays.h"
#include "rank_of_poker.h"






int no_unique(int poker[5])
{//判断对子三条四条葫芦 
	int q = (poker[0] & 0xff) * (poker[1] & 0xff) * (poker[2] & 0xff) * (poker[3] & 0xff) * (poker[4] & 0xff);
	int index = 4888;
	int low = 0, high = 4887;
//	printf("%d\n",q); 
	while (low <= high)
	{
		if (q < products[(low + high)/2])
		{
			high = (low + high) / 2 - 1;
		}
		else if (q > products[(low + high)/2])
		{
			low = (low + high) / 2 + 1;
		}
		else
			return values[(low + high) / 2];
	}
//	printf("don't find prime sum\n"); 
	return 0;
}

int rank_of_poker(int poker[])
{
	int ret = 0; //返回值 
	int q;	//索引 
	//判断扑克是不是同花
	int judge =  poker[0] & poker[1] & poker[2] & poker[3] & poker[4] & 0xF000;
	q = (poker[0] | poker[1] | poker[2] | poker[3] | poker[4]) >> 16;
//	printf("%d\n",q); 
	if (judge)
	{
		ret = flushes[q];
	}
	else
	{
		//判断高牌与顺子
		ret = unique5[q];
		if (ret == 0)
		{
			ret = no_unique(poker);
		} 
	}
	return accumulative_rank[ret];
}
