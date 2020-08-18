#ifndef __AES_H__
#define __AES_H__

#define AES_128
//#define AES_192
//#define AES_256

#ifdef AES_128
	#define BLOCK_LEN		16			//分组长度（字节）
	#define KEY_LEN			16			//秘钥长度（字节）
	#define ENCRYPT_NUM		10			//加密轮数
#elif AES_192
	#define BLOCK_LEN		16			//分组长度（字节）
	#define KEY_LEN			24			//秘钥长度（字节）
	#define ENCRYPT_NUM		12			//加密轮数
#else
	#define BLOCK_LEN		16			//分组长度（字节）
	#define KEY_LEN			32			//秘钥长度（字节）
	#define ENCRYPT_NUM		14			//加密轮数
#endif	

void AES(u8 *plaintext, u8 *key, u8 *ciphertext);
void DeAES(u8 *ciphertext, u8 *key, u8 *plaintext);	
	
#endif
