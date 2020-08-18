#include <stdio.h>
#include "type.h"
#include "SHA384.h"

/*初始哈希值*/
static const u64 SHA384_H[8] = {0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939, 
								0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4};
/*全局变量，存放每组哈希后的值*/
u64 SHA384_H1[8];
/*初始化常数*/
static const u64 SHA384_K[80] = 
	{		  0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538, 
              0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe, 
              0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 
              0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 
              0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab, 
              0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725, 
              0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 
              0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b, 
              0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218, 
              0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 
              0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 
              0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec, 
              0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c, 
              0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6, 
              0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 
              0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817};
/*************************************************
*function name:Byte2DWord
*function:字节组合成双字
*input:byte-字节   len-输入字节长度,长度必须为8的倍数
*output:dword-双字
**此函数为大端模式，高字节存放在低地址
**************************************************/
static void Byte2DWord(u8 *byte,u64 *dword,u8 len)
{
	u8 i,j;
	for(i=0;i<len;i+=8)
	{
		*(dword+i/8)=0;//清零
		for(j=0;j<8;j++)
			*(dword+i/8) |= (u64)(*(byte+i+j))<<((7-j)*8);
		//*(dword+i/8) = (u64)(*(byte+i))<<56 | (u64)(*(byte+i+1))<<48 | (u64)(*(byte+i+2))<<40 | (u64)(*(byte+i+3))<<32 | (u64)(*(byte+i+4))<<24 |(u64)(*(byte+i+5))<<16 |(u64)(*(byte+i+6))<<8 | (u64)(*(byte+i+7));
	}
}
/*************************************************
*function name:DWord2Byte
*function:双字拆成字节
*input:dword-字 
*output:byte-字节 
**此函数为大端模式，高字节存放在低地址
**************************************************/
static void DWord2Byte(u64 dword, u8 *byte)
{
	u8 i;
	for(i=0;i<8;i++)
		byte[i] = (u8)(dword>>(8*(7-i)))&0xff;
}
/*************************************************
*function name:ROL6464
*function:循环左移
*input:data-64位的双字,len-左移位数
*output:64位双字
**************************************************/
static u64 ROL64(u64 data,u8 len)
{
	u64 l,r;
	l = data<<len;
	r = data>>(64-len);
	return (l|r);
}
/*************************************************
*function name:ROR6464
*function:循环右移
*input:data-64位的双字,len-右移位数
*output:64位双字
**************************************************/
static u64 ROR64(u64 data,u8 len)
{
	u64 l,r;
	r = data>>len;
	l = data<<(64-len);
	return (l|r);
}
/*************************************************
*function name:SHA384_ExtendPlaintext
*function:扩展分组，生成80个子明文
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void SHA384_ExtendPlaintext(u8 *data,u64 *w)
{
	u8 i;
	u64 s[2]={0};
	Byte2DWord(data,w,128);//生成w[0]-w[15]
	for(i=16;i<80;i++)
	{
		s[0] = ROR64(w[i-15],1)^ROR64(w[i-15],8)^(w[i-15]>>7);
		s[1] = ROR64(w[i-2],19)^ROR64(w[i-2],61)^(w[i-2]>>6);
		w[i] = w[i-16]+s[0]+w[i-7]+s[1];//
	}
}
/*************************************************
*function name:SHA384Process
*function:SHA384加密过程
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void SHA384_Process(u8 *data)
{
	u8 i;
	u64 w[80]={0};//存放80个子明文
	u64 S1,ch,temp1,S0,maj,temp2;//中间变量
	u64 a,b,c,d,e,f,g,h;

	SHA384_ExtendPlaintext(data,w);//扩展分组，生成80个子明文

	a=SHA384_H1[0]; b=SHA384_H1[1]; c=SHA384_H1[2]; d=SHA384_H1[3]; e=SHA384_H1[4]; f=SHA384_H1[5]; g=SHA384_H1[6]; h=SHA384_H1[7];//上一轮哈希值

	for(i = 0;i<80;i++)//80轮加密
	{       
        S1 = ROR64(e,14)^ROR64(e,18)^ROR64(e,41);
		ch = (e&f)^((~e)&g);
		temp1 = h+S1+ch+SHA384_K[i]+w[i];
		S0 = ROR64(a,28)^ROR64(a,34)^ROR64(a,39);
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
	SHA384_H1[0] +=a;
	SHA384_H1[1] +=b;
	SHA384_H1[2] +=c;
	SHA384_H1[3] +=d;
	SHA384_H1[4] +=e;
	SHA384_H1[5] +=f;
	SHA384_H1[6] +=g;
	SHA384_H1[7] +=h;
}
/*************************************************
*function name:SHA384
*function:通过秘钥将明文计算出密文信息摘要
*input:	plaintext-明文，len-明文长度(字节)
*output:ciphertext-密文信息摘要
**明文最大长度：2^128 bit,（本例程最大明文长度为2^64 Byte）
**生成摘要长度：512 bit
**************************************************/
void SHA384(u8 *plaintext,u64 bytelen, u8 *ciphertext)
{
	u8 i;
	u8 data[128];		//存放一组128个字节数据
	u64 bitlen[2]={0};	//比特长度

	for(i=0;i<8;i++)	//初始哈希值赋值
		SHA384_H1[i] = SHA384_H[i];

	while(bytelen>=128)		//明文长度大于分组长度
	{
		for(i=0;i<128;i++)
			data[i] = *plaintext++;
		bytelen -=128;	//字节长度减64
		if(bitlen[0] == 0xfffffffffffffc00) //比特长度大于0xffffffffffffffff,进位
		{
			bitlen[1]+=1;
			bitlen[0] = 0;
		}
		else 
			bitlen[0] +=1024;//比特长度加1024
		SHA384_Process(data);//计算	
	}

	for(i=0;i<bytelen;i++)//剩余的明文字节
	{
		data[i] = *plaintext++;
		if(bitlen[0] == 0xfffffffffffffff8) //比特长度大于0xffffffffffffffff,进位
		{
			bitlen[1]+=1;
			bitlen[0] = 0;
		}
		else 
			bitlen[0] +=8;//比特长度加
	}

	data[bytelen]=0x80;//直接补1字节（1000 0000）
	bytelen++;

	/*补完字节后，剩余字节分3种情况:=112；<112；>112*/
	if(bytelen == 112)//情况1，填充明文长度（bit）
	{	
		DWord2Byte(bitlen[1],&data[112]);
		DWord2Byte(bitlen[0],&data[120]);
		SHA384_Process(data);//计算
	}
	else if(bytelen < 112)//情况2，填充字节
	{	
		for(i=bytelen;i<112;i++)
			data[i] = 0x00;
		DWord2Byte(bitlen[1],&data[112]);//填充明文长度（bit）
		DWord2Byte(bitlen[0],&data[120]);
		SHA384_Process(data);//计算
	}
	else if(bytelen >112)//情况3，填充字节
	{
		for(i=bytelen;i<120;i++)
			data[i] = 0x00;
		SHA384_Process(data);//计算
		for(i=0;i<112;i++)//填充字节
			data[i] = 0x00;
		DWord2Byte(bitlen[1],&data[112]);//填充明文长度（bit）
		DWord2Byte(bitlen[0],&data[120]);
		SHA384_Process(data);//计算
	}
	/*加密完成，哈希值转换为字节*/
	for(i=0;i<6;i++)
		DWord2Byte(SHA384_H1[i],ciphertext+8*i);

}