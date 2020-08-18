#ifndef __SM4_H__
#define __SM4_H__
#include "type.h"

void SM4(u8 *plaintext,u8 *key, u8 *ciphertext);
void DeSM4(u8 *ciphertext,u8 *key, u8 *plaintext);

#endif

