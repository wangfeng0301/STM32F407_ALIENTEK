#include <stdio.h>
#include "type.h"
#include "SHA1.h"

/*���ӱ���*/
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
///*ȫ�ֱ��������ÿ���ϣ���ֵ*/
u32 A;
u32 B;
u32 C;
u32 D;
u32 E;
/*����*/
static const u32 K[4] = {0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
/*************************************************
*function name:Byte2Word
*function:�ֽ���ϳ���
*input:byte-�ֽ�   len-�����ֽڳ���,���ȱ���Ϊ4�ı���
*output:word-��
**�˺���Ϊ���ģʽ�����ֽڴ���ڵ͵�ַ
**************************************************/
static void Byte2Word(u8 *byte,u32 *word,u8 len)
{
	u8 i;
	for(i=0;i<len;i+=4)
		*(word+i/4) = *(byte+i)<<24 | *(byte+i+1)<<16 | *(byte+i+2)<<8 | *(byte+i+3);
}
/*************************************************
*function name:Word2Byte
*function:�ֲ���ֽ�
*input:word-�� 
*output:byte-�ֽ� 
**�˺���Ϊ���ģʽ�����ֽڴ���ڵ͵�ַ
**************************************************/
static void Word2Byte(u32 word, u8 *byte)
{
	u8 i;
	for(i=0;i<4;i++)
		byte[i] = (u8)(word>>(8*(3-i)))&0xff;
}
/*************************************************
*function name:ROL
*function:ѭ������
*input:data-32λ����,len-����λ��
*output:32λ��
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
*function:��չ���飬����80��������
*input:	data-��������(�ֽ�)
*output:w-������
**************************************************/
static void ExtendPlaintext(u8 *data,u32 *w)
{
	u8 i;
	u32 temp;
	Byte2Word(data,w,64);//����w[0]-w[15]
	for(i=16;i<80;i++)
	{
		temp = w[i-3]^w[i-8]^w[i-14]^w[i-16];
		w[i] = ROL(temp,1);//ѭ������1λ
	}
}
/*************************************************
*function name:Sha1Process
*function:SHA1���ܹ���
*input:	data-��������(�ֽ�)
*output:w-������
**************************************************/
static void Sha1Process(u8 *data)
{
	u8 i,j;
	u32 w[80];//���80��������
	u32 a,b,c,d,e;
	u32 temp;
	a = A; b = B; c = C; d = D; e = E;
	ExtendPlaintext(data,w);//��չ���飬����80��������
	for(j = 0;j<80;j++)
	{
        switch(j/20)//ѡ���߼�����
		{   
			case 0:   //0-19��ִ������
				temp = (b&c)|((~b)&d);
                temp += K[0];  //����ǳ���
                break;
            case 1:  //20-39��ִ������
                temp = (b^c^d);
                temp += K[1];
                break;
            case 2: //40-59��ִ������
                temp = (b&c)|(b&d)|(c&d);
                temp += K[2];
                break;
            case 3: //60-79��ִ������
                temp = (b^c^d);
                temp += K[3];
                break;
        }
        temp = temp+ROL(a,5)+e+w[j]; //��������������ɵ�80��������  ��a����ֵһ���Ǳ仯֮ǰ�ģ�
		e = d;
		d = c;
		c = ROL(b,30); //ѭ������30λ
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
*function:ͨ����Կ�����ļ����������ϢժҪ
*input:	plaintext-���ģ�len-���ĳ���(�ֽ�)
*output:ciphertext-������ϢժҪ
**������󳤶ȣ�2^64 bit,��������������ĳ���Ϊ2^32 Byte,��4GB��
**����ժҪ���ȣ�160 bit
**************************************************/
void SHA1(u8 *plaintext,u32 bytelen, u8 *ciphertext)
{
	u8 i;
	u8 data[64];		//���һ��64���ֽ�����
	u32 bitlen[2]={0};	//���س���
	u32 w[16]={0};		//���ÿ�����ɵ�16����

	A = A1;B = B1;C = C1;D = D1;E = E1;//��ʼ������

	while(bytelen>=64)		//���ĳ��ȴ��ڷ��鳤��
	{
		for(i=0;i<64;i++)
			data[i] = *plaintext++;
		bytelen -=64;	//�ֽڳ��ȼ�64
		if(bitlen[0] == 0xfffffe00) //���س��ȴ���0xffffffff,��λ
		{
			bitlen[1]+=1;
			bitlen[0] = 0;
		}
		else 
			bitlen[0] +=512;//���س��ȼ�512
		Sha1Process(data);//����	
	}

	for(i=0;i<bytelen;i++)//ʣ��������ֽ�
		data[i] = *plaintext++;
	if(bitlen[0] == 0xfffffe00) //���س��ȴ���0xffffffff,��λ
	{
		bitlen[1]+=1;
		bitlen[0] = 0;
	}
	else 
		bitlen[0] +=bytelen*8;//���س��ȼ�

	data[bytelen]=0x80;//ֱ�Ӳ�1�ֽڣ�1000 0000��
	bytelen++;

	/*�����ֽں�ʣ���ֽڷ�3�����:=56��<56��>56*/
	if(bytelen == 56)//���1��������ĳ��ȣ�bit��
	{	
		Word2Byte(bitlen[1],&data[56]);
		Word2Byte(bitlen[0],&data[60]);
		Sha1Process(data);//����
	}
	else if(bytelen < 56)//���2������ֽ�
	{	
		for(i=bytelen;i<56;i++)
			data[i] = 0x00;
		Word2Byte(bitlen[1],&data[56]);//������ĳ��ȣ�bit��
		Word2Byte(bitlen[0],&data[60]);
		Sha1Process(data);//����
	}
	else if(bytelen >56)//���3������ֽ�
	{
		for(i=bytelen;i<64;i++)
			data[i] = 0x00;
		Sha1Process(data);//����
		for(i=0;i<56;i++)//����ֽ�
			data[i] = 0x00;
		Word2Byte(bitlen[1],&data[56]);//������ĳ��ȣ�bit��
		Word2Byte(bitlen[0],&data[60]);
		Sha1Process(data);//����
	}
	/*������ɣ���ϣֵת��Ϊ�ֽ�*/
	Word2Byte(A,ciphertext);
	Word2Byte(B,ciphertext+4);
	Word2Byte(C,ciphertext+8);
	Word2Byte(D,ciphertext+12);
	Word2Byte(E,ciphertext+16);
}