#ifndef __AES_H__
#define __AES_H__
#include "stdint.h"  
void AES(unsigned char* key);

void KeyExpansion(unsigned char* key, unsigned char w[][4][4]);
unsigned char FFmul(unsigned char a, unsigned char b);
void AddRoundKey(unsigned char state[][4], unsigned char k[][4]);
void InvSubBytes(unsigned char state[][4]);

unsigned char* InvCipher(unsigned char* input);
void* InvCipherfor(void* input, int length);//Êý×é½âÃÜ
void InvShiftRows(unsigned char state[][4]);
void InvMixColumns(unsigned char state[][4]);

unsigned char* Cipher(unsigned char* input);
void* Cipherfor(void* input, int length);
void SubBytes(unsigned char state[][4]);
void ShiftRows(unsigned char state[][4]);
void MixColumns(unsigned char state[][4]);
#endif
