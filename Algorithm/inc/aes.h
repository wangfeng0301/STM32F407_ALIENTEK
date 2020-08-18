#ifndef __AES_H__
#define __AES_H__

#define AES_128
//#define AES_192
//#define AES_256

#ifdef AES_128
	#define BLOCK_LEN		16			//���鳤�ȣ��ֽڣ�
	#define KEY_LEN			16			//��Կ���ȣ��ֽڣ�
	#define ENCRYPT_NUM		10			//��������
#elif AES_192
	#define BLOCK_LEN		16			//���鳤�ȣ��ֽڣ�
	#define KEY_LEN			24			//��Կ���ȣ��ֽڣ�
	#define ENCRYPT_NUM		12			//��������
#else
	#define BLOCK_LEN		16			//���鳤�ȣ��ֽڣ�
	#define KEY_LEN			32			//��Կ���ȣ��ֽڣ�
	#define ENCRYPT_NUM		14			//��������
#endif	

void AES(u8 *plaintext, u8 *key, u8 *ciphertext);
void DeAES(u8 *ciphertext, u8 *key, u8 *plaintext);	
	
#endif
