#include <stdio.h>
#include "type.h"
#include "SHA256.h"

/*初始哈希值*/
static const u32 SHA256_H[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
						 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
/*全局变量，存放每组哈希后的值*/
u32 SHA256_H1[8];
/*初始化常数*/
static const u32 SHA256_K[64] = 
	{  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
/*************************************************
*function name:Byte2Word
*function:字节组合成字
*input:byte-字节   len-输入字节长度,长度必须为4的倍数
*output:word-字
**此函数为大端模式，高字节存放在低地址
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
**此函数为大端模式，高字节存放在低地址
**************************************************/
static void Word2Byte(u32 word, u8 *byte)
{
	u8 i;
	for(i=0;i<4;i++)
		byte[i] = (u8)(word>>(8*(3-i)))&0xff;
}
/*************************************************
*function name:ROL32
*function:循环左移
*input:data-32位的字,len-左移位数
*output:32位字
**************************************************/
static u32 ROL32(u32 data,u8 len)
{
	u32 l,r;
	l = data<<len;
	r = data>>(32-len);
	return (l|r);
}
/*************************************************
*function name:ROR32
*function:循环右移
*input:data-32位的字,len-右移位数
*output:32位字
**************************************************/
static u32 ROR32(u32 data,u8 len)
{
	u32 l,r;
	r = data>>len;
	l = data<<(32-len);
	return (l|r);
}
/*************************************************
*function name:SHA256_ExtendPlaintext
*function:扩展分组，生成80个子明文
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void SHA256_ExtendPlaintext(u8 *data,u32 *w)
{
	u8 i;
	u32 s[2]={0};
	Byte2Word(data,w,64);//生成w[0]-w[15]
	for(i=16;i<64;i++)
	{
		s[0] = ROR32(w[i-15],7)^ROR32(w[i-15],18)^(w[i-15]>>3);
		s[1] = ROR32(w[i-2],17)^ROR32(w[i-2],19)^(w[i-2]>>10);
		w[i] = w[i-16]+s[0]+w[i-7]+s[1];//
	}
}
/*************************************************
*function name:Sha256Process
*function:SHA256加密过程
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void SHA256_Process(u8 *data)
{
	u8 i;
	u32 w[64];//存放64个子明文
	u32 S1,ch,temp1,S0,maj,temp2;//中间变量
	u32 a,b,c,d,e,f,g,h;

	SHA256_ExtendPlaintext(data,w);//扩展分组，生成80个子明文

	a=SHA256_H1[0]; b=SHA256_H1[1]; c=SHA256_H1[2]; d=SHA256_H1[3]; e=SHA256_H1[4]; f=SHA256_H1[5]; g=SHA256_H1[6]; h=SHA256_H1[7];//上一轮哈希值

	for(i = 0;i<64;i++)//64轮加密
	{
        
        S1 = ROR32(e,6)^ROR32(e,11)^ROR32(e,25);
		ch = (e&f)^((~e)&g);
		temp1 = h+S1+ch+SHA256_K[i]+w[i];
		S0 = ROR32(a,2)^ROR32(a,13)^ROR32(a,22);
		maj = (a&b)^(a&c)^(b&c);
		temp2 = S0+maj;

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1+temp2;
    }
	SHA256_H1[0] +=a;
	SHA256_H1[1] +=b;
	SHA256_H1[2] +=c;
	SHA256_H1[3] +=d;
	SHA256_H1[4] +=e;
	SHA256_H1[5] +=f;
	SHA256_H1[6] +=g;
	SHA256_H1[7] +=h;
}
/*************************************************
*function name:SHA256
*function:通过秘钥将明文计算出密文信息摘要
*input:	plaintext-明文，len-明文长度(字节)
*output:ciphertext-密文信息摘要
**明文最大长度：2^64 bit,（本例程最大明文长度为2^32 Byte,即4GB）
**生成摘要长度：256 bit
**************************************************/
void SHA256(u8 *plaintext,u32 bytelen, u8 *ciphertext)
{
	u8 i;
	u8 data[64];		//存放一组64个字节数据
	u32 bitlen[2]={0};	//比特长度

	for(i=0;i<8;i++)	//初始哈希值赋值
		SHA256_H1[i] = SHA256_H[i];

	while(bytelen>=64)		//明文长度大于分组长度
	{
		for(i=0;i<64;i++)
			data[i] = *plaintext++;
		bytelen -=64;	//字节长度减64
		if(bitlen[0] == 0xfffffe00) //比特长度大于0xffffffff,进位
		{
			bitlen[1]+=1;
			bitlen[0] = 0;
		}
		else 
			bitlen[0] +=512;//比特长度加512
		SHA256_Process(data);//计算	
	}

	for(i=0;i<bytelen;i++)//剩余的明文字节
	{
		data[i] = *plaintext++;
		if(bitlen[0] == 0xfffffff8) //比特长度大于0xffffffff,进位
		{
			bitlen[1]+=1;
			bitlen[0] = 0;
		}
		else 
			bitlen[0] +=8;//比特长度加
	}

	data[bytelen]=0x80;//直接补1字节（1000 0000）
	bytelen++;

	/*补完字节后，剩余字节分3种情况:=56；<56；>56*/
	if(bytelen == 56)//情况1，填充明文长度（bit）
	{	
		Word2Byte(bitlen[1],&data[56]);
		Word2Byte(bitlen[0],&data[60]);
		SHA256_Process(data);//计算
	}
	else if(bytelen < 56)//情况2，填充字节
	{	
		for(i=bytelen;i<56;i++)
			data[i] = 0x00;
		Word2Byte(bitlen[1],&data[56]);//填充明文长度（bit）
		Word2Byte(bitlen[0],&data[60]);
		SHA256_Process(data);//计算
	}
	else if(bytelen >56)//情况3，填充字节
	{
		for(i=bytelen;i<64;i++)
			data[i] = 0x00;
		SHA256_Process(data);//计算
		for(i=0;i<56;i++)//填充字节
			data[i] = 0x00;
		Word2Byte(bitlen[1],&data[56]);//填充明文长度（bit）
		Word2Byte(bitlen[0],&data[60]);
		SHA256_Process(data);//计算
	}
	/*加密完成，哈希值转换为字节*/
	for(i=0;i<8;i++)
		Word2Byte(SHA256_H1[i],ciphertext+4*i);

}