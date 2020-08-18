#ifndef __DES_H__
#define __DES_H__

#include "type.h"

#define DES_ENCRYPT_NUM		16			//¼ÓÃÜÂÖÊı
//void IPExchange(u8 *plaintext,u32 *L0,u32 *R0);
//void IPExchange(u8 *plaintext);
//void KeyExchange(u8 *key);

void DES(u8 *plaintext,u8 *key, u8 *ciphertext);
#endif


