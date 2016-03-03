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
	int honesty;
 
 }
 
 
 */
/*
 *return 1~10
 *1 :we are in the most dangerous situation
 *10:we are in safest situation
 */

static rate_of_bet rob[8];
static history_info his_in[8];
unsigned int  judge_rule;

void init_other_player()
{
	int i;
	judge_rule = 0;
	for(i = 0; i < totalPlayer ; i++){
		rob[i].rate_bet    =  0.0;
		rob[i].need_notice =  0;
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
	unsigned int ret1 = 0;
	unsigned int ret2 = 0;
	unsigned int ret = 0;
	unsigned int score_player[8];
	unsigned int cu_in[8] = {0,0,0,0,0,0,0,0};
	//srand(time(NULL));
	printf("totalplayer = %d\n",totalPlayer);
	stat_info(cu_in);
	analyse_history_info();
	ret1 = analyse_superplayer(cu_in,score_player);
	if(curr_orbiculus == BLIND){
		ret2 = optimal_Blind();
	}
//	printf("------------------------------&lf\n",ret);
//	printf("-----------######-------------&lf\n",ret);
//	printf("------------------------------&lf\n",ret);
	ret = (ret1 < ret2) ? ret2 : ret1;
	ret = ret *10 + rand()%11;
	if (ret > 100) ret = 100;
	return ret;
}

unsigned int optimal_Blind(){
	int i = 0;
	int blind_statis[6];
	int ret;
	memset(blind_statis,0,sizeof(blind_statis));
	for(i = 0; i < 8; i++){
		if(playerList[i].liveordead == LIVE){
			blind_statis[playerList[i].state] ++;
		}
	}
	if(blind_statis[ALL_IN] > 0){
		ret = 4;
	}else if(blind_statis[RAISE] > 0){
		ret = 6;
	}else if(blind_statis[CALL] > 0){
		ret = 7;
	}else{
		
		ret = 10;
	}
	
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
				     rob[i].need_notice = 1;
				     break;
			case CALL  :p[i] = CALL;
				     his_in[i].call ++;
				     rob[i].need_notice = 1;
			         break;
			case RAISE :p[i] = RAISE;
				    his_in[i].raise ++;
				    rob[i].need_notice = 1;
				    break;
			case ALL_IN:p[i] = ALL_IN;
				    his_in[i].all_in ++;
				    rob[i].need_notice = 0;
				    break;
			case FOLD  :p[i] = FOLD;
				    his_in[i].fold ++;
				    rob[i].need_notice = 0;
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



unsigned int analyse_superplayer(unsigned int *cu_in,unsigned int *score)
{
	int i;
	unsigned int min = 10;
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
						score[i] = 9 + rand() % 3;
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 6 + rand() % 3;
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 7 + rand() % 4;
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 8 + rand() % 3;
						goto go_out;
				    }
				     break;
			case CALL  :if(his_in[i].relat_variance <= two){
						score[i] = 7 + rand() % 4;
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 4 + rand() % 3;
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 5 + rand() % 4;
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 6 + rand() % 5;
						goto go_out;
				    }
			             break;
			case RAISE :if(his_in[i].relat_variance <= two){
						score[i] = 5 + rand() % 6;
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 2 + rand() % 3;
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 3 + rand() % 4;
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 4 + rand() % 5;
						goto go_out;
				    }
				    break;
			case ALL_IN:if(his_in[i].relat_variance <= two){
						score[i] = 3 + rand() % 6;
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 0 + rand() % 3;
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 1 + rand() % 4;
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 2 + rand() % 5;
						goto go_out;
				    }
					/*else{
				    	if( his_in[i].relat_variance < 0.7)
				    		score[i] = 1;
					else score[i] = 2;
				    };*/
				    break;
			case FOLD  :if(his_in[i].relat_variance <= two){
						score[i] = 10;// + rand() % 5;
						goto go_out;
					}
					if(two < his_in[i].relat_variance && his_in[i].relat_variance <= three){
						score[i] = 8 + rand() % 3;
						goto go_out;
					}
					if(three < his_in[i].relat_variance && his_in[i].relat_variance <= four){
						score[i] = 9 + rand() % 2;
						goto go_out;
					}
					if(four < his_in[i].relat_variance && his_in[i].relat_variance <= five ){
					    score[i] = 10;// + rand() % 5;
						goto go_out;
				    }
    go_out:
				    break;
		}
	    if(i == meIndex)continue;
	     min = (min > score[i]) ? score[i]:min;
	}
	return min;
}

unsigned int analyse_Bet()
{
	int i;
	int ret = 0;
	double rate_of_bet[8];
	double max = 0.0;
	for(i = 0; i < totalPlayer; i++){
	 	if(rob[i].need_notice){
			rob[i].rate_bet = playerList[i].bet/(playerList[i].bet + playerList[i].jetton);
			max =( max < rob[i].rate_bet) ? rob[i].rate_bet : max;
		}
	}
	if(max > 0.75){
		ret = 4;
	}else{
		if(max > 0.5){
			ret = 5;
		}else{
			ret = 6;
		}	
	}
	return ret;
}

/*next will do*/
/*unsigned int analyse_Honesty()
{


 return 

}*/


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
/*
analyse_Progressive()
{
return 


}
   */
