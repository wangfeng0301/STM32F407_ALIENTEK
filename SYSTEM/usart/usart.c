#include "sys.h"
#include "usart.h"
#include "ErrSW.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	   
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//串口1初始化 
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/5/2
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//********************************************************************************
//修改说明
//V1.1 20150411
//修改OS_CRITICAL_METHOD宏判断为：SYSTEM_SUPPORT_OS
////////////////////////////////////////////////////////////////////////////////// 	 
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
		USART1->DR = (u8) ch;      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  

u8 APDU_CLA;
u8 APDU_INS;
u8 APDU_P1;
u8 APDU_P2;
u8 *APDU_DATA;

u16 gRecvApduLen;//要接收的数据长度
u16 gLc,gLe;
u8 *gBufCB = USART_RX_BUF+2;//前两个字节用于发送数据时填充数据长度
u8 *gSendBuf;
u16 siDataLen2Send;//最终要发送的数据长度
u8 UartAPDUFlag = 0;//接收APDU标志位
	#define APDUFLAG 0xF0
	#define APDULEN1 0x02
	#define APDULEN2 0x04
u16 APDULen = 0;

StatusWordStruct gStatusWord;
	
void USART1_IRQHandler(void)
{
	u8 res;	
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART1->SR &(1<<5))//接收到数据
	{
		res=USART1->DR;
		#ifdef UART_APDU
		if(UartAPDUFlag & APDUFLAG)
		{
			if(!(UartAPDUFlag & APDULEN1))
			{
				APDULen = res;
				UartAPDUFlag |= APDULEN1;
			}
			else
			{
				APDULen = (APDULen<<8) + res;
				UartAPDUFlag |= APDULEN2;
				//接收到标志位和长度后，关闭接收中断，使用查询接收
				USART1->CR1&=~(1<<5);    	//关闭接收缓冲区非空中断使能
			}
			return;
			
		}
		if(res == APDUFLAG)//接收APDU的标志位
		{
			UartAPDUFlag = res;
		}
		#else
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(res!=0x0a)
					USART_RX_STA=0;//接收错误,重新开始
				else 
					USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(res==0x0d)
					USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=res;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))
						USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}  	
		#endif
	} 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif										 
//初始化IO 串口1
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率 
void uart_init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV@OVER8=0
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分@OVER8=0 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->AHB1ENR|=1<<0;   	//使能PORTA口时钟  
	RCC->APB2ENR|=1<<4;  	//使能串口1时钟 
	GPIO_Set(GPIOA,PIN9|PIN10,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA9,PA10,复用功能,上拉输出
 	GPIO_AF_Set(GPIOA,9,7);	//PA9,AF7
	GPIO_AF_Set(GPIOA,10,7);//PA10,AF7  	   
	//波特率设置
 	USART1->BRR=mantissa; 	//波特率设置	 
	USART1->CR1&=~(1<<15); 	//设置OVER8=0 
	USART1->CR1|=1<<3;  	//串口发送使能 
#if EN_USART1_RX		  	//如果使能了接收
	//使能接收中断 
	USART1->CR1|=1<<2;  	//串口接收使能
	USART1->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);//组2，最低优先级 
#endif
	USART1->CR1|=1<<13;  	//串口使能
}

/*data:发送的数据
 *len:发送的长度*/
BOOL Uart1_Send(u8 *data,u16 len)
{
	if(data == NULL)//发送数据为空，直接返回TRUE
		return TRUE;
	while(len--)
	{
		while((USART1->SR&(1<<6))==0);//循环发送,直到发送完毕   
		USART1->DR = *data++; 
	}
	return TRUE;
}
/*data:接收的数据首地址
 *len:接受数据长度*/
BOOL Uart1_Receive(u8 *data,u16 len)
{
	u16 i = 0;
	u16 Timeout = 40000;
	if(data == NULL)
		return FALSE;
	while(len--)
	{
		while(!(USART1->SR &(1<<5)) && i++<Timeout);//等待接收到数据
		if(i>=Timeout)
			return FALSE;
		*data++=USART1->DR; 
		i = 0;
	}
	return TRUE;
}
/*接收APDU指令*/
u16 Receive_APDU(void)
{
	u16 RecvDataLen = 0;
	gSW = 0x9000;
	
	if(UartAPDUFlag == (APDUFLAG|APDULEN1|APDULEN2))
	{
		RecvDataLen = APDULen;
		if(RecvDataLen>USART_REC_LEN)
		{
			SET_SW(APDU_LEN_OVERFLOW);//长度溢出
		}
		if(!Uart1_Receive(gBufCB,RecvDataLen))//接收APDU
		{
			SET_SW(RECV_APDU_FAIL);//接收失败
		}
		//接受完之后标志位和长度清零
		UartAPDUFlag = 0;//标志位清零
		APDULen = 0;
		USART1->CR1|=(1<<5);//接收缓冲区非空中断使能
		if(gSW != 0x9000)
		{
			return 2;
		}
		else
		{	
			return RecvDataLen;
		}
	}
	else//标志位和长度不对，直接返回0
	{
		//if(UartAPDUFlag == APDUFLAG && RecvDataLen == 0)
		//	USART1->CR1 |= (1<<5);    	//打开接收缓冲区非空中断使能
		return 0;
	}		
}
/*将接收到的指令解析成APDU格式*/
BOOL CheckAPDUValidation (void)
{
	if(0 == gRecvApduLen)
		WARNING(SW_IO_BUFFER_OVERFLOW);
 	if(gRecvApduLen < 4)
 		WARNING(SW_INS_INSTRUCTION_NO_DEFINE);

	APDU_CLA = gBufCB[0];
	APDU_INS = gBufCB[1];
	APDU_P1 = gBufCB[2];
	APDU_P2 = gBufCB[3];

	gLc = gLe = 0;
	APDU_DATA = &gBufCB[0];
	switch(gRecvApduLen)
	{
		case 4:
			break;
		case 5:
			gLe = gBufCB[4];
			APDU_DATA = &gBufCB[5];
			//if(0 == gLe)
				//gLe = 256;
			break;
		case 6:
			if(0x7E == gBufCB[0]
				&& 0 == gBufCB[4])
			{
				gRecvApduLen = 5;
			}
			else if(1 == gBufCB[4])
			{	
				gLc = 1;
				APDU_DATA = &gBufCB[5];
			}
			else
				WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
			break;
		case 7:
			if(2 == gBufCB[4])
			{
				gLc = 2;
				APDU_DATA = &gBufCB[5];
			}
			else if(0 == gBufCB[4])
			{
				gLe = (gBufCB[5] << 8) + gBufCB[6];
			}
			else if(1 == gBufCB[4])
			{
				gLc = 1;
				gLe = gBufCB[6];
				//if(0 == gLe)
					//gLe = 256;
				APDU_DATA = &gBufCB[5];
			}	
			else
				WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
			break;
		default:
			if(gBufCB[4])
			{
				gLc = gBufCB[4];
				//if(0 == gLc)
					//gLc = 256;
				APDU_DATA = gBufCB + 5;
				if(5 + gLc + 1 == gRecvApduLen)
				{	
					gLe = gBufCB[gRecvApduLen - 1];
					//if(0 == gLe)
						//gLe = 256;
				}	
				else if(5 + gLc != gRecvApduLen)
					WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
			}
			else
			{
				APDU_DATA = gBufCB + 7;
				gLc = (gBufCB[5] << 8) + gBufCB[6];

				if((4 + 3 + gLc + 2) == gRecvApduLen)
				{
					gLe = (((u16)gBufCB[gRecvApduLen - 2])<<8) + gBufCB[gRecvApduLen - 1];
				}
				else if(4 + 3 + gLc != gRecvApduLen)
					WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
			}
			break;
	}
	return TRUE;
}
/*将数据赋值在gSendBuf中，并不真正发送*/
void SendData2Pc (void  *pData, u16 len)
{
	gSendBuf = USART_RX_BUF;
	//gSendBuf+2  = pData;
	siDataLen2Send = len+2;
	gSendBuf[0] = (siDataLen2Send>>8)&0xFF;//发送长度高字节
	gSendBuf[1] = siDataLen2Send&0xFF;//发送长度低字节
}
/*填充状态字，发送*/
void SendSW2Pc(void)
{
	((u8 *)gSendBuf)[siDataLen2Send++] = gSW2;
	((u8 *)gSendBuf)[siDataLen2Send++] = gSW1;
	Uart1_Send(gSendBuf,siDataLen2Send);
}











