#include "player.h"
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<stdio.h>
#include<string.h>
extern player playerList[8];
extern int    curr_orbiculus;           /*current orbiculus*/
extern int    totalPlayer;             /*the total number of playerList*/
extern int    my_previous;              /*my previous  player index in array palyers[7]*/
extern int    meIndex;
extern int blindMoney;
/*
 struct player 
 {
 	int pid;
	int money;
	int jetton; // the salary we have
	int bet;    //the salary we bet this time
	/
	*1:check
	*2:call
	*3:raise
	*4:all_in
	*5:fold
	
	/
	int state;
	/
	*1:blind
	*2:flop
	*3:turn
	*4:river
	/
	int orbiculus;

 	int raisemoney;  这一轮放到锅里的前
 }
*/

/*
 *return 1~10
 *1 :we are in the most dangerous situation
 *10:we are in safest situation
 */

//static rate_of_bet rob[8];
static history_info his_in[8];
unsigned int  judge_rule;

void init_other_player()
{
	int i;
	judge_rule = 0;
	for(i = 0; i < totalPlayer ; i++){
		//rob[i].rate_bet    =  0.0;
	//	rob[i].need_notice =  0;
	        his_in[i].check = 0;
	        his_in[i].call  = 0;
	        his_in[i].raise = 0;
	        his_in[i].all_in= 0;
	        his_in[i].fold  = 0;
	        his_in[i].variance       = 0.0;
	        his_in[i].relat_variance = 0.0;
	}

}
unsigned int other_Players()
{
	int i;
	double ret1 = 10.0;
	double ret2 = 10.0;

	double ret = 0;
	double score_player[8];
	unsigned int cu_in[8] = {0,0,0,0,0,0,0,0};
	//srand(time(NULL));
	printf("totalplayer = %d\n",totalPlayer);
	stat_info(cu_in);
	analyse_history_info();
	ret1 = analyse_superplayer(cu_in,score_player);
	ret2 = optimal_Blind();
	if(curr_orbiculus == BLIND){
		ret = ret2;
		printf("^^^^^^^^^^^^^^ret2 =%lf\n",ret2);
	}else{
		ret =ret1;
		printf("^^^^^^^^^^^^^^ret1 =%lf\n",ret1);
	}
//	printf("------------------------------&lf\n",ret);
//	printf("-----------######-------------&lf\n",ret);
//	printf("------------------------------&lf\n",ret);
	
	printf("^^^^^^^^^^^^^^ret = %lf\n",ret);
	ret = ret *10.0 ;//+ rand()%11;
	
	if (ret > 100.0) ret = 100.0;

	return (unsigned int)ret;
}

double punish_rate(player *pplayer)
{
	int rank = pplayer->raisemoney;
	int base = playerList[meIndex].jetton + playerList[meIndex].money;
	if (rank == 0){
		printf("------------------------------------------------\n");
		printf("player state is %d\n",pplayer->state);
		printf("player bet is %d\n",pplayer->bet);
		printf("player raisemoney is %d\n",pplayer->raisemoney);
		printf("player jetton is %d\n",pplayer->jetton);
		printf("player jetton is %d\n",pplayer->pid);
	}
	int rate = base / rank;
	if (rate >= 50) return 1.5;
	if(rate <= 6 ) return 1.0;
	return 1.0 +  (((double)rate - 6.0)/100.0);
}


double optimal_Blind(){
	int i = 0;
	//int blind_statis[6];
	double ret = 10.0;
	double tmp ;
	//memset(blind_statis,0,sizeof(blind_statis));
	for(i = 0; i < 8; i++){
		if( i == meIndex)
			continue;
		if(playerList[i].liveordead == LIVE ){
			switch(playerList[i].state){
					case CHECK : 
							tmp = 9.0;
							ret = (ret > tmp) ? tmp : ret; 
							 break;
					case CALL  :
							tmp =7.0 *punish_rate(&playerList[i]);
							ret = (ret > tmp) ? tmp : ret; 
							 break;
					case RAISE :
							tmp =6.0 *punish_rate(&playerList[i]);
							ret = (ret > tmp) ? tmp : ret; 
							 break;
					case ALL_IN:
							tmp = 4.0;
							ret = (ret > tmp) ? tmp:ret; 
							break;
					case FOLD  ://p[i] = FOLD;
							tmp = 10.0;
							ret = (ret > tmp) ? tmp : ret; 
							break;
				}
		}
			//blind_statis[playerList[i].state] ++;
	}
	/*if(blind_statis[ALL_IN] > 0){
		ret = 4.0;
	}else if(blind_statis[RAISE] > 0){
		ret = 6.0;
	}else if(blind_statis[CALL] > 0){
		ret = 7.0;
	}else{
		ret = 10.0;
	}*/
	
	return ret;
	
}


void stat_info(unsigned int *p)
{
	int i;
	for(i = 0; i < 8; i++){
			if(playerList[i].liveordead == DEAD)
				continue;
		switch(playerList[i].state){
			case CHECK : p[i] = CHECK;
				     his_in[i].check ++;
				     break;
			case CALL  :p[i] = CALL;
				     his_in[i].call ++;
			         break;
			case RAISE :p[i] = RAISE;
				    his_in[i].raise ++;
				    break;
			case ALL_IN:p[i] = ALL_IN;
				    his_in[i].all_in ++;
				    break;
			case FOLD  :p[i] = FOLD;
				    his_in[i].fold ++;
				    break;
		}
	}
	judge_rule ++;
}
double cal_judge_rule(unsigned int div_num)
{
	int  i = 0;
	double variance = 0.0;
	double pp_num = (double)judge_rule/(double)div_num;
	double average= (double)judge_rule/5.0;
	for(i = 0; i < div_num; i++){
		variance += (pp_num - average) * (pp_num - average);
	}
	for(i = div_num; i < 5; i++){
		variance += average * average;
	}
	variance = sqrt(variance)/5.0;
	return variance;
}





double analyse_superplayer(unsigned int *cu_in,double *score)
{
	int i;
	double min = 10.0;
	double one   = cal_judge_rule(1);
	double two   = cal_judge_rule(2);
	double three = cal_judge_rule(3);
	double four  = cal_judge_rule(4);
	double five  = cal_judge_rule(5);
	//srand(time(NULL));
	for(i = 0; i < 8;i++){
		if(playerList[i].liveordead == DEAD)
			continue;
		switch(cu_in[i]){
			case CHECK : if(his_in[i].relat_variance <= two){
						score[i] = 9.0 + (double)(rand() % 3);
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 6.0 + (double)(rand() % 3);
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 7.0 + (double)(rand() % 4);
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 8.0 + (double)(rand() % 3);
						goto go_out;
				    }
				     break;
			case CALL  :if(his_in[i].relat_variance <= two){
						score[i] = 7.0 + (double)(rand() % 4);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 4.0 + (double)(rand() % 3);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 5.0 + (double)(rand() % 4);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 6.0 + (double)(rand() % 5);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
				    }
			             break;
			case RAISE :if(his_in[i].relat_variance <= two){
						score[i] = 5.0 + (double)(rand() % 6);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 2.0 + (double)(rand() % 3);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 3.0 + (double)(rand() % 4);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 4.0 + (double)(rand() % 5);
						score[i] *= punish_rate(&playerList[i]);
						goto go_out;
				    }
				    break;
			case ALL_IN:if(his_in[i].relat_variance <= two){
						score[i] = 3.0 + (double)(rand() % 6);
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 0.0 + (double)(rand() % 3);
						printf("in here=======================\n");
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 1.0 + (double)(rand() % 4);
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 2.0 + (double)(rand() % 5);
						goto go_out;
				    }
				    break;
			case FOLD  :if(his_in[i].relat_variance <= two){
						score[i] = 10.0;// + rand() % 5;
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 8.0 + (double)(rand() % 3);
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 9.0 + (double)(rand() % 2);
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 10.0;// + rand() % 5;
						goto go_out;
				    }
    go_out:
				    break;
		}
	    if(i == meIndex)
			continue;
		if(score[i] !=0 )
		min = (min > score[i]) ? score[i]:min;
	}
	printf("^^^^^^^^^^^^^^^^min = %lf \n",min);
	return min;
}




unsigned int  analyse_history_info()
 {
 
	 int i;
	 double max = cal_judge_rule(1);
	// for(i = 0; i < totalPlayer; i++){
	 //	cal_variance(&his_in[i]);
	 //	max = (max < his_in[i].variance) ? his_in[i].variance : max;
	 //}

	 for(i = 0; i < 8; i++){
		 if(playerList[i].liveordead == DEAD)
			 continue;
		cal_variance(&his_in[i]);
		if( max == 0)max += 1.0;
	 	his_in[i].relat_variance = his_in[i].variance / max;
	 }
 
 
 }

void cal_variance(history_info *p)
{
	int    sum      = 0;
	double average  = 0;
	double variance = 0.0;
	sum = p->check + p->call + p->raise + p->all_in + p->fold;
	average = (double)sum / 5.0;
	variance =  ((double)p->check  - average) * ((double)p->check  - average) + \
				((double)p->call   - average) * ((double)p->call   - average) + \
				((double)p->raise  - average) * ((double)p->raise  - average) + \
				((double)p->all_in - average) * ((double)p->all_in - average) + \
				((double)p->fold   - average) * ((double)p->fold   - average);
	p->variance = sqrt(variance)/5.0;

}

