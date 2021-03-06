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
//ALIENTEK 探索者STM32F407开发板 实验51
//USB读卡器(Slave)实验 
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司


USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB状态
extern vu8 bDeviceState;		//USB连接 情况


/*解析APDU指令*/
void ParseExcuteIns (void)
{
	gSW=0x9000;
	//gSendBuf=gBufCB;
	siDataLen2Send=0;

	if(FALSE==CheckAPDUValidation())
		INS_WARNING(0x6D00);

	switch(APDU_INS)
	{
	
		case 0x01://娴嬭瘯涓插彛閫氫俊
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
		case 0x03://鍔犲瘑绠楁硶锛屽搱甯岀畻娉�
		{
			if(APDU_P1 == 0x00)//鍝堝笇绠楁硶
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
			else if(APDU_P1 == 0x01)//瀵圭О鍔犲瘑绠楁硶
			{
				switch(APDU_P2)
				{
					u8 key[16];
					case 0://aes鍔犲瘑
						mymemcpy(key,APDU_DATA,16);//aes绉橀挜16瀛楄妭
						AES(APDU_DATA+16, key, gBufCB);
					break;
					case 1://aes瑙ｅ瘑
						mymemcpy(key,APDU_DATA,16);//aes绉橀挜16瀛楄妭
						DeAES(APDU_DATA+16, key, gBufCB);
					break;
					case 2://des鍔犲瘑瑙ｅ瘑
						mymemcpy(key,APDU_DATA,8);//aes绉橀挜8瀛楄妭
						DES(APDU_DATA+8, key, gBufCB);
					break;
					case 3://sm4鍔犲瘑
						mymemcpy(key,APDU_DATA,16);//sm4绉橀挜16瀛楄妭
						SM4(APDU_DATA+16, key, gBufCB);
					break;
					case 4://SM4瑙ｅ瘑
						mymemcpy(key,APDU_DATA,16);//sm4绉橀挜16瀛楄妭
						DeSM4(APDU_DATA+16, key, gBufCB);
					default:
						INS_WARNING(SW_INS_INSTRUCTION_NO_DEFINE);
				}
				SendData2Pc(gBufCB,gLe);
			}
			else if(APDU_P1 == 0x02)//CRC鏍￠獙
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

	Stm32_Clock_Init(336,8,2,7);//设置时钟,168Mhz 
	delay_init(168);			//延时初始化  
	uart_init(84,115200);		//初始化串口波特率为115200 
	LED_Init();					//初始化LED  
 	LCD_Init();					//LCD初始化  
 	KEY_Init();					//按键初始化  
	W25QXX_Init();				//初始化W25Q128  
	
	POINT_COLOR=RED;//设置字体为红色
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
 	POINT_COLOR=RED;//设置字体为红色
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"USB Card Reader TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2014/7/21");	
	if(SD_Init())LCD_ShowString(30,130,200,16,16,"SD Card Error!");	//检测SD卡错误
	else //SD 卡正常
	{   															  
		LCD_ShowString(30,130,200,16,16,"SD Card Size:     MB"); 
 		LCD_ShowNum(134,130,SDCardInfo.CardCapacity>>20,5,16);	//显示SD卡容量
 	}
	if(W25QXX_ReadID()!=W25Q128)LCD_ShowString(30,130,200,16,16,"W25Q128 Error!");	//检测W25Q128错误
	else //SPI FLASH 正常
	{   														 
		LCD_ShowString(30,150,200,16,16,"SPI FLASH Size:12MB");	 
	}  
 	LCD_ShowString(30,170,200,16,16,"USB Connecting...");//提示正在建立连接 	    
	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
	delay_ms(1800);	
	while(1)
	{	
		delay_ms(1);				  
		if(USB_STA!=USB_STATUS_REG)//状态改变了 
		{	 						   
			LCD_Fill(30,190,240,190+16,WHITE);//清除显示			  	   
			if(USB_STATUS_REG&0x01)//正在写		  
			{
				LED1=0;
				LCD_ShowString(30,190,200,16,16,"USB Writing...");//提示USB正在写入数据	 
			}
			if(USB_STATUS_REG&0x02)//正在读
			{
				LED1=0;
				LCD_ShowString(30,190,200,16,16,"USB Reading...");//提示USB正在读出数据  		 
			}	 										  
			if(USB_STATUS_REG&0x04)LCD_ShowString(30,210,200,16,16,"USB Write Err ");//提示写入错误
			else LCD_Fill(30,210,240,210+16,WHITE);//清除显示	  
			if(USB_STATUS_REG&0x08)LCD_ShowString(30,230,200,16,16,"USB Read  Err ");//提示读出错误
			else LCD_Fill(30,230,240,230+16,WHITE);//清除显示    
			USB_STA=USB_STATUS_REG;//记录最后的状态
		}
		if(Divece_STA!=bDeviceState) 
		{
			if(bDeviceState==1)LCD_ShowString(30,170,200,16,16,"USB Connected    ");//提示USB连接已经建立
			else LCD_ShowString(30,170,200,16,16,"USB DisConnected ");//提示USB被拔出了
			Divece_STA=bDeviceState;
		}
		tct++;
		if(tct==200)
		{
			tct=0;
			LED1=1;
			LED0=!LED0;//提示系统在运行
			if(USB_STATUS_REG&0x10)
			{
				offline_cnt=0;//USB连接了,则清除offline计数器
				bDeviceState=1;
			}else//没有得到轮询 
			{
				offline_cnt++;  
				if(offline_cnt>10)bDeviceState=0;//2s内没收到在线标记,代表USB被拔出了
			}
			USB_STATUS_REG=0;
		}
	};
#endif
}
















