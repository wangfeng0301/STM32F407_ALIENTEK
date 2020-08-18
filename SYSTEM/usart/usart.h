#ifndef __USART_H
#define __USART_H 
#include "sys.h"
#include "stdio.h"	  
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

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收

#define UART_APDU

extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	

extern u8 APDU_CLA;
extern u8 APDU_INS;
extern u8 APDU_P1;
extern u8 APDU_P2;
extern u8 *APDU_DATA;
extern u16 siDataLen2Send;//最终要发送的数据长度
extern u16 gRecvApduLen;//要接收的数据长度
extern u16 gLc,gLe;
extern u8 *gSendBuf;
extern u8 *gBufCB;

void uart_init(u32 pclk2,u32 bound); 
u16 Receive_APDU(void);
BOOL Uart1_Send(u8 *data,u16 len);
BOOL CheckAPDUValidation (void);
void SendData2Pc (void  *pData, u16 len);
void SendSW2Pc(void);

#endif	   
















