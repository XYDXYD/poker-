#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "poker_decision.h"
#include "player.h"
#include "money.h"

int m_socket_id = -1;

Pokers pokers[52];

int curr_orbiculus = BLIND;		//当前一局阶段 
int mycardDecision = 0;			//0-100 最差到最好 
int playerDecision = 0;			//1-10 最差到最好
int totalPlayer;				//玩家数量
player playerList[8];			//所有玩家
int meIndex;					//我的下标
int my_previous;				//我前面一个人的下标 ： 这个值应该是随着场上人数变化而变化的  ERROR
int totalPot;					//锅里的钱
int blindMoney = 0;				//大盲注钱 同时是最小加注额
int leastBetMoney = 0;			//我最少应该投到锅里的钱
int noFoldNum = 0;				//未fold的人数

unsigned int myCard[2];			//我的牌
unsigned int publicCard[5]; 	//公牌

/*只在盲注阶段用 判断盲注阶段是否有RAISE*/
int haveRaiseInBind()
{
	int i;
	for( i = 0; i < 8; i++)
	{
		if(playerList[i].state == RAISE)
			return 1;
	}
	return 0;
	
}
/*
   根据player的seat来找到pid对应的下标
*/
int findTrueIndex(int pid)
{
	int i;
	for( i = 0; i < 8; i++)
	{
		if(playerList[i].pid == pid)
			return i;
	}
}

/*
   根据player的seat来找到我的当前的前面一个玩家的index
*/
int findTruePrevious( )
{
	int i = 1;
	int preIndex = (8 + meIndex - i) % 8;
	while(1)
	{
		if(playerList[preIndex].in == IN)
			if(preIndex != meIndex)
				return preIndex;
			else	
				return -1;					//场上只有我
		else
		{
			i++;
			preIndex = (8 + meIndex - i) % 8;
		}	
	}
}

/*
   找到其余玩家中剩余钱数最多人的钱数 money + jetton
*/
int findMaxOtherMoney( )
{
	int preindex;
	int otherMaxmoney = 0;
	for(preindex = 0; preindex < 8; preindex++)
	{
		if(preindex != meIndex)
		{
			if( otherMaxmoney < playerList[preindex].jetton + playerList[preindex].money )
				otherMaxmoney = playerList[preindex].jetton + playerList[preindex].money;
		}
	}
	return otherMaxmoney;
				
}

/* 
   处理server的消息 
*/
int server_msg_kind(const char* buffer)
{
	int  msgkind = 0;
	if(NULL != strstr(buffer, "game-over"))
		return -1;
	if(NULL != strstr(buffer,"seat/ ")) 		msgkind = 1;
	if(NULL != strstr(buffer,"blind/ ")) 		msgkind = 2;
	if(NULL != strstr(buffer,"hold/ ")) 		msgkind = 3;
	if(NULL != strstr(buffer,"inquire/ ")) 		msgkind = 4;
	if(NULL != strstr(buffer,"flop/ ")) 		msgkind = 5;
	if(NULL != strstr(buffer,"turn/ ")) 		msgkind = 6;
	if(NULL != strstr(buffer,"river/ ")) 		msgkind = 7;
	if(NULL != strstr(buffer,"showdown/ ")) 	msgkind = 8;
	if(NULL != strstr(buffer,"port-win/ ")) 	msgkind = 9;
	return msgkind;
}

int main(int argc, char *argv[])
{
     
	if (argc != 6)
	{
		printf("Usage: ./%s server_ip server_port my_ip my_port my_id\n", argv[0]);
		return -1;
	}
	
	/* 初始化52张牌 */
	init_pokers();
    
	/* 
	   如果对方已经关闭socket 还是向其发送数据 会引起SIGPIPE 导致本地进程关闭
	   解决的方法是捕获SIGPIPE信号并延时重发
	*/
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;	
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, 0); 
	
	/* 获取输入参数 *//*server IP &port*//*my IP &port*//*my_id*/
	in_addr_t server_ip = inet_addr(argv[1]);
	in_port_t server_port = htons(atoi(argv[2])); 
	in_addr_t my_ip = inet_addr(argv[3]);
	in_port_t my_port = htons(atoi(argv[4])); 
	int my_id = atoi(argv[5]);
    srand((unsigned)(my_id * (unsigned)time(NULL)));
	 //int rand_seed = rand()%10;
	/* 创建socket */
	while((m_socket_id = socket(PF_INET, SOCK_STREAM, 0))<0);

	/* 设置socket选项，地址重复使用，防止程序非正常退出，下次启动时bind失败 */
	int is_reuse_addr = 1;
	setsockopt(m_socket_id, SOL_SOCKET, SO_REUSEADDR, \
			  (const char*)&is_reuse_addr, sizeof(is_reuse_addr));

	/* 绑定指定ip和port，不然会被server拒绝 */
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = my_ip;
	my_addr.sin_port = my_port;
	while((bind(m_socket_id, (struct sockaddr*)&my_addr, sizeof(my_addr)))<0);

	/* 连接server *//* sleep 100ms, 然后重试，保证无论server先起还是后起，都不会有问题 */
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = server_ip;
	server_addr.sin_port = server_port;
	while(connect(m_socket_id, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		usleep(100*1000); 
	}

	/* 向server注册 */
	char reg_msg[50] = {'\0'};
	snprintf(reg_msg, sizeof(reg_msg) - 1, "reg: %d %s \n", my_id, "E3421"); 
	
	/* SOCKER_ERROR = -1*/
	while(send(m_socket_id, reg_msg, strlen(reg_msg) + 1, 0) < 0)
	{
		usleep(100*1000); 
	}
	//fprintf(pFile,"%s",reg_msg);
	/* 进入游戏 */
	init_other_player();
	
	char msg_from_server[BUFSIZ] = {'\0'};			
	int len_from_server = 0;
	char msg_to_server[50] = {'\0'};
	int len_to_server = 0;
	int msg_kind = -1;
	
	int pidIndex;											//下标 用时初始
	//行"\n"分割
	char delims[] = "\n";
    char *result = NULL;
	char tmpline[50] = {'\0'};
	char *lineptr = NULL;
	//行中":"分割
	char delimsLine[] = ":";
	char *resultLine = NULL;
	char tmpmao[50] = {'\0'};
	char *maoptr = NULL;
	//行中" "分割
	char delimsEach[] = " ";
	char *resultEach = NULL;
	char tmpeach[50] = {'\0'};
	char *eachptr = NULL;
	//角色
	int role;
	//牌面信息
	char color[10];
	char point[3];	
	int state;
	
	unsigned int allCard[7] = {0};
	unsigned int tmp5card[5] = {0};
	unsigned int tmp6card[6] = {0};
	unsigned int tmp7card[7] = {0};
	int cardIndex = 0;
	while(1)
	{
		len_from_server = 0;
		len_to_server = 0;
		msg_kind = -1;
		result = NULL;
		lineptr = NULL;
		resultLine = NULL;
		maoptr = NULL;
		resultEach = NULL;
		eachptr = NULL;

		memset(msg_from_server,'\0',BUFSIZ);
		len_from_server = recv(m_socket_id, msg_from_server, sizeof(msg_from_server) - 1, 0);
		
		//printf("msg%s \n",msg_from_server);
		
		if(NULL != strstr(msg_from_server, "game-over"))
			break;
		if(0 == strcmp(msg_from_server, "game-over \n"))
			break;
		if(len_from_server <= 0)/*如果小于等于0表示出错或者0字节*/
		{
			continue;
		}
		/* 获取消息第一行 */
		result = strtok_r( msg_from_server, delims, &lineptr );
			
		/* 判断消息类型，类型值msg_kind 若game over 跳出循环 */
DealMsg:
		if (-1 == (msg_kind = server_msg_kind(result)))
		{
			break;
		}
			
		//指向消息下一行
		result = strtok_r( NULL, delims, &lineptr );
		
		switch(msg_kind)
		{
			case 1:
			//printf("seat\n");
			/*a)发布座次信息：seat-info-msg（轮流坐庄）
			----------------------------------------
			seat/ eol
			button: pid jetton money eol
			small blind: pid jetton money eol
			(big blind: pid jetton money eol)0-1
			(pid jetton money eol)0-5
			/seat eol
			---------------------------------------
			含义：各牌手的座次信息，即发牌和喊注次序
			方向：server -> player
			方式：广播
			说明：只剩下两个玩家时，将没有大盲  注及后续座位信息
			*/
			//根据行数判断人数,默认顺序是:庄家 0 小盲注 1  大盲注 2  其他人
			/* 初始化52张牌 */
			//srand(time(NULL));
			init_pokers();
			pidIndex = 0;
			role = 0;
			resultLine = NULL;
			resultEach = NULL;
			curr_orbiculus = BLIND;
			while(strcmp(result,"/seat "))
			{
			    /* 暂存下一行 */
				memset(tmpline,'\0',50);
				strcpy(tmpline,result);
				//每行一个人,包括玩家自己，最多前两行有符号":",
				//指向":"前
				resultLine = strtok_r( tmpline, delimsLine, &maoptr );
				memset(tmpeach,'\0',50);
				strcpy(tmpeach,result);
				//庄家
				if(0 == strcmp(resultLine,"button"))
				{
					role = 3;
					//指向":"后方
					resultLine = strtok_r( NULL, delimsLine, &maoptr );
					memset(tmpeach,'\0',50);
					strcpy(tmpeach,resultLine+1);
				}
					//小盲注
				if(0 == strcmp(resultLine,"small blind"))
				{	
					role = 2;
					//指向":"后方
					resultLine = strtok_r( NULL, delimsLine, &maoptr );
					memset(tmpeach,'\0',50);
					strcpy(tmpeach,resultLine+1);
				}
				//大盲注
				if(0 == strcmp(resultLine,"big blind"))
				{	
					role = 1;
					//指向":"后方
					resultLine = strtok_r( NULL, delimsLine, &maoptr );
					memset(tmpeach,'\0',50);
					strcpy(tmpeach,resultLine+1);
				}
					
				//解析每个属性
				resultEach = strtok_r( tmpeach, delimsEach,&eachptr );
				playerList[pidIndex].pid = atoi(resultEach);
				
				resultEach = strtok_r( NULL, delimsEach ,&eachptr);
				playerList[pidIndex].jetton = atoi(resultEach);
				
				resultEach = strtok_r( NULL, delimsEach ,&eachptr);
				playerList[pidIndex].money = atoi(resultEach);
				
				playerList[pidIndex].bet = 0;
				playerList[pidIndex].state = -1;
				playerList[pidIndex].role = role;
				playerList[pidIndex].liveordead = LIVE;
				playerList[pidIndex].raisemoney = 0;
				playerList[pidIndex].in = IN;
				
				if(playerList[pidIndex].pid == my_id)
				{
					meIndex = pidIndex;
					my_previous = (8 + meIndex - pidIndex) % 8;
				}
				//下一行
				//printf("eachline%s \n",result);
				result = strtok_r( NULL, delims, &lineptr );
				pidIndex++;
				role = 0;
			}
			//记录人数
			totalPlayer = pidIndex;
			noFoldNum = totalPlayer - 1;
			/*
			int iiii;
			for(iiii = 0; iiii < 8; iiii++)
			{
				printf("--------playerList[%d].money: %d\n",iiii,playerList[iiii].money);
				printf("--------playerList[%d].jetton: %d\n",iiii,playerList[iiii].jetton);
				printf("--------playerList[%d].bet: %d\n",iiii,playerList[iiii].bet);
			}
			*/
			//printf("eachline%s \n",result);
			//看一下后面还有没有消息
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
			{
				//printf("@@@@@@@@@@@@@@%s \n",result);
				goto DealMsg;
			}
			//printf("#################%s \n",result);
			break;
			
			case 2:
			/*b)强制押盲注：blind-msg
			-------------------------------------------
			blind/ eol
			(pid: bet eol)1-2
			/blind eol
			------------------------------------------
			含义：server发布盲注玩家自动扣除盲注金额
			方向：server –> player
			方式：广播
			说明：大盲注金额为小盲注的2倍，只有2个玩家时，没有大盲注信息
			*/
			//需要更新盲注玩家的信息 默认顺序  小盲注 大盲注
			pidIndex = 1;
			resultLine = NULL;
			resultEach = NULL;
			curr_orbiculus = BLIND;
			while(strcmp(result,"/blind "))
			{
				memset(tmpline,'\0',50);
				strcpy(tmpline,result);
				//每行一个人,有两行
				resultLine = strtok_r( tmpline, delimsLine, &maoptr );
				//指向":"后方
				resultLine = strtok_r( NULL, delimsLine, &maoptr );
				//过掉bet前面的空格
				memset(tmpeach,'\0',50);
				strcpy(tmpeach,resultLine+1);
				//过掉bet后面的空格
				resultEach = strtok_r( tmpeach, delimsEach,&eachptr );
				//更新相关数据
				playerList[pidIndex].bet = atoi(resultEach);
				playerList[pidIndex].jetton -= atoi(resultEach);
				//下一行
				result = strtok_r( NULL, delims, &lineptr );
				pidIndex++;
			}
			blindMoney = playerList[1].bet * 2;
			//看一下后面还有没有消息
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
			
			break;
				
			case 3:
			/*c)为每位牌手发两张底牌：hold-cards-msg
			----------------------------------------
			hold/ eol
			color point eol
			color point eol
			/hold eol	
			---------------------------------------
			含义：server发2张手牌
			方向：server –> player
			方式：单播
			int msg_to_bit(char color[], char point);
			*/	
			pidIndex = 0;
			resultEach = NULL;
			memset(color,0,10); 							//数据初始化--清零
			memset(point,0,3);
			//确定有两行
			while(strcmp(result,"/hold "))
			{
				memset(tmpeach,'\0',50);
				strcpy(tmpeach,result);
				//花色
				resultEach = strtok_r( tmpeach, delimsEach,&eachptr );
				memset(color,'\0',10);
				strcpy(color,resultEach);
				//点数
				resultEach = strtok_r( NULL, delimsEach,&eachptr );
				memset(point,'\0',3);
				strcpy(point,resultEach);
				if(strlen(point)>1)	strcpy(point,"1");
				//处理后保存
				myCard[pidIndex] = msg_to_bit(color,point[0]);
				//下一行
				result = strtok_r( NULL, delims, &lineptr );
				pidIndex++;
			}
			curr_orbiculus = BLIND;
			
			//自己两张
			mycardDecision = pre_flop_decision(myCard);
			//我的钱目前第一
			if((playerList[meIndex].jetton + playerList[meIndex].money) > findMaxOtherMoney())
			{
				mycardDecision = 201;
			}
			//盲注阶段有人加注
			else if( mycardDecision < 90 && haveRaiseInBind())
			{
				mycardDecision = 0;
			}
				
			//看一下后面还有没有消息	
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
			
			break;
			
			case 4:
			/*d)翻牌前喊注： inquire-msg/action-msg（多次）
			  f)翻牌圈喊注： inquire-msg/action-msg（多次）
			  h)转牌圈喊注： inquire-msg/action-msg（多次）
			  j)河牌圈喊注： inquire-msg/action-msg（多次）
			inquire-msg:
			-------------------------------------------------------------------------
			inquire/ eol
			(pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
			total pot: num eol
			/inquire eol
			--------------------------------------------------------------------------
			含义：server向player询问行动决策
			方向：server –> player
			方式：单播
			说明：
			player只有收到此询问消息之后, 才能发出action消息 
			询问消息中会包含如下内容： 
			a)	本手牌已行动过的所有玩家（包括被询问者和盲注）的手中筹码、剩余金币数、本手牌累计投注额、及最近的一次有效action，按逆时针座次由近及远排列，上家排在第一个
			b)	当前底池总金额（本轮投注已累计）
			*/	
			resultLine = NULL;
			resultEach = NULL;
			state = -1;
			int trueIndex;
			int i = 0;
			leastBetMoney = 0;
			//srand(time(NULL));
			for(i = 0; i < 8; i++ )
			{
				playerList[i].liveordead = DEAD;
			}
			playerList[meIndex].liveordead = LIVE;
			//确定有两行
			while(1)
			{
				// 暂存这一行
				memset(tmpline,'\0',50);
				strcpy(tmpline,result);
				// 判断 total pot: 15400 
				resultLine = strtok_r( tmpline, delimsLine, &maoptr );
				if(0 == strcmp(resultLine,"total pot"))
				{
					resultLine = strtok_r( NULL, delimsLine, &maoptr );
					break;
				}
				memset(tmpeach,'\0',50);
				strcpy(tmpeach,result);
				resultEach = strtok_r( tmpeach, delimsEach,&eachptr );
				//pid 这个相当于没用
				trueIndex = findTrueIndex(atoi(resultEach));
				//jettop 本轮押注
				resultEach = strtok_r( NULL, delimsEach,&eachptr );
				playerList[trueIndex].jetton = atoi(resultEach);
				//money 剩余金币数
				resultEach = strtok_r( NULL, delimsEach,&eachptr );
				playerList[trueIndex].money = atoi(resultEach);
				//bet   本局剩余投注额
				resultEach = strtok_r( NULL, delimsEach,&eachptr );
				if(atoi(resultEach) - playerList[trueIndex].bet != 0)
					playerList[trueIndex].raisemoney = atoi(resultEach) - playerList[trueIndex].bet;
				else
					playerList[trueIndex].raisemoney = 0;
		
				
				playerList[trueIndex].bet = atoi(resultEach);
	
				//我们应该押注的最少的钱，他们bet的最大值
				if(leastBetMoney < playerList[trueIndex].bet)
				{
					leastBetMoney = playerList[trueIndex].bet;
				}	
				//state 最近的一次有效action    注意这里要判断是跟注all_in还是加注aii-in 无法判断
				/*state*/
				/*
				#define BLINDSTATE	0
				#define CHECK     	1
				#define CALL      	2
				#define RAISE     	3
				#define ALL_IN    	4
				#define FOLD      	5
				*/	
				//0:无;1:blind 2:check 3:call 4:raise 5:跟all_in 6:加all_in 7:fold
				resultEach = strtok_r( NULL, delimsEach,&eachptr );
				if(0 == strcmp(resultEach, "blind"))	state = BLINDSTATE;
				if(0 == strcmp(resultEach, "check"))	state = CHECK;
				if(0 == strcmp(resultEach, "call"))		state = CALL;
				if(0 == strcmp(resultEach, "raise"))	state = RAISE;
				//怎样判断是加注all_in还是跟注all_in  下面的方法是错的
				if(0 == strcmp(resultEach, "all_in"))
				{
						state = ALL_IN;
				}	
				if(0 == strcmp(resultEach, "fold"))
				{
					if(playerList[trueIndex].state != FOLD)
					{
						noFoldNum--;
						playerList[trueIndex].in = OUT;
					}
					state = FOLD;
				}
				
				playerList[trueIndex].state = state;
				playerList[trueIndex].liveordead = LIVE;
				//下一行
				result = strtok_r( NULL, delims, &lineptr );
			}
			totalPot = atoi(resultLine);
			//看一下后面还有没有消息
			
			result = strtok_r( NULL, delims, &lineptr );
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
			{
				goto DealMsg;
			}
			//-------------------------------------------------------------------------------------------//
			/*
			action-msg:
			--------------------------------------------------------------------------
			check | call | raise num | all_in | fold eol
			--------------------------------------------------------------------------
			含义：在收到server的inquire消息后，player发出的行动消息
			方向：player –> server
			方式：单播
			说明：
			行动分为：check（让牌），call（跟注），raise（加注），all_in（全押），fold（弃牌） 
			只有raise（加注）消息需要指明注额，值为在当前注额基础上增加的部分 
			玩家需要做出与手上剩余筹码金额一致的行动决策，否则服务器端将强制纠错。
			
			强制纠错规则： 
				若需要跟注时check，则强制跟注;
				若加注金额少于当前最低注限，则server端强制补齐； 
				若消息接收异常或内容非法，则默认弃牌
			*/
			/*0:无;1:blind 2:check 3:call 4:raise 5:跟all_in 6:加all_in 7:fold*/	
		/**************************************************************************************/
		/**************************************************************************************/
		/**************************************************************************************/
		/**************************************************************************************/
			memset(msg_to_server,'\0',50);
			/*
			  根据两个方面进行判断后得出此次的action
			  mycardDecision 由牌面值判断 int 0 - 100 0最坏 100最好
			  playerDecision 由对手的情况判断 int 1 - 10 1最坏 10最好
			*/
			//mycardDecision已在之前各消息中得到
		//	playerDecision = other_Players();  //1-100
			//printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$playerDecision %u\n",playerDecision);
			
			int finalDecision = 0;
			int blind_rand = 0;
			
			int lastBetMoneyDecision = (playerList[meIndex].jetton - (leastBetMoney - playerList[meIndex].bet));
			lastBetMoneyDecision = (lastBetMoneyDecision < 0 ? 1: lastBetMoneyDecision);
			int callLastBetMoneyDecision = playerList[meIndex].jetton - (leastBetMoney - playerList[meIndex].bet);
			int leastCallMoney = leastBetMoney - playerList[meIndex].bet;
			
			int moneyDecision = money_decision();
			playerDecision = other_Players();
			
			printf("--------curr_orbiculus: %d\n",curr_orbiculus);
			printf("--------playerDecision: %d\n",playerDecision);
			printf("--------mycardDecision: %d\n",mycardDecision);
			
			
			my_previous = findTruePrevious();
			
			//场上只有我 call以收钱
			if(-1 == my_previous)
			{
				snprintf(msg_to_server, sizeof(msg_to_server)-1, "call \n");
			}
			else if( (mycardDecision < 75) && (playerList[my_previous].state == CHECK))
			{
				snprintf(msg_to_server, sizeof(msg_to_server)-1, "check \n");
			}
			else
			{
				//BLIND阶段	
				if(curr_orbiculus == BLIND)
				{	
					finalDecision = 0.5 * mycardDecision + 0.5 * moneyDecision;				
					//-----决策------------//
					if(mycardDecision > 200)
					{
						snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");
					}
					else
					{
						//比较好的情况 保证一定不会弃牌
						if(mycardDecision > 80 && moneyDecision > 90)
						{
							snprintf(msg_to_server, sizeof(msg_to_server)-1, "call \n");
						}
						//比较坏的情况，保证一定弃牌
						else if( mycardDecision < 20 || moneyDecision < 60 )
						{
							snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");
						}
						//中间情况，按照概率弃牌
						else
						{
							snprintf(msg_to_server, sizeof(msg_to_server)-1, "call \n");
						}
					}
				}
				else if(curr_orbiculus == FLOP)
				{
					lastBetMoneyDecision *= (mycardDecision/300.0);
					//如果两者都很好，选择加注 
					if(mycardDecision > 90 && moneyDecision > 90)
					{
						snprintf(msg_to_server, sizeof(msg_to_server)-1, "raise %d \n", lastBetMoneyDecision);
					}
					//比较坏的情况，保证一定弃牌
					else if( mycardDecision < 30 || moneyDecision < 50 )
					{
						snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");
					}
					//中间情况，按照finalDecision判断
					else
					{
						//特殊情况先处理
						if( mycardDecision > 95 || moneyDecision > 95  && mycardDecision > 80)
						{
							snprintf(msg_to_server, sizeof(msg_to_server)-1, "raise %d \n", lastBetMoneyDecision);
						}
						//非特殊情况					
						else
						{
							if(moneyDecision > 90)
							{
								finalDecision = 0.8 * mycardDecision + 0.2 * playerDecision;
							}
							else if(moneyDecision > 70)
							{
								finalDecision = 0.6 * mycardDecision + 0.4 * playerDecision;
							}
							else//(mycardDecision > 70)
							{
								finalDecision = 0.5* mycardDecision + 0.5 * playerDecision;
							}		
							
							//--------------------------------------------------------------
							
							if(finalDecision > 50)
							{
								if( callLastBetMoneyDecision > 0 && callLastBetMoneyDecision < 41)  
								{
									if(playerList[meIndex].money > 500)
										snprintf(msg_to_server, sizeof(msg_to_server)-1, "all_in \n");
									else 
										snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");	
								}
								else
								{
									snprintf(msg_to_server, sizeof(msg_to_server)-1, "call \n");
								}
							}
							else
							{
								snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");	
							}
							
	
						}
					}
				}	
				else if(curr_orbiculus == TURN)
				{
					lastBetMoneyDecision *= (mycardDecision/300.0);
					//如果两者都很好，选择加注 
					if(mycardDecision > 90 && moneyDecision > 90)
					{
						snprintf(msg_to_server, sizeof(msg_to_server)-1, "raise %d \n", lastBetMoneyDecision);
					}
					//比较坏的情况，保证一定弃牌
					else if( mycardDecision < 30 || moneyDecision < 50 )
					{
						snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");
					}
					//中间情况，按照finalDecision判断
					else
					{
						//特殊情况先处理
						if( mycardDecision > 95 || moneyDecision > 95 && mycardDecision > 80)
						{
							snprintf(msg_to_server, sizeof(msg_to_server)-1, "raise %d \n", lastBetMoneyDecision);
						}
						//非特殊情况					
						else
						{
							if(moneyDecision > 90)
							{
								finalDecision = 0.8 * mycardDecision + 0.2 * playerDecision;
							}
							else if(moneyDecision > 70)
							{
								finalDecision = 0.6 * mycardDecision + 0.4 * playerDecision;
							}
							else//(mycardDecision > 70)
							{
								finalDecision = 0.5* mycardDecision + 0.5 * playerDecision;
							}		
							
							//--------------------------------------------------------------
							
							if(finalDecision > 50)
							{
								if( callLastBetMoneyDecision > 0 && callLastBetMoneyDecision < 41)  
								{
									if(playerList[meIndex].money > 500)
										snprintf(msg_to_server, sizeof(msg_to_server)-1, "all_in \n");
									else 
										snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");	
								}
								else
								{
									snprintf(msg_to_server, sizeof(msg_to_server)-1, "call \n");
								}
							}
							else
							{
								snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");	
							}
							
	
						}
					}
				}
				else if(curr_orbiculus == RIVER)
				{
					lastBetMoneyDecision *= (mycardDecision/300.0);
					//如果两者都很好，选择加注 
					if(mycardDecision > 90 && moneyDecision > 90)
					{
						snprintf(msg_to_server, sizeof(msg_to_server)-1, "raise %d \n", lastBetMoneyDecision);
					}
					//比较坏的情况，保证一定弃牌
					else if( mycardDecision < 30 || moneyDecision < 50 )
					{
						snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");
					}
					//中间情况，按照finalDecision判断
					else
					{
						//特殊情况先处理
						if( mycardDecision > 95 || moneyDecision > 95  && mycardDecision > 80)
						{
							snprintf(msg_to_server, sizeof(msg_to_server)-1, "raise %d \n", lastBetMoneyDecision);
						}
						//非特殊情况					
						else
						{
							if(moneyDecision > 90)
							{
								finalDecision = 0.8 * mycardDecision + 0.2 * playerDecision;
							}
							else if(moneyDecision > 70)
							{
								finalDecision = 0.6 * mycardDecision + 0.4 * playerDecision;
							}
							else//(mycardDecision > 70)
							{
								finalDecision = 0.5* mycardDecision + 0.5 * playerDecision;
							}		
							
							//--------------------------------------------------------------
							
							if(finalDecision > 50)
							{
								if( callLastBetMoneyDecision > 0 && callLastBetMoneyDecision < 41)  
								{
									if(playerList[meIndex].money > 500)
										snprintf(msg_to_server, sizeof(msg_to_server)-1, "all_in \n");
									else 
										snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");	
								}
								else
								{
									snprintf(msg_to_server, sizeof(msg_to_server)-1, "call \n");
								}
							}
							else
							{
								snprintf(msg_to_server, sizeof(msg_to_server)-1, "fold \n");	
							}
							
	
						}
					}

				}
			//	printf("mycardDecision = %d\n", mycardDecision);
			//	printf("playerDecision = %d\n", playerDecision);
			//	printf("finalDecision = %d\n", finalDecision);
			}
		/**************************************************************************************/
		/**************************************************************************************/
		/**************************************************************************************/
		/**************************************************************************************/
		    //printf("-----------action %s\n",msg_to_server);
			//printf("@@@action: %s",msg_to_server);
			while(len_to_server = send(m_socket_id, msg_to_server, strlen(msg_to_server), 0)< 0);
			//srand(time(NULL));
			//rand_seed += rand()%99;
			break;
				
			case 5:
			/*e)发出三张公共牌：flop-msg
			--------------------------------------
			flop/ eol
			color point eol
			color point eol
			color point eol
			/flop eol
			--------------------------------------
			含义：server发出三张公共牌
			方向：server –> player
			方式：广播
			*/
			pidIndex = 0;
			resultEach = NULL;
			memset(color,0,10); 							//数据初始化--清零
			memset(point,0,3); 
			curr_orbiculus = FLOP;
			//确定有三行
			while(strcmp(result,"/flop "))
			{
				memset(tmpeach,'\0',50);
				strcpy(tmpeach,result);
				//花色
				resultEach = strtok_r( tmpeach, delimsEach,&eachptr );
				memset(color,'\0',10);
				strcpy(color,resultEach);
				//点数
				resultEach = strtok_r( NULL, delimsEach,&eachptr );
				memset(point,'\0',3);
				strcpy(point,resultEach);
				if(strlen(point)>1)	strcpy(point,"1");
				//处理后保存
				publicCard[pidIndex] = msg_to_bit(color,point[0]);
				//下一行
				result = strtok_r( NULL, delims, &lineptr );
				pidIndex++;
			}
			//更新52张牌
			allCard[0] = myCard[0];
			allCard[1] = myCard[1];
			publicCard[3] = 0;
			publicCard[4] = 0;
			for(cardIndex = 0; cardIndex < 5; cardIndex++)
			{
				allCard[cardIndex + 2] = publicCard[cardIndex];
			}
			update_card(allCard);
			//自己两张+公牌三张
			tmp5card[0] = myCard[0];
			tmp5card[1] = myCard[1];
			for(cardIndex = 0; cardIndex < 3; cardIndex++)
			{
				tmp5card[cardIndex + 2] = publicCard[cardIndex];
			}
			mycardDecision = flop_decision(tmp5card);
			//调试
			for(cardIndex = 0; cardIndex < 5; cardIndex++)
			{
				//printf("allcard[%d] = %X\n",cardIndex,tmp5card[cardIndex]);
			}
			//printf("mycardDecision = %d\n",mycardDecision);
			//
			
			//看一下后面还有没有消息
			
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
				
			break;
				
			case 6:
			/*f)翻牌圈喊注：inquire-msg/action-msg（多次）*/
			/*g)发出一张公共牌（转牌）：turn-msg
			--------------------------------------------
			turn/ eol
			color point eol
			/turn eol
			--------------------------------------------
			含义：server发出一张转牌
			方向：server –> player
			方式：广播	
			*/
			resultEach = NULL;
			memset(color,0,10); 							//数据初始化--清零
			memset(point,0,3); 
			curr_orbiculus = TURN;
			//确定只有一行
			memset(tmpeach,'\0',50);
			strcpy(tmpeach,result);
			//花色
			resultEach = strtok_r( tmpeach, delimsEach,&eachptr );
			memset(color,'\0',10);
			strcpy(color,resultEach);
			//点数
			resultEach = strtok_r( NULL, delimsEach,&eachptr );
			memset(point,'\0',3);
			strcpy(point,resultEach);
			if(strlen(point)>1)	strcpy(point,"1");
			//处理后保存
			publicCard[3] = msg_to_bit(color, point[0]);
			
			//更新52张牌
			allCard[0] = myCard[0];
			allCard[1] = myCard[1];
			for(cardIndex = 0; cardIndex < 5; cardIndex++)
			{
				allCard[cardIndex + 2] = publicCard[cardIndex];
			}
			update_card(allCard);
			//自己两张+公牌四张
			tmp6card[0] = myCard[0];
			tmp6card[1] = myCard[1];
			publicCard[4] = 0;
			for(cardIndex = 0; cardIndex < 4; cardIndex++)
			{
				tmp6card[cardIndex + 2] = publicCard[cardIndex];
			}
			mycardDecision = turn_decision(tmp6card);
			//调试
			for(cardIndex = 0; cardIndex < 6; cardIndex++)
			{
				//printf("allcard[%d] = %X\n",cardIndex,tmp6card[cardIndex]);
			}
			//printf("mycardDecision = %d\n",mycardDecision);
			//
			//看一下后面还有没有消息
			
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
			
			break;
			
			case 7:
			/*h)转牌圈喊注： inquire-msg/action-msg（多次）*/
			/*i)发出一张公共牌（河牌）：river-msg
			----------------------------------------------
			river/ eol
			color point eol
			/river eol
			----------------------------------------------
			含义：server发出一张河牌
			方向：server –> player
			方式：广播
			*/
			resultEach = NULL;
			memset(color,0,10); 							//数据初始化--清零
			memset(point,0,3); 
			curr_orbiculus = RIVER;
			//确定只有一行
			memset(tmpeach,'\0',50);
			strcpy(tmpeach,result);
			//花色
			resultEach = strtok_r( tmpeach, delimsEach,&eachptr );
			memset(color,'\0',10);
			strcpy(color,resultEach);
			//点数
			resultEach = strtok_r( NULL, delimsEach,&eachptr );
			memset(point,'\0',3);
			strcpy(point,resultEach);
			if(strlen(point)>1)	strcpy(point,"1");
			//处理后保存
			publicCard[4] = msg_to_bit(color, point[0]);
			
			//更新52张牌
			allCard[0] = myCard[0];
			allCard[1] = myCard[1];
			for(cardIndex = 0; cardIndex < 5; cardIndex++)
			{
				allCard[cardIndex + 2] = publicCard[cardIndex];
			}
			update_card(allCard);
			//自己两张+公牌五张
			tmp7card[0] = myCard[0];
			tmp7card[1] = myCard[1];
			for(cardIndex = 0; cardIndex < 5; cardIndex++)
			{
				tmp7card[cardIndex + 2] = publicCard[cardIndex];
			}
			mycardDecision = river_decision(tmp7card);
			//调试
			for(cardIndex = 0; cardIndex < 7; cardIndex++)
			{
				//printf("allcard[%d] = %X\n",cardIndex,tmp7card[cardIndex]);
			}
			//printf("mycardDecision = %d\n",mycardDecision);
			//
			
			
			//看一下后面还有没有消息
			
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
			
			break;
			
			case 8:
			/*j)河牌圈喊注：inquire-msg/action-msg（多次）*/
			/*k)若有两家以上未盖牌则摊牌比大小：showdown-msg
			---------------------------------------------
			showdown/ eol
			common/ eol
			(color point eol)5
			/common eol
			(rank: pid color point color point nut_hand eol)2-8
			/showdown eol
			---------------------------------------------
			含义：摊牌消息（含5张公共牌、被摊牌选手的2张手牌及其最佳手牌牌型）
			方向：server –> player
			方式：广播
			说明：只有2人或2人以上未盖牌时，server才发出摊牌消息，盖牌玩家的底牌不被公布
			*/
			//不需要做处理
			//看一下后面还有没有消息
			
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
			
			break;
			
			case 9:
			/*l)公布彩池分配结果：pot-win-msg
			-------------------------------------------
			pot-win/ eol
			(pid: num eol)0-8
			/pot-win eol
			-------------------------------------------
			含义：server公布彩池分配结果
			方向：server –> player
			方式：广播
			说明：如果所有玩家都弃牌，彩池金额全部丢弃
			*/
			//更新playerList便于后面决策判断信息
			//没必要处理在得到inquire消息的时候会有相应的信息供我们用
			//看一下后面还有没有消息
			
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
			
			break;
				
			default:
			//看一下后面还有没有消息
			
			result = strtok_r( NULL, delims, &lineptr );
			if(NULL != result)
				goto DealMsg;
			
			break;
		}//switch
			
		//}//有消息的
	}
	/*3.本场比赛结束（game-over-msg）
	game-over eol
	含义：游戏结束
	方向：server –> player
	方式：广播
	说明：player收到此消息后应当先关闭与server的socket连接，再退出程序。
	*/
	//fclose(pFile); // 关闭文件
	close(m_socket_id);
	return 0;
}
