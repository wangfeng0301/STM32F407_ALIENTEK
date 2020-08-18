#ifndef __USART_H
#define __USART_H 
#include "sys.h"
#include "stdio.h"	  
//////////////////////////////////////////////////////////////////////////////////	   
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����1��ʼ�� 
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/5/2
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵��
//V1.1 20150411
//�޸�OS_CRITICAL_METHOD���ж�Ϊ��SYSTEM_SUPPORT_OS
////////////////////////////////////////////////////////////////////////////////// 

#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����

#define UART_APDU

extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	

extern u8 APDU_CLA;
extern u8 APDU_INS;
extern u8 APDU_P1;
extern u8 APDU_P2;
extern u8 *APDU_DATA;
extern u16 siDataLen2Send;//����Ҫ���͵����ݳ���
extern u16 gRecvApduLen;//Ҫ���յ����ݳ���
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
















