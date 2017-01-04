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
int open_lock = 0; //��λ���Ƿ���

 int main(void)
 {	
	u8 t;
	u8 len;	 
 
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	uart_init(9600);	 //���ڳ�ʼ��Ϊ9600
	USART2_Init(9600);
	LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ� 
	KEY_Init();
	TIM2_Int_Init(99,7199);//10Khz�ļ���Ƶ�ʣ�����99Ϊ10ms 
	
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
	
	USART1_send_data(te,2);//���񴮿ڵ�һ�η���ʱ���һ���ֽڻ�û�У��ʳ�ʼ���ȷ���һ��
	USART2_send_data(te,2);
	
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
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//�رմ��ڽ����ж�	
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





 //��ʾ����״̬
void showLcdStr1(u8 * data)
{
	LCD_P8x16Str(0,2,data);	
}

 //��ʾ�������ӹ���
void showLcdStr2(u8 * data)
{
	LCD_P8x16Str(0,4,data);
}

 //��ʾ�������ӽ��
void showLcdStr3(u8 * data)
{
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
	int time_us = 500;

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



u8 fb_all_state[30] = {0x73,0x2d,0x5B};//��'s-' ��ͷ������������ÿ��ͣ��λ��״̬����001��ʼ��1��ʾ��ʹ�ã�0��ʾδʹ��

unsigned char rec_cmd[4] = {0x5A,0x00,0x02,0x51};
u8 cmd_string[20] = {0};
int cmd_times = 0;

char rec_index = 1;

char table_state[21] = {0}; //Ĭ����20��ͣ��λ��δ��Ϊ0����ʹ��Ϊ1����Ӧ�±�ͣ��λ���

void USART1_IRQHandler(void)                	//����1�жϷ������
{
	static u8 recCommandFlag = 0;
	u8 res;
	u8 i;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�
	{
		res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		//printf("%c",Res);//���ؽ��յ�������
		//showLcdNum(Res);
		//�ж��Ƿ�������
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
				}

				connectRequestFlag = 0;
				connectIndex = 0;					
			}						
		}

		//����ģʽ
		if (recCommandFlag == 1)
		{
			if (res == 0x54)//��ѯ��ǰͣ��λ��ʹ�����
			{
				for (i=1; i<21; i++) //��ȡÿ��ͣ��λ��״̬
				{
					fb_all_state[2+i] = table_state[i];
				}
				
				//����֣��Զ�����ʱ���һ�� s �ַ��޷����ͳ�ȥ
				sendData2(fb_all_state, 23);
				showLcdStr2("get state  ");
				//������λ
				recCommandFlag = 0;
				return;
			}
			
			rec_cmd[rec_index++] = res;
			if (rec_index == 4) //�յ��򿪻��߹ر�ĳ����λ��������
			{
				//ZigBee�㲥
				USART2_send_data(rec_cmd, 4);
				
				//״̬������
				if (rec_cmd[3] == 0x52) //�ر� ռ��
				{
					table_state[rec_cmd[2]] = 1; //��ͣ��λ��ռ��
				} else if (rec_cmd[3] == 0x51) //�� δռ��
				{
					table_state[rec_cmd[2]] = 0; //��ͣ��λ���ͷ�
				}
				
				
				//lcd��ʾ
				cmd_times++;
				sprintf(cmd_string, "cmd:%d  ", cmd_times);				
				showLcdStr3(cmd_string);
				clean_chars(cmd_string, 20);
				
				
				//������λ
				recCommandFlag = 0;
				rec_index = 1;
				
				
			}
			
			return;	
		}	

		if (res == 0x5A) //���ܵ�Э��ͷ0x5A
		{
			recCommandFlag = 1;
		} 
				 
    } 
} 

u8 fb_cmd[] = {0x73,0x2d,0x5a,0x00,0x01,0x53};//��'s-' ��ͷ������֣��Զ�����ʱ���һ�� s �ַ��޷����ͳ�ȥ


void USART2_IRQHandler(void)    
{  
    u8 res;
	static int fb_times = 0;
	static u8 recCommandFlag = 0;
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)  
    {       
		res = USART_ReceiveData(USART2);	//��ȡ���յ�������
		//USART_SendData(USART2, res);
			
		if (res == 0x5A) //���ܵ�Э��ͷ0x5A
		{
			recCommandFlag = 1;
			return;
		} 
		
		//����ģʽ
		if (recCommandFlag == 1)
		{
			rec_cmd[rec_index++] = res;
			if (rec_index == 4) //�յ�ĳ��ͣ��λ�ķ����������뿪����λ����
			{
				//���ͷ������ݵ�������  s - ����
				fb_cmd[3] = rec_cmd[1];
				fb_cmd[4] = rec_cmd[2];
				fb_cmd[5] = rec_cmd[3];
				
				//����֣��Զ�����ʱ���һ�� s �ַ��޷����ͳ�ȥ
				sendData2(fb_cmd, 6);
				
				
				//״̬������
				table_state[rec_cmd[2]] = 0; //��ͣ��λ���ͷ�
				
				//lcd��ʾ
				fb_times++;
				sprintf(cmd_string, "fb:%d  ", fb_times);				
				showLcdStr2(cmd_string);
				clean_chars(cmd_string, 20);
				
				
				//������λ
				recCommandFlag = 0;
				rec_index = 1;

				
			}
			
			return;	
		}
		
    }
      
}


unsigned int timer10ms = 0;
void TIM2_IRQHandler(void)   //TIM3�ж�
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ 
		timer10ms++;
		if (timer10ms % 50 == 0) 
		{
			LED1=!LED1;
		}
		
		if (timer10ms % 100 == 0) 
		{
			
		}
	}
}


u8 csfff[4] = {0x12,0x89,0xC4};
char * p;
void keyFun(void)
{
	int t=KEY_Scan(0);		//�õ���ֵ
	switch(t)
	{				 
		case KEY1_PRES:
			connectServer();
		
			break;
		case KEY2_PRES:
			
			break;
		case KEY3_PRES:	
			
			//���ͷ������ݵ�������  server + ����
			
			//USART1_send_data(fb_cmd, 6);
			sendData2(fb_cmd, 6);
			break;
		default:;
			delay_ms(10);	
	}
}