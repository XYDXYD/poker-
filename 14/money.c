#include "money.h"
#include "player.h"
#include <stdio.h>

extern player playerList[8];
extern int    meIndex;
extern int 	  leastBetMoney;	

int money_decision()
{
	int index;
	int leastallmoney = 0;
	int maxraisemoney = 0;
	int leastcallmoney  = leastBetMoney - playerList[meIndex].bet;
	double raise_rate;
	double my_rate;
	double sum_rate;
	//find max 
	leastcallmoney = (leastcallmoney > playerList[meIndex].jetton ? playerList[meIndex].jetton : leastcallmoney);
	for(index = 0; index < 8; index++)
	{
		if(index == meIndex || playerList[index].liveordead == DEAD || playerList[index].in == OUT ) continue;
		if(maxraisemoney < playerList[index].raisemoney)
		{
			maxraisemoney = playerList[index].raisemoney;
			leastallmoney = playerList[index].money + playerList[index].jetton + playerList[index].raisemoney;
		}
		else if(maxraisemoney == playerList[index].raisemoney && leastallmoney > playerList[index].money + playerList[index].jetton)
		{
			leastallmoney = playerList[index].money + playerList[index].jetton + playerList[index].raisemoney;
		}

	}
	//max_rate
	//if(leastallmoney  0)
	printf("--------------leastallmoney %d\n",leastallmoney);
	if(maxraisemoney == 0)
		raise_rate = 0;
	else
		raise_rate = (double)maxraisemoney /  leastallmoney;
	//my_rate
	//if(!((playerList[meIndex].money + playerList[meIndex].jetton)))
	printf("--------------myleastallmoney %d\n",playerList[meIndex].money + playerList[meIndex].jetton);
	my_rate = (double)leastcallmoney / (playerList[meIndex].money + playerList[meIndex].jetton);
	
	//结合
	sum_rate = raise_rate + my_rate;
	printf("<<<<<<<<<<<<<<<<<<<raise_rate%lf\n",raise_rate);
	printf("<<<<<<<<<<<<<<<<<<<my_rate%lf\n",my_rate);
	printf("<<<<<<<<<<<<<<<<<<<sum_rate%lf\n",sum_rate);
	//判断
	if( sum_rate > 2)	return (int)0;
	if( sum_rate > 1)	return (int)(50 * ( 2 - sum_rate ));
    return (int)(50 + 50 * (1 - sum_rate));
	
}

