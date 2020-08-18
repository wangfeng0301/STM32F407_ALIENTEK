#include "sys.h"
#include "usart.h"
#include "ErrSW.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
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
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
		USART1->DR = (u8) ch;      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  

u8 APDU_CLA;
u8 APDU_INS;
u8 APDU_P1;
u8 APDU_P2;
u8 *APDU_DATA;

u16 gRecvApduLen;//Ҫ���յ����ݳ���
u16 gLc,gLe;
u8 *gBufCB = USART_RX_BUF+2;//ǰ�����ֽ����ڷ�������ʱ������ݳ���
u8 *gSendBuf;
u16 siDataLen2Send;//����Ҫ���͵����ݳ���
u8 UartAPDUFlag = 0;//����APDU��־λ
	#define APDUFLAG 0xF0
	#define APDULEN1 0x02
	#define APDULEN2 0x04
u16 APDULen = 0;

StatusWordStruct gStatusWord;
	
void USART1_IRQHandler(void)
{
	u8 res;	
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART1->SR &(1<<5))//���յ�����
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
				//���յ���־λ�ͳ��Ⱥ󣬹رս����жϣ�ʹ�ò�ѯ����
				USART1->CR1&=~(1<<5);    	//�رս��ջ������ǿ��ж�ʹ��
			}
			return;
			
		}
		if(res == APDUFLAG)//����APDU�ı�־λ
		{
			UartAPDUFlag = res;
		}
		#else
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(res!=0x0a)
					USART_RX_STA=0;//���մ���,���¿�ʼ
				else 
					USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(res==0x0d)
					USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=res;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))
						USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}  	
		#endif
	} 
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif										 
//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������ 
void uart_init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV@OVER8=0
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������@OVER8=0 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->AHB1ENR|=1<<0;   	//ʹ��PORTA��ʱ��  
	RCC->APB2ENR|=1<<4;  	//ʹ�ܴ���1ʱ�� 
	GPIO_Set(GPIOA,PIN9|PIN10,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA9,PA10,���ù���,�������
 	GPIO_AF_Set(GPIOA,9,7);	//PA9,AF7
	GPIO_AF_Set(GPIOA,10,7);//PA10,AF7  	   
	//����������
 	USART1->BRR=mantissa; 	//����������	 
	USART1->CR1&=~(1<<15); 	//����OVER8=0 
	USART1->CR1|=1<<3;  	//���ڷ���ʹ�� 
#if EN_USART1_RX		  	//���ʹ���˽���
	//ʹ�ܽ����ж� 
	USART1->CR1|=1<<2;  	//���ڽ���ʹ��
	USART1->CR1|=1<<5;    	//���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);//��2��������ȼ� 
#endif
	USART1->CR1|=1<<13;  	//����ʹ��
}

/*data:���͵�����
 *len:���͵ĳ���*/
BOOL Uart1_Send(u8 *data,u16 len)
{
	if(data == NULL)//��������Ϊ�գ�ֱ�ӷ���TRUE
		return TRUE;
	while(len--)
	{
		while((USART1->SR&(1<<6))==0);//ѭ������,ֱ���������   
		USART1->DR = *data++; 
	}
	return TRUE;
}
/*data:���յ������׵�ַ
 *len:�������ݳ���*/
BOOL Uart1_Receive(u8 *data,u16 len)
{
	u16 i = 0;
	u16 Timeout = 40000;
	if(data == NULL)
		return FALSE;
	while(len--)
	{
		while(!(USART1->SR &(1<<5)) && i++<Timeout);//�ȴ����յ�����
		if(i>=Timeout)
			return FALSE;
		*data++=USART1->DR; 
		i = 0;
	}
	return TRUE;
}
/*����APDUָ��*/
u16 Receive_APDU(void)
{
	u16 RecvDataLen = 0;
	gSW = 0x9000;
	
	if(UartAPDUFlag == (APDUFLAG|APDULEN1|APDULEN2))
	{
		RecvDataLen = APDULen;
		if(RecvDataLen>USART_REC_LEN)
		{
			SET_SW(APDU_LEN_OVERFLOW);//�������
		}
		if(!Uart1_Receive(gBufCB,RecvDataLen))//����APDU
		{
			SET_SW(RECV_APDU_FAIL);//����ʧ��
		}
		//������֮���־λ�ͳ�������
		UartAPDUFlag = 0;//��־λ����
		APDULen = 0;
		USART1->CR1|=(1<<5);//���ջ������ǿ��ж�ʹ��
		if(gSW != 0x9000)
		{
			return 2;
		}
		else
		{	
			return RecvDataLen;
		}
	}
	else//��־λ�ͳ��Ȳ��ԣ�ֱ�ӷ���0
	{
		//if(UartAPDUFlag == APDUFLAG && RecvDataLen == 0)
		//	USART1->CR1 |= (1<<5);    	//�򿪽��ջ������ǿ��ж�ʹ��
		return 0;
	}		
}
/*�����յ���ָ�������APDU��ʽ*/
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
/*�����ݸ�ֵ��gSendBuf�У�������������*/
void SendData2Pc (void  *pData, u16 len)
{
	gSendBuf = USART_RX_BUF;
	//gSendBuf+2  = pData;
	siDataLen2Send = len+2;
	gSendBuf[0] = (siDataLen2Send>>8)&0xFF;//���ͳ��ȸ��ֽ�
	gSendBuf[1] = siDataLen2Send&0xFF;//���ͳ��ȵ��ֽ�
}
/*���״̬�֣�����*/
void SendSW2Pc(void)
{
	((u8 *)gSendBuf)[siDataLen2Send++] = gSW2;
	((u8 *)gSendBuf)[siDataLen2Send++] = gSW1;
	Uart1_Send(gSendBuf,siDataLen2Send);
}











