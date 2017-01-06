#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "key.h"
#include "timer.h"
#include "stepMotor.h"
#include "capture.h"
#include "usart2.h"
#include "lcd.h"
#include <stdlib.h>


u8 userId[2] = {0x55, 0x51};
char park_num[4] = {0};


void connectServer();

void showLcdStr1(u8 * data);
void showLcdStr2(u8 * data);
void showLcdStr3(u8 * data);

char hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

char* toHex(u8 num);
char hexRes[2];
char* toHex(u8 num)
{
	hexRes[0] = hex[num/16];
	hexRes[1] = hex[num%16];
	return hexRes;
}


char  hexString[40] = {0};

char * bytesToHex(u8 cs[], int len)
{
	int i=0, id=0, num;
	
	for (; i<len; i++)
	{
		hexString[id++] = hex[cs[i]/16];
		hexString[id++] = hex[cs[i]%16];
	}
	
	return hexString;
}

void clean_chars(char c[], int len)
{
	int i=0;
	
	for (; i<len; i++)
	{
		c[i] = 0;
	}
}

u8 te[] = {0x4f,0x4b,'g'};



void keyFun(void);
int dir = 1;
int angle = 90;
int open_lock = 0; //车位锁是否开启

 int main(void)
 {	
	u8 t;
	u8 len;	 
 
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(9600);	 //串口初始化为9600
	USART2_Init(9600);
	LED_Init();		  	 //初始化与LED连接的硬件接口 
	KEY_Init();
	TIM2_Int_Init(99,7199);//10Khz的计数频率，计数99为10ms 
	
	motor_init();
	capture_init();
	
	LCD_Init() ;
	
	toHex(userId[0]);
	park_num[0] =  hexRes[0];
	park_num[1] =  hexRes[1];
	toHex(userId[1]);
	park_num[2] =  hexRes[0];
	park_num[3] =  hexRes[1];
	
	sprintf(park_num, "park:%c%c%c%c", park_num[0],park_num[1],park_num[2],park_num[3]);
	LCD_P8x16Str(0,0,park_num);
	 
	LED2=1;
	LED3=0;
	
	USART1_send_data(te,2);//好像串口第一次发送时候第一个字节会没有，故初始化先发送一次
	USART2_send_data(te,2);
	
	connectServer();
	
	while(1)
	{
		keyFun();		
	}	 
}

//0D 0A 43 4F 4E 4E 45 43 54 20 4F 4B 0D 0A

u8 ok[6] = {0x0D,0x0A,0x4F,0x4B,0x0D,0x0A};
volatile u8 connectRequestFlag = 0;
volatile u8 connectOkFlag = 0, sendInfoFlag = 0;
volatile u8 isSending = 0; 

						   
u8 connectOk[] = {0x0D, 0x0A, 0x4F, 0x4B, 0x0D, 0x0A,
				  0x0D, 0x0A, 0x43, 0x4F,
				  0x4E, 0x4E, 0x45, 0x43, 
				  0x54, 0x20, 0x4F, 0x4B, 0x0D, 0x0A};

u8 alreadyCon[] = {0x0D, 0x0A, 0x45, 0x52,
				   0x52, 0x4F, 0x52, 0x0D, 
				   0x0A, 0x0D, 0x0A, 0x41, 
				   0x4C, 0x52, 0x45, 0x41};

u8 sendOK[4] = {0x0D, 0x0A, 0x53, 0x45};

u8 connectIndex = 0;
u8 sendIndex = 0;
u8 recSIM[30];

u8 isConnect(u8 rec[]) 
{
	u8 i = 0;
	for (i=0; i<16; i++)
	{
		if (rec[i] != connectOk[i])
		{
			return 0;
		}
	}
	return 1;
}

u8 isAlreadyConnect(u8 rec[]) 
{
	u8 i = 0;
	for (i=0; i<16; i++)
	{
		if (rec[i] != alreadyCon[i])
		{
			return 0;
		}
	}
	return 1;
}

u8 isSendOK() 
{
	u8 i = 0,sendRec[4] = {0};
	for (i=0; i<4; i++)
	{
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//关闭串口接收中断	
		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == 0);
		sendRec[i] = USART_ReceiveData(USART1);
		//showLcdStr(toHex(sendRec[i]));

		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	}
	
	for (i=0; i<4; i++)
	{
		if (sendRec[i] != sendOK[i]) 
		{
			showLcdStr1("send failed");	
		}
	} 
	if (i == 4) 
	{
	   	showLcdStr1("send ok    ");
	}
	return 1;
}





 //显示发送状态
void showLcdStr1(u8 * data)
{
	LCD_P8x16Str(0,2,"              ");
	LCD_P8x16Str(0,2,data);	
}

 //显示网络连接过程
void showLcdStr2(u8 * data)
{
	LCD_P8x16Str(0,4,"              ");
	LCD_P8x16Str(0,4,data);
}

 //显示网络连接结果
void showLcdStr3(u8 * data)
{
	LCD_P8x16Str(0,6,"              ");
	LCD_P8x16Str(0,6,data);	
}

void registUser()
{
	showLcdStr1("regist     ");								
	printf("AT+CIPSEND\r\n");
	delay_ms(500);
			//sendRequstFlag = 1;
	printf("userId-");
	printf("%c%c%c\r\n",userId[0],userId[1],0x1A);
	//isSendOK();
}

void sendData(u8 * data)
{
	showLcdStr1("sending");								
	printf("AT+CIPSEND\r\n");
	delay_ms(500);

			//sendRequstFlag = 1;
	printf(data);
	printf("%c\r\n",0x1A);
	isSendOK();
}

void sendData2(u8 data[], int len)
{
	showLcdStr1("sending");								
	printf("AT+CIPSEND\r\n");
	delay_ms(500);

			//sendRequstFlag = 1;
	USART1_send_data(data, len);
	delay_ms(100);
	printf("%c\r\n",0x1A);
	isSendOK();
}

void sendDataBefor() 
{
	showLcdStr1("sending");								
	printf("AT+CIPSEND\r\n");
	delay_ms(500);
}

void sendDataEnd() 
{
	printf("%c\r\n",0x1A);
	isSendOK();
}


void connectServer() 
{	
	int time_us = 300;

	showLcdStr1("         ");	
	showLcdStr3("         ");
	
	printf("ATE0\r\n");
	delay_ms(time_us);

	printf("AT+CIPCLOSE=1\r\n");
	showLcdStr2("close1         ");
	delay_ms(time_us);


	printf("AT+CIPSHUT\r\n");
	showLcdStr2("close2         ");
	delay_ms(time_us);

	printf("AT+CGCLASS=\"B\"\r\n");	
	showLcdStr2("1         ");
	delay_ms(time_us);

		
	printf("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n");
	showLcdStr2("2         ");
	delay_ms(time_us);

	printf("AT+CGATT=1\r\n");
	showLcdStr2("3         ");
	delay_ms(time_us);


	printf("AT+CIPCSGP=1,\"CMNET\"\r\n");
	showLcdStr2("4         ");
	delay_ms(time_us);


	printf("AT+CLPORT=\"TCP\",\"2000\"\r\n");
	showLcdStr2("5         ");
	delay_ms(time_us);


	//printf("AT+CIPSTART=\"TCP\",\"haitao.uicp.io\",\"20005\"\r\n");
	printf("AT+CIPSTART=\"TCP\",\"haitao.uicp.io\",\"23712\"\r\n");
	connectRequestFlag = 1;
	showLcdStr2("6         ");
	delay_ms(time_us);
		
}

void keep_connect(void)
{
	
}

void closeServer() 
{
	int time_us = 1000;
	printf("AT+CIPCLOSE=1\r\n");
	showLcdStr2("close1         ");
	delay_ms(time_us);


	printf("AT+CIPSHUT\r\n");
	showLcdStr2("close2         ");
	delay_ms(time_us);
}



u8 fb_all_state[30] = {0x73,0x2d,0x5B};//以's-' 开头，后面依次是每个停车位的状态，从001开始，1表示已使用，0表示未使用

unsigned char rec_cmd[4] = {0x5A,0x00,0x02,0x51};
u8 cmd_string[20] = {0};
int cmd_times = 0;

char rec_index = 1;

#define CAR_PARK_NUM 10 //停车位的数量

char table_state[CAR_PARK_NUM+1] = {0}; //默认有CAR_PARK_NUM个停车位，/
                                        //未用为0，已使用为1，对应下标停车位编号

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	static u8 recCommandFlag = 0;
	u8 res;
	u8 i;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断
	{
		res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		//printf("%c",Res);//返回接收到的数据
		//showLcdNum(Res);
		//判断是否建立连接
		if (connectRequestFlag == 1)
		{
			//showLcdNum(Res);
			recSIM[connectIndex++] = res;
			if (connectIndex == 16) 
			{
				if((isConnect(recSIM) == 1) || (isAlreadyConnect(recSIM) == 1)) 
				{
					showLcdStr3("con ok   ");
					connectOkFlag = 1;
					registUser();									
				}
				else
				{
					showLcdStr3("con fail");
					connectOkFlag = 0;
					registUser();
				}

				connectRequestFlag = 0;
				connectIndex = 0;					
			}						
		}

		//命令模式
		if (recCommandFlag == 1)
		{
			if (res == 0x54)//查询当前停车位的使用情况
			{
				for (i=1; i<CAR_PARK_NUM + 1; i++) //获取每个停车位的状态
				{
					fb_all_state[2+i] = table_state[i];
				}
				
				//很奇怪，自动发送时候第一个 s 字符无法发送出去
				sendData2(fb_all_state, CAR_PARK_NUM + 3);
				showLcdStr2("get state  ");
				//变量复位
				recCommandFlag = 0;
				return;
			}
			
			rec_cmd[rec_index++] = res;
			if (rec_index == 4) //收到打开或者关闭某个车位锁的命令
			{
				//ZigBee广播
				USART2_send_data(rec_cmd, 4);
				
				//状态表更新
				if (rec_cmd[3] == 0x52) //关闭 占用
				{
					table_state[rec_cmd[2]] = 1; //该停车位被占用
				} else if (rec_cmd[3] == 0x51) //打开 未占用
				{
					table_state[rec_cmd[2]] = 0; //该停车位被释放
				}
				
				
				//lcd显示
				cmd_times++;
				sprintf(cmd_string, "cmd:%d  ", cmd_times);				
				showLcdStr3(cmd_string);
				clean_chars(cmd_string, 20);
				
				
				//变量复位
				recCommandFlag = 0;
				rec_index = 1;
				
				
			}
			
			return;	
		}	

		if (res == 0x5A) //接受到协议头0x5A
		{
			recCommandFlag = 1;
		} 
				 
    } 
} 

u8 fb_cmd[] = {0x73,0x2d,0x5a,0x00,0x01,0x53};//以's-' 开头，很奇怪，自动发送时候第一个 s 字符无法发送出去


void USART2_IRQHandler(void)    
{  
    u8 res;
	static int fb_times = 0;
	static u8 recCommandFlag = 0;
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)  
    {       
		res = USART_ReceiveData(USART2);	//读取接收到的数据
		//USART_SendData(USART2, res);
			
		if (res == 0x5A) //接受到协议头0x5A
		{
			recCommandFlag = 1;
			return;
		} 
		
		//命令模式
		if (recCommandFlag == 1)
		{
			rec_cmd[rec_index++] = res;
			if (rec_index == 4) //收到某个停车位的反馈，车辆离开，车位锁打开
			{
				//发送反馈数据到服务器  s - 命令
				fb_cmd[3] = rec_cmd[1];
				fb_cmd[4] = rec_cmd[2];
				fb_cmd[5] = rec_cmd[3];
				
				//很奇怪，自动发送时候第一个 s 字符无法发送出去
				sendData2(fb_cmd, 6);
				
				
				//状态表更新
				table_state[rec_cmd[2]] = 0; //该停车位被释放
				
				//lcd显示
				fb_times++;
				sprintf(cmd_string, "fb:%d  ", fb_times);				
				showLcdStr2(cmd_string);
				clean_chars(cmd_string, 20);
				
				
				//变量复位
				recCommandFlag = 0;
				rec_index = 1;

				
			}
			
			return;	
		}
  }
      
}


unsigned long timer10ms = 1;
void TIM2_IRQHandler(void)   //TIM3中断
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
		timer10ms++;
		if (timer10ms % 50 == 0) 
		{
			LED1=!LED1;
		}
		
		if (timer10ms % (100*60*1) == 0) //3分钟自动重新连接服务器
		{
			//connectServer();
			//keep_connect();
			sendData("00");
		}
	}
}


u8 csfff[4] = {0x12,0x89,0xC4};
char * p;
void keyFun(void)
{
	int t=KEY_Scan(0);		//得到键值
	switch(t)
	{				 
		case KEY1_PRES:
			connectServer();
		
			break;
		case KEY2_PRES:
			keep_connect();
			break;
		case KEY3_PRES:	
			
			//发送反馈数据到服务器  server + 命令
			
			//USART1_send_data(fb_cmd, 6);
			sendData2(fb_cmd, 6);
			break;
		default:;
			delay_ms(10);	
	}
}
