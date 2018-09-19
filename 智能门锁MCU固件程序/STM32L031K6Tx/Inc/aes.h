#ifndef _AES_H
#define _AES_H

//#include "SH79F161B.h"
#include "rsa.h"

#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>


#define Nb                      4

#define Nk                      4 
#define Nr                      10


#define AesBuf_Size             (RsaByte_Size*3+4+2)   //20170925�޸�(˽Կ+ģֵ+����+Ⱥ���ַ3(��Ҫ��4)+��־)

/************************* �ⲿ���� ***************************/

extern uint8_t   aes_w[Nb*(Nr+1)*4]; // expanded key
//extern uint8_t *  aes_w; 
extern uint8_t   aes_in[16];
extern uint8_t   aes_out[AesBuf_Size];   

/************************* �ⲿ���� ***************************/
extern void key_expansion(uint8_t *key, uint8_t *w) ;
extern void cipher(uint8_t *in, uint8_t *out, uint8_t *w);
extern void inv_cipher(uint8_t *in, uint8_t *out, uint8_t *w) ;


#endif

