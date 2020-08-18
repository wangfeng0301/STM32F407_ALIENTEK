#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "rng.h"
#include "aes.h"
#include "des.h"
#include "sha1.h"
#include "sha224.h"
#include "sha256.h"
#include "sha384.h"
#include "sha512.h"
#include "sm4.h"
#include "malloc.h" 
#include "w25qxx.h"    
#include "sdio_sdcard.h"    
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h" 
#include "ErrSW.h"
//ALIENTEK ̽����STM32F407������ ʵ��51
//USB������(Slave)ʵ�� 
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾


USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB״̬
extern vu8 bDeviceState;		//USB���� ���


/*����APDUָ��*/
void ParseExcuteIns (void)
{
	gSW=0x9000;
	//gSendBuf=gBufCB;
	siDataLen2Send=0;

	if(FALSE==CheckAPDUValidation())
		INS_WARNING(0x6D00);

	switch(APDU_INS)
	{
	
		case 0x01://测试串口通信
		{
			if(0x00 != APDU_CLA)
				INS_WARNING(0x6D00);
			mymemset(gBufCB,0xCC,gLe);
			SendData2Pc(gBufCB,gLe);
		}
		break;
		case 0x02:
		{
			if(0x00 != APDU_CLA)
				INS_WARNING(0x6D00);
			if(APDU_P1 == 0x00)
			{
				LED0 = APDU_P2;
				break;
			}
			if(APDU_P1 == 0x01)
			{
				LED1 = APDU_P2;
				break;
			}
		}
		break;
		case 0x03://加密算法，哈希算法
		{
			if(APDU_P1 == 0x00)//哈希算法
			{
				switch(APDU_P2)
				{
					case 0://sha1
						SHA1(APDU_DATA,gLc,gBufCB);
					break;
					case 1://sha224
						SHA224(APDU_DATA,gLc,gBufCB);
					break;
					case 2://sha256
						SHA256(APDU_DATA,gLc,gBufCB);
					break;
					case 3://sha384
						SHA384(APDU_DATA,gLc,gBufCB);
					break;
					case 4://sha512
						SHA512(APDU_DATA,gLc,gBufCB);
					break;
					default:
						INS_WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
				}
				SendData2Pc(gBufCB,gLe);
			}
			else if(APDU_P1 == 0x01)//对称加密算法
			{
				switch(APDU_P2)
				{
					u8 key[16];
					case 0://aes加密
						mymemcpy(key,APDU_DATA,16);//aes秘钥16字节
						AES(APDU_DATA+16, key, gBufCB);
					break;
					case 1://aes解密
						mymemcpy(key,APDU_DATA,16);//aes秘钥16字节
						DeAES(APDU_DATA+16, key, gBufCB);
					break;
					case 2://des加密解密
						mymemcpy(key,APDU_DATA,8);//aes秘钥8字节
						DES(APDU_DATA+8, key, gBufCB);
					break;
					case 3://sm4加密
						mymemcpy(key,APDU_DATA,16);//sm4秘钥16字节
						SM4(APDU_DATA+16, key, gBufCB);
					break;
					case 4://SM4解密
						mymemcpy(key,APDU_DATA,16);//sm4秘钥16字节
						DeSM4(APDU_DATA+16, key, gBufCB);
					default:
						INS_WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
				}
				SendData2Pc(gBufCB,gLe);
			}
			else if(APDU_P1 == 0x02)//CRC校验
			{
				switch(APDU_P2)
				{
					case 0://sha1
				
					break;
					case 1://sha224
				
					break;
					case 2://sha256
				
					break;
					case 3://sha384
				
					break;
					case 4://sha512
				
					break;
					default:
						INS_WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
				}
			}
			else
				INS_WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
		}
		break;
		case 0x84://get rand
		{
			u16 i;
			if(0x00 != APDU_CLA)
				INS_WARNING(0x6D00);
			if(RNG_Init())
				INS_WARNING(0x6E00);
			for(i = 0;i<gLe;i++)
				gBufCB[i] = RNG_Get_RandomNum()%256;
			SendData2Pc(gBufCB,gLe);
		}
		break;
		

		default:
			INS_WARNING(SW_INS_INSTRUCTION_NO_DEFINE);

	}
}

int main(void)
{         
	u8 offline_cnt=0;
	u8 tct=0;
	u8 USB_STA;
	u8 Divece_STA;

	Stm32_Clock_Init(336,8,2,7);//����ʱ��,168Mhz 
	delay_init(168);			//��ʱ��ʼ��  
	uart_init(84,115200);		//��ʼ�����ڲ�����Ϊ115200 
	LED_Init();					//��ʼ��LED  
 	LCD_Init();					//LCD��ʼ��  
 	KEY_Init();					//������ʼ��  
	W25QXX_Init();				//��ʼ��W25Q128  
	
	POINT_COLOR=RED;//��������Ϊ��ɫ
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"APDU TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2019/12/17");
	
	while(1)
	{
		if(0 != (gRecvApduLen = Receive_APDU()))
		{
			ParseExcuteIns();
			SendSW2Pc();
		}
	}
#if 0  
 	POINT_COLOR=RED;//��������Ϊ��ɫ
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"USB Card Reader TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2014/7/21");	
	if(SD_Init())LCD_ShowString(30,130,200,16,16,"SD Card Error!");	//���SD������
	else //SD ������
	{   															  
		LCD_ShowString(30,130,200,16,16,"SD Card Size:     MB"); 
 		LCD_ShowNum(134,130,SDCardInfo.CardCapacity>>20,5,16);	//��ʾSD������
 	}
	if(W25QXX_ReadID()!=W25Q128)LCD_ShowString(30,130,200,16,16,"W25Q128 Error!");	//���W25Q128����
	else //SPI FLASH ����
	{   														 
		LCD_ShowString(30,150,200,16,16,"SPI FLASH Size:12MB");	 
	}  
 	LCD_ShowString(30,170,200,16,16,"USB Connecting...");//��ʾ���ڽ������� 	    
	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
	delay_ms(1800);	
	while(1)
	{	
		delay_ms(1);				  
		if(USB_STA!=USB_STATUS_REG)//״̬�ı��� 
		{	 						   
			LCD_Fill(30,190,240,190+16,WHITE);//�����ʾ			  	   
			if(USB_STATUS_REG&0x01)//����д		  
			{
				LED1=0;
				LCD_ShowString(30,190,200,16,16,"USB Writing...");//��ʾUSB����д������	 
			}
			if(USB_STATUS_REG&0x02)//���ڶ�
			{
				LED1=0;
				LCD_ShowString(30,190,200,16,16,"USB Reading...");//��ʾUSB���ڶ�������  		 
			}	 										  
			if(USB_STATUS_REG&0x04)LCD_ShowString(30,210,200,16,16,"USB Write Err ");//��ʾд�����
			else LCD_Fill(30,210,240,210+16,WHITE);//�����ʾ	  
			if(USB_STATUS_REG&0x08)LCD_ShowString(30,230,200,16,16,"USB Read  Err ");//��ʾ��������
			else LCD_Fill(30,230,240,230+16,WHITE);//�����ʾ    
			USB_STA=USB_STATUS_REG;//��¼����״̬
		}
		if(Divece_STA!=bDeviceState) 
		{
			if(bDeviceState==1)LCD_ShowString(30,170,200,16,16,"USB Connected    ");//��ʾUSB�����Ѿ�����
			else LCD_ShowString(30,170,200,16,16,"USB DisConnected ");//��ʾUSB���γ���
			Divece_STA=bDeviceState;
		}
		tct++;
		if(tct==200)
		{
			tct=0;
			LED1=1;
			LED0=!LED0;//��ʾϵͳ������
			if(USB_STATUS_REG&0x10)
			{
				offline_cnt=0;//USB������,�����offline������
				bDeviceState=1;
			}else//û�еõ���ѯ 
			{
				offline_cnt++;  
				if(offline_cnt>10)bDeviceState=0;//2s��û�յ����߱��,����USB���γ���
			}
			USB_STATUS_REG=0;
		}
	};
#endif
}
















