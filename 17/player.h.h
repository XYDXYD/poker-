
/*state
*默认状态
*/

#define BLINDSTATE 0
#define CHECK      1
#define CALL       2
#define RAISE      3
#define ALL_IN     4
#define FOLD       5

/*ribculous*/
#define BLIND     1
#define FLOP      2
#define TURN      3
#define RIVER     4

#define true      1
#define false     0

#define LIVE 	  0
#define DEAD 	  1


#define IN  	  0
#define OUT 	  1

typedef struct {
	int pid;
	int jetton;		/*本局赌注*/
	int money;		/*总金币*/
	int role;		/*3:button(庄家);2:small blind(小盲注);1:big blind(大盲注);0:普通玩家*/
	int bet;		
	int state;		/*0:无;1:blind 2:check 3:call 4:raise 5:跟all_in 6:加all_in 7:fold*/
	int liveordead; /* LIVE DEAD */
	int raisemoney; /* 这一轮放到锅里的前*/
	int in;
}player;

typedef struct{
	double rate_bet;
	int    need_notice;
} rate_of_bet;
typedef struct {
	int    check ;            
	int    call  ;
	int    raise ;              
	int    all_in;            
	int    fold  ;
	double variance       ;
	double relat_variance ;	
} history_info;

void         stat_info(unsigned int *);
double analyse_superplayer(unsigned int *,double *);
void         init_other_player();
unsigned int analyse_history_info();
unsigned int other_Players();
void         cal_variance(history_info *);
//unsigned int analyse_Bet();
double cal_judge_rule(unsigned int div_num);
double optimal_Blind();
double punish_rate(player *pplayer);

/*ananlyse_Honesty()*/
/*analyse_Progressive()*/
