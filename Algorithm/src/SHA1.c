#include <stdio.h>
#include "type.h"
#include "SHA1.h"

/*链接变量*/
static const u32 A1 = 0x67452301;
static const u32 B1 = 0xEFCDAB89;
static const u32 C1 = 0x98BADCFE;
static const u32 D1 = 0x10325476;
static const u32 E1 = 0xC3D2E1F0;

//u32 A = 0x67452301;
//u32 B = 0xEFCDAB89;
//u32 C = 0x98BADCFE;
//u32 D = 0x10325476;
//u32 E = 0xC3D2E1F0;
///*全局变量，存放每组哈希后的值*/
u32 A;
u32 B;
u32 C;
u32 D;
u32 E;
/*常数*/
static const u32 K[4] = {0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
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
*function name:ExtendPlaintext
*function:扩展分组，生成80个子明文
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void ExtendPlaintext(u8 *data,u32 *w)
{
	u8 i;
	u32 temp;
	Byte2Word(data,w,64);//生成w[0]-w[15]
	for(i=16;i<80;i++)
	{
		temp = w[i-3]^w[i-8]^w[i-14]^w[i-16];
		w[i] = ROL(temp,1);//循环左移1位
	}
}
/*************************************************
*function name:Sha1Process
*function:SHA1加密过程
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void Sha1Process(u8 *data)
{
	u8 i,j;
	u32 w[80];//存放80个子明文
	u32 a,b,c,d,e;
	u32 temp;
	a = A; b = B; c = C; d = D; e = E;
	ExtendPlaintext(data,w);//扩展分组，生成80个子明文
	for(j = 0;j<80;j++)
	{
        switch(j/20)//选择逻辑函数
		{   
			case 0:   //0-19步执行这里
				temp = (b&c)|((~b)&d);
                temp += K[0];  //这个是常数
                break;
            case 1:  //20-39步执行这里
                temp = (b^c^d);
                temp += K[1];
                break;
            case 2: //40-59步执行这里
                temp = (b&c)|(b&d)|(c&d);
                temp += K[2];
                break;
            case 3: //60-79步执行这里
                temp = (b^c^d);
                temp += K[3];
                break;
        }
        temp = temp+ROL(a,5)+e+w[j]; //这里就是上面生成的80个分组了  给a赋的值一定是变化之前的！
		e = d;
		d = c;
		c = ROL(b,30); //循环左移30位
		b = a;
		a = temp;       
    }
	A +=a;
	B +=b;
	C +=c;
	D +=d;
	E +=e;
}
/*************************************************
*function name:SHA1
*function:通过秘钥将明文计算出密文信息摘要
*input:	plaintext-明文，len-明文长度(字节)
*output:ciphertext-密文信息摘要
**明文最大长度：2^64 bit,（本例程最大明文长度为2^32 Byte,即4GB）
**生成摘要长度：160 bit
**************************************************/
void SHA1(u8 *plaintext,u32 bytelen, u8 *ciphertext)
{
	u8 i;
	u8 data[64];		//存放一组64个字节数据
	u32 bitlen[2]={0};	//比特长度
	u32 w[16]={0};		//存放每组生成的16个字

	A = A1;B = B1;C = C1;D = D1;E = E1;//初始化常亮

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
		Sha1Process(data);//计算	
	}

	for(i=0;i<bytelen;i++)//剩余的明文字节
		data[i] = *plaintext++;
	if(bitlen[0] == 0xfffffe00) //比特长度大于0xffffffff,进位
	{
		bitlen[1]+=1;
		bitlen[0] = 0;
	}
	else 
		bitlen[0] +=bytelen*8;//比特长度加

	data[bytelen]=0x80;//直接补1字节（1000 0000）
	bytelen++;

	/*补完字节后，剩余字节分3种情况:=56；<56；>56*/
	if(bytelen == 56)//情况1，填充明文长度（bit）
	{	
		Word2Byte(bitlen[1],&data[56]);
		Word2Byte(bitlen[0],&data[60]);
		Sha1Process(data);//计算
	}
	else if(bytelen < 56)//情况2，填充字节
	{	
		for(i=bytelen;i<56;i++)
			data[i] = 0x00;
		Word2Byte(bitlen[1],&data[56]);//填充明文长度（bit）
		Word2Byte(bitlen[0],&data[60]);
		Sha1Process(data);//计算
	}
	else if(bytelen >56)//情况3，填充字节
	{
		for(i=bytelen;i<64;i++)
			data[i] = 0x00;
		Sha1Process(data);//计算
		for(i=0;i<56;i++)//填充字节
			data[i] = 0x00;
		Word2Byte(bitlen[1],&data[56]);//填充明文长度（bit）
		Word2Byte(bitlen[0],&data[60]);
		Sha1Process(data);//计算
	}
	/*加密完成，哈希值转换为字节*/
	Word2Byte(A,ciphertext);
	Word2Byte(B,ciphertext+4);
	Word2Byte(C,ciphertext+8);
	Word2Byte(D,ciphertext+12);
	Word2Byte(E,ciphertext+16);
}