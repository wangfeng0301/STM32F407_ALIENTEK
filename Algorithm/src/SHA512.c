/********此函数错误，用SHA512MY函数****************/
#include <stdio.h>
#include "type.h"
#include "SHA512.h"

/*初始哈希值,将64bit拆分成两个32bit字，每行拼起来对应64bit*/
static const u32 SHA512_H[8][2] = { 0x6a09e667,0xf3bcc908, 
									0xbb67ae85,0x84caa73b, 
									0x3c6ef372,0xfe94f82b, 
									0xa54ff53a,0x5f1d36f1, 
									0x510e527f,0xade682d1, 
									0x9b05688c,0x2b3e6c1f, 
									0x1f83d9ab,0xfb41bd6b, 
									0x5be0cd19,0x137e2179};
/*全局变量，存放每组哈希后的值*/
u32 SHA512_H1[8][2];
/*初始化常数*/
static const u32 SHA512_K[80][2] = 
	{0x428a2f98,0xd728ae22,  0x71374491,0x23ef65cd,  0xb5c0fbcf,0xec4d3b2f,  0xe9b5dba5,0x8189dbbc,  0x3956c25b,0xf348b538, 
     0x59f111f1,0xb605d019,  0x923f82a4,0xaf194f9b,  0xab1c5ed5,0xda6d8118,  0xd807aa98,0xa3030242,  0x12835b01,0x45706fbe, 
     0x243185be,0x4ee4b28c,  0x550c7dc3,0xd5ffb4e2,  0x72be5d74,0xf27b896f,  0x80deb1fe,0x3b1696b1,  0x9bdc06a7,0x25c71235, 
     0xc19bf174,0xcf692694,  0xe49b69c1,0x9ef14ad2,  0xefbe4786,0x384f25e3,  0x0fc19dc6,0x8b8cd5b5,  0x240ca1cc,0x77ac9c65, 
     0x2de92c6f,0x592b0275,  0x4a7484aa,0x6ea6e483,  0x5cb0a9dc,0xbd41fbd4,  0x76f988da,0x831153b5,  0x983e5152,0xee66dfab, 
     0xa831c66d,0x2db43210,  0xb00327c8,0x98fb213f,  0xbf597fc7,0xbeef0ee4,  0xc6e00bf3,0x3da88fc2,  0xd5a79147,0x930aa725, 
     0x06ca6351,0xe003826f,  0x14292967,0x0a0e6e70,  0x27b70a85,0x46d22ffc,  0x2e1b2138,0x5c26c926,  0x4d2c6dfc,0x5ac42aed, 
     0x53380d13,0x9d95b3df,  0x650a7354,0x8baf63de,  0x766a0abb,0x3c77b2a8,  0x81c2c92e,0x47edaee6,  0x92722c85,0x1482353b, 
     0xa2bfe8a1,0x4cf10364,  0xa81a664b,0xbc423001,  0xc24b8b70,0xd0f89791,  0xc76c51a3,0x0654be30,  0xd192e819,0xd6ef5218, 
     0xd6990624,0x5565a910,  0xf40e3585,0x5771202a,  0x106aa070,0x32bbd1b8,  0x19a4c116,0xb8d2d0c8,  0x1e376c08,0x5141ab53, 
     0x2748774c,0xdf8eeb99,  0x34b0bcb5,0xe19b48a8,  0x391c0cb3,0xc5c95a63,  0x4ed8aa4a,0xe3418acb,  0x5b9cca4f,0x7763e373, 
     0x682e6ff3,0xd6b2b8a3,  0x748f82ee,0x5defb2fc,  0x78a5636f,0x43172f60,  0x84c87814,0xa1f0ab72,  0x8cc70208,0x1a6439ec, 
     0x90befffa,0x23631e28,  0xa4506ceb,0xde82bde9,  0xbef9a3f7,0xb2c67915,  0xc67178f2,0xe372532b,  0xca273ece,0xea26619c, 
     0xd186b8c7,0x21c0c207,  0xeada7dd6,0xcde0eb1e,  0xf57d4f7f,0xee6ed178,  0x06f067aa,0x72176fba,  0x0a637dc5,0xa2c898a6, 
     0x113f9804,0xbef90dae,  0x1b710b35,0x131c471b,  0x28db77f5,0x23047d84,  0x32caab7b,0x40c72493,  0x3c9ebe0a,0x15c9bebc, 
     0x431d67c4,0x9c100d4c,  0x4cc5d4be,0xcb3e42b6,  0x597f299c,0xfc657e2a,  0x5fcb6fab,0x3ad6faec,  0x6c44198c,0x4a475817};
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
static u32 ROL32(u32 data,u8 len)
{
	u32 l,r;
	l = data<<len;
	r = data>>(32-len);
	return (l|r);
}
/*************************************************
*function name:ROR
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
*function name:ROL64
*function:64位循环左移
*input:data1,data2-32位的字,len-左移位数
*output:temp1,temp2-32位字
**************************************************/
static void ROL64(u32 data1,u32 data2,u8 len,u32 *temp1,u32 *temp2)
{
	u32 l1,r1,l2,r2;
	if(len<=32)
	{
		l1 = data1<<len;	 //新数据第1字节左部分
		r2 = data1>>(32-len);//新数据第2字节右部分
		l2 = data2<<len;	 //新数据第2个字左部分
		r1 = data2>>(32-len);//新数据第1字节右部分
	}
	if(len>32 && len<=64)
	{
		r2 = data2>>(len-32);//新数据第2个字右部分
		l2 = data1<<(64-len);//新数据第2字节左部分
		r1 = data1>>(64-len);//新数据第1字节右部分
		l1 = data2<<(len-32);//新数据第1字节左部分
	}
	*temp1 = l1 | r1;
	*temp2 = l2 | r2;
}
/*************************************************
*function name:ROR64
*function:64位循环右移
*input:data1，data2-32位的字,len-右移位数
*output:temp1,temp2-32位字
**************************************************/
static void ROR64(u32 data1,u32 data2,u8 len,u32 *temp1,u32 *temp2)
{
	u32 l1,r1,l2,r2;
	if(len<=32)
	{
		r1 = data1>>len;	 //新数据第1字节右部分
		l2 = data1<<(32-len);//新数据第2字节左部分
		r2 = data2>>len;	 //新数据第2个字右部分
		l1 = data2<<(32-len);//新数据第1字节左部分		
	}
	if(len>32 && len<=64)
	{
		r2 = data1>>(len-32);//新数据第2个字右部分
		l2 = data2<<(64-len);//新数据第2字节左部分
		r1 = data2>>(len-32);//新数据第1字节右部分
		l1 = data1<<(64-len);//新数据第1字节左部分
	}
	*temp1 = l1 | r1;
	*temp2 = l2 | r2;	
}
/*************************************************
*function name:leftshift64
*function:64位左移
*input:data1,data2-32位的字,len-左移位数
*output:temp1,temp2-32位字
**************************************************/
static void leftshift64(u32 data1,u32 data2,u8 len,u32 *temp1,u32 *temp2)
{
	u32 l1,r1,l2,r2;
	if(len<=32)
	{
		l1 = data1<<len;	 //新数据第1字节左部分
		r1 = data2>>(32-len);//新数据第1字节右部分
		r2 = 0x00;			 //新数据第2字节右部分
		l2 = data2<<len;	 //新数据第2个字左部分		
	}
	if(len>32 && len<=64)
	{
		r2 = 0x00;			 //新数据第2个字右部分
		l2 = data1<<(64-len);//新数据第2字节左部分
		r1 = data1>>(64-len);//新数据第1字节右部分
		l1 = data2<<(len-32);//新数据第1字节左部分
	}
	*temp1 = l1 | r1;
	*temp2 = l2 | r2;
}
/*************************************************
*function name:rightshift64
*function:64位右移
*input:data1，data2-32位的字,len-右移位数
*output:temp1,temp2-32位字
**************************************************/
static void rightshift64(u32 data1,u32 data2,u8 len,u32 *temp1,u32 *temp2)
{
	u32 l1,r1,l2,r2;
	if(len<=32)
	{
		r1 = data1>>len;	 //新数据第1字节右部分
		l1 = 0x00;			 //新数据第1字节左部分
		l2 = data1<<(32-len);//新数据第2字节左部分
		r2 = data2>>len;	 //新数据第2个字右部分
				
	}
	if(len>32 && len<=64)
	{
		r2 = data1>>(len-32);//新数据第2个字右部分
		l2 = data2<<(64-len);//新数据第2字节左部分
		r1 = data2>>(len-32);//新数据第1字节右部分
		l1 = 0x00;			 //新数据第1字节左部分
	}
	*temp1 = l1 | r1;
	*temp2 = l2 | r2;	
}

/*************************************************
*function name:add64
*function:64位相加
*input:data1-32位字，data2-32位位字,
*output:temp1,temp2-32位字
**************************************************/
static void add64(u32 *data1,u32 *data2,u32 temp[2])
{
	u32 sum;
	u32 *a = data1,*b = data2;
	//a[0] = data1[0]; a[1] = data1[1];
	//b[0] = data2[0]; b[1] = data2[1];

	sum = (a[0]&0xffff)+(b[0]&0xffff);//低16位相加
	sum = (sum>>16)+((a[0]>>16)&0xffff)+((b[0]>>16)&0xffff);//高16位相加+进位
	
	temp[0] = a[0]+b[0];
	temp[1] = (sum>>16)+a[1]+b[1];//进位+高位字
}
/*************************************************
*function name:SHA512_ExtendPlaintext
*function:扩展分组，生成80个子明文
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void SHA512_ExtendPlaintext(u8 *data,u32 *w)
{
	u8 i;
	u32 s0[2]={0};
	u32 s1[2]={0};
	u32 temp1,temp2;//临时变量存放64位循环移动后的值
	Byte2Word(data,w,128);//生成w[0]-w[32]，每两个为1组组成64bit
	for(i=16;i<80;i++)//80轮
	{
		s0[0]=0;s0[1]=0;//清零
		s1[0]=0;s1[1]=0;

		ROR64(w[2*i-15*2],w[2*i+1-(15*2+1)],1,&temp1,&temp2);
		s0[0] ^= temp1; 
		s0[1] ^= temp2;
		ROR64(w[2*i-15*2],w[2*i+1-(15*2+1)],8,&temp1,&temp2);
		s0[0] ^= temp1; 
		s0[1] ^= temp2;
		rightshift64(w[2*i-15*2],w[2*i+1-(15*2+1)],7,&temp1,&temp2);
		s0[0] ^= temp1; 
		s0[1] ^= temp2;

		ROR64(w[2*i-2*2],w[2*i+1-(2*2+1)],19,&temp1,&temp2);
		s1[0] ^= temp1; 
		s1[1] ^= temp2;
		ROR64(w[2*i-2*2],w[2*i+1-(2*2+1)],61,&temp1,&temp2);
		s1[0] ^= temp1; 
		s1[1] ^= temp2;
		rightshift64(w[2*i-2*2],w[2*i+1-(2*2+1)],6,&temp1,&temp2);
		s1[0] ^= temp1; 
		s1[1] ^= temp2;

		w[i*2] = w[i*2-16*2]+s0[0]+w[i*2-7*2]+s1[0];//
		w[i*2+1] = w[i*2+1-(16*2+1)]+s0[1]+w[i*2+1-(7*2+1)]+s1[1];//
	}
}
/*************************************************
*function name:SHA512_Process
*function:SHA512加密过程
*input:	data-分组明文(字节)
*output:w-子明文
**************************************************/
static void SHA512_Process(u8 *data)
{
	u8 i,j;
	u32 w[160];//存放80个子明文,每个子明文64bit,共160字节
	u32 S1[2],ch[2],temp1[2],S0[2],maj[2],temp2[2];//中间变量
	u32 a[2],b[2],c[2],d[2],e[2],f[2],g[2],h[2];
	u32 temp3,temp4;

	SHA512_ExtendPlaintext(data,w);//扩展分组，生成80个子明文,共160*32bit

	a[0]=SHA512_H1[0][0]; b[0]=SHA512_H1[1][0]; c[0]=SHA512_H1[2][0]; d[0]=SHA512_H1[3][0]; e[0]=SHA512_H1[4][0]; f[0]=SHA512_H1[5][0]; g[0]=SHA512_H1[6][0]; h[0]=SHA512_H1[7][0];//上一轮哈希值
	a[1]=SHA512_H1[0][1]; b[1]=SHA512_H1[1][1]; c[1]=SHA512_H1[2][1]; d[1]=SHA512_H1[3][1]; e[1]=SHA512_H1[4][1]; f[1]=SHA512_H1[5][1]; g[1]=SHA512_H1[6][1]; h[1]=SHA512_H1[7][1];//上一轮哈希值

	for(i = 0;i<80;i++)//80轮加密
	{   
		for(j=0;j<2;j++)//清零
		{
			S1[j]=0;ch[j]=0;temp1[j]=0;S0[j]=0;maj[j]=0;temp2[j]=0;
		}
        
		ROR64(a[0],a[1],14,&temp3,&temp4);
		S1[0]^=temp3;
		S1[1]^=temp4;
		ROR64(a[0],a[1],18,&temp3,&temp4);
		S1[0]^=temp3;
		S1[1]^=temp4;
		ROR64(a[0],a[1],41,&temp3,&temp4);
		S1[0]^=temp3;
		S1[1]^=temp4;

		ch[0] = (e[0]&f[0])^((~e[0])&g[0]);
		ch[1] = (e[1]&f[1])^((~e[1])&g[1]);

		add64(h,S1,temp1);
		add64(temp1,ch,temp1);
		add64(temp1,(u32*)&SHA512_K[2*i][0],temp1);
		add64(temp1,&w[i*2],temp1);
		//S1 = ROR(e,6)^ROR(e,11)^ROR(e,25);
		//ch = (e&f)^((~e)&g);
		//temp1 = h+S1+ch+SHA512_K[i]+w[i];

		ROR64(a[0],a[1],28,&temp3,&temp4);
		S0[0]^=temp3;
		S0[1]^=temp4;
		ROR64(a[0],a[1],34,&temp3,&temp4);
		S0[0]^=temp3;
		S0[1]^=temp4;
		ROR64(a[0],a[1],39,&temp3,&temp4);
		S0[0]^=temp3;
		S0[1]^=temp4;

		maj[0] = (a[0]&b[0])^(a[0]&c[0])^(b[0]&c[0]);
		maj[1] = (a[1]&b[1])^(a[1]&c[1])^(b[1]&c[1]);

		add64(S0,maj,temp2);
		//S0 = ROR(a,2)^ROR(a,13)^ROR(a,22);
		//maj = (a&b)^(a&c)^(b&c);
		//temp2 = S0+maj;

		h[0] = g[0];h[1] = g[1];
		g[0] = f[0];g[1] = f[1];
		f[0] = e[0];f[1] = e[1];
		add64(d,temp1,e);//e[0] = d[0] + temp1[0];		
		d[0] = c[0];d[1] = c[1];
		c[0] = b[0];c[1] = b[1];
		b[0] = a[0];b[1] = a[1];
		add64(temp1,temp1,a);//a = temp1+temp2;
    }
	add64(&SHA512_H1[0][0], a, &SHA512_H1[0][0]);//SHA512_H1[0] +=a;
	add64(&SHA512_H1[1][0], b, &SHA512_H1[1][0]);//SHA512_H1[1] +=b;
	add64(&SHA512_H1[2][0], c, &SHA512_H1[2][0]);//SHA512_H1[2] +=c;
	add64(&SHA512_H1[3][0], d, &SHA512_H1[3][0]);//SHA512_H1[3] +=d;
	add64(&SHA512_H1[4][0], e, &SHA512_H1[4][0]);//SHA512_H1[4] +=e;
	add64(&SHA512_H1[5][0], f, &SHA512_H1[5][0]);//SHA512_H1[5] +=f;
	add64(&SHA512_H1[6][0], g, &SHA512_H1[6][0]);//SHA512_H1[6] +=g;
	add64(&SHA512_H1[7][0], h, &SHA512_H1[7][0]);//SHA512_H1[7] +=h;
}
/*************************************************
*function name:SHA512
*function:通过秘钥将明文计算出密文信息摘要
*input:	plaintext-明文，len-明文长度(字节)
*output:ciphertext-密文信息摘要
**明文最大长度：2^128 bit,（本例程最大明文长度为2^32 Byte,即4GB）
**生成摘要长度：512 bit
**************************************************/
void SHA512(u8 *plaintext,u32 bytelen, u8 *ciphertext)
{
	u8 i;
	u8 data[128];		//存放一组128个字节数据
	u32 bitlen[4]={0};	//比特长度

	for(i=0;i<8;i++)	//初始哈希值赋值
	{
		SHA512_H1[i][0] = SHA512_H[i][0];
		SHA512_H1[i][1] = SHA512_H[i][1];
	}

	while(bytelen>=128)		//明文长度大于分组长度,每组128字节
	{
		for(i=0;i<128;i++)
			data[i] = *plaintext++;
		bytelen -=128;	//字节长度减128
		if(bitlen[0] == 0xfffffc00) //比特长度大于0xffffffff,进位
		{
			bitlen[0] = 0;
			if(bitlen[1]==0xffffffff)//比特长度大于0xffffffff,进位
			{
				bitlen[1] = 0;
				if(bitlen[2]==0xffffffff)//比特长度大于0xffffffff,进位
				{
					bitlen[2] = 0;
					if(bitlen[3]==0xffffffff)
					{
						printf("文件长度超限！\r\n");
						printf("文件长度最长为 2^128 bit！\r\n");
						return;
					}
					else
						bitlen[3]++;
				}
				else
					bitlen[2]++;
			}
			else 
				bitlen[1] ++;
		}
		else 
			bitlen[0] +=1024;//比特长度加1024
		SHA512_Process(data);//计算	
	}

	for(i=0;i<bytelen;i++)//剩余的明文字节
	{
		data[i] = *plaintext++;
		if(bitlen[0] == 0xfffffff8) //比特长度大于0xffffffff,进位
		{
			bitlen[0] = 0;
			if(bitlen[1]==0xffffffff)//比特长度大于0xffffffff,进位
			{
				bitlen[1] = 0;
				if(bitlen[2]==0xffffffff)//比特长度大于0xffffffff,进位
				{
					bitlen[2] = 0;
					if(bitlen[3]==0xffffffff)
					{
						printf("文件长度超限！\r\n");
						printf("文件长度最长为 2^128 bit！\r\n");
						return;
					}
					else
						bitlen[3]++;
				}
				else
					bitlen[2]++;
			}
			else 
				bitlen[1] ++;
		}
		else 
			bitlen[0] +=8;//比特长度加
	}

	data[bytelen]=0x80;//直接补1字节（1000 0000）
	bytelen++;

	/*补完字节后，剩余字节分3种情况:=112；<112；>112*/
	if(bytelen == 112)//情况1，填充明文长度（bit）
	{	
		Word2Byte(bitlen[3],&data[112]);//填充明文长度
		Word2Byte(bitlen[2],&data[116]);
		Word2Byte(bitlen[1],&data[120]);
		Word2Byte(bitlen[0],&data[124]);
		SHA512_Process(data);//计算
	}
	else if(bytelen < 112)//情况2，填充字节
	{	
		for(i=bytelen;i<112;i++)
			data[i] = 0x00;
		Word2Byte(bitlen[3],&data[112]);//填充明文长度
		Word2Byte(bitlen[2],&data[116]);
		Word2Byte(bitlen[1],&data[120]);
		Word2Byte(bitlen[0],&data[124]);
		SHA512_Process(data);//计算
	}
	else if(bytelen >112)//情况3，填充字节
	{
		for(i=bytelen;i<128;i++)
			data[i] = 0x00;
		SHA512_Process(data);//计算
		for(i=0;i<112;i++)//填充字节
			data[i] = 0x00;
		Word2Byte(bitlen[3],&data[112]);//填充明文长度
		Word2Byte(bitlen[2],&data[116]);
		Word2Byte(bitlen[1],&data[120]);
		Word2Byte(bitlen[0],&data[124]);
		SHA512_Process(data);//计算
	}
	/*加密完成，哈希值转换为字节*/
	for(i=0;i<8;i++)
	{
		Word2Byte(SHA512_H1[0][i],ciphertext+8*i);
		Word2Byte(SHA512_H1[1][i],ciphertext+8*i+4);
	}

}

