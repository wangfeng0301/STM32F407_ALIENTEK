#include <stdio.h>
#include "type.h"
#include "SM4.h"

u32 MK[4]={0};//加密秘钥
u32 rk[36]={0};//轮秘钥，由加密秘钥生成
u32 CK[32]={0};//固定参数用于秘钥扩展
u32 X[36]={0};//中间值

//系统参数
static const u32 FK[4]={0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC};
/*S盒*/
static const u8 S_BOX[16][16]=
	{	0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
		0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
		0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
		0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
		0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
		0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
		0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
		0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
		0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
		0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
		0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
		0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
		0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
		0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd, 0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
		0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
		0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48};

/*************************************************
*function name:Byte2Word
*function:字节组合成字
*input:byte-字节   len-输入字节长度,长度必须为4的倍数
*output:word-字
**************************************************/
static void Byte2Word(u8 *byte,u32 *word,u8 len)
{
	u8 i;
	for(i=0;i<len;i+=4)
		*(word+i/4) = *(byte+i)<<24 | *(byte+i+1)<<16 | *(byte+i+2)<<8 | *(byte+i+3);
}
/*************************************************
*function name:Word2Byte
*function:字拆成字节
*input:word-字 
*output:byte-字节 
**************************************************/
static void Word2Byte(u32 word, u8 *byte)
{
	u8 i;
	for(i=0;i<4;i++)
		byte[i] = (u8)(word>>(8*(3-i)))&0xff;
}
/*************************************************
*function name:Sconversion
*function:S盒变换
*input:data-8位的字
*output:8位字
**************************************************/
static u8 SConversion(u8 data)
{
	u8 line;//行
	u8 col;//列
	line = (data>>4)&0x0f;
	col = data&0x0f;
	return S_BOX[line][col];
}
/*************************************************
*function name:Tao
*function:非线性变换
*input:data-32位的字
*output:32位字
**************************************************/
static u32 Tao(u32 data)
{
	u8 i;
	u32 temp=0;
	for(i=0;i<4;i++)
		temp |= SConversion((data>>(8*(3-i)))&0x000000ff)<<(8*(3-i));
	return temp;
}
/*************************************************
*function name:ROL
*function:循环左移
*input:data-32位的字,len-左移位数
*output:32位字
**************************************************/
static u32 ROL(u32 data,u8 len)
{
	u32 l,r;
	l = data<<len;
	r = data>>(32-len);
	return (l|r);
}
/*************************************************
*function name:L
*function:线性变换
*input:data-32位的字
*output:32位字
**************************************************/
static u32 L(u32 data)
{
	return (data^ROL(data,2)^ROL(data,10)^ROL(data,18)^ROL(data,24));
}
/*************************************************
*function name:T
*function:合成置换
*input:data-32位的字
*output:32位字
**************************************************/
static u32 T(u32 data)
{
	return L(Tao(data));
}
/*************************************************
*function name:L1
*function:线性变换,生成轮秘钥时使用
*input:data-32位的字
*output:32位字
**************************************************/
static u32 L1(u32 data)
{
	return (data^ROL(data,13)^ROL(data,23));
}
/*************************************************
*function name:T1
*function:合成置换,生成轮秘钥时使用
*input:data-32位的字
*output:32位字
**************************************************/
static u32 T1(u32 data)
{
	return L1(Tao(data));
}
/*************************************************
*function name:F
*function:轮函数
*input:X-输入4个字   rk-轮秘钥
*output:32位字
**************************************************/
static u32 F(u32 x0, u32 x1, u32 x2, u32 x3, u32 rk)
{
	return (x0^T(x1^x2^x3^rk));
}

/*************************************************
*function name:Antitone
*function:反序变换
*input:data-输入4个字
*output:32位字
**************************************************/
static u32 Antitone(u32 data)
{
	
}

/*************************************************
*function name:ExtendKey
*function:扩展秘钥
*input:	key-秘钥
*output:void
**************************************************/
static void SM4_ExtendKey(u8 *key)
{
	u8 i,j;
	u8 ck[4];

	Byte2Word(key,MK,16);		//加密秘钥变换成字
	for(i=0;i<4;i++)			//生成初始轮秘钥
		rk[i] = MK[i]^FK[i];
	for(i=0;i<32;i++)			//生成CK
	{
		for(j=0;j<4;j++)
			ck[j] = ((i<<2)+j)*7%256;
		Byte2Word(ck,&CK[i],4);
	}
	for(i=0;i<32;i++)			//生成轮秘钥
		rk[i+4] = rk[i]^T1(rk[i+1]^rk[i+2]^rk[i+3]^CK[i]);
}
/*************************************************
*function name:SM4
*function:通过秘钥将明文计算出密文
*input:	plaintext-明文，key-秘钥
*output:ciphertext-密文
**************************************************/
void SM4(u8 *plaintext,u8 *key, u8 *ciphertext)
{
	u8 i;
	SM4_ExtendKey(key);			//生成轮秘钥
	Byte2Word(plaintext,X,16);	//明文变换成字
	for(i=0;i<32;i++)			//加密变换
		X[i+4] = F(X[i],X[i+1],X[i+2],X[i+3],rk[i+4]);
	for(i=0;i<4;i++)			//得到最终密文
		Word2Byte(X[35-i],ciphertext+4*i);
}

/*************************************************
*function name:SM4
*function:通过秘钥将明文计算出密文
*input:	plaintext-明文，key-秘钥
*output:ciphertext-密文
**************************************************/
void DeSM4(u8 *ciphertext,u8 *key, u8 *plaintext)
{
	u8 i;
	SM4_ExtendKey(key);			//生成轮秘钥
	Byte2Word(plaintext,X,16);	//明文变换成字
	for(i=0;i<32;i++)			//加密变换
		X[i+4] = F(X[i],X[i+1],X[i+2],X[i+3],rk[35-i]);
	for(i=0;i<4;i++)			//得到最终密文
		Word2Byte(X[35-i],ciphertext+4*i);
}

