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


#define AesBuf_Size             (RsaByte_Size*3+4+2)   //20170925修改(私钥+模值+密文+群组地址3(但要填4)+标志)

/************************* 外部变量 ***************************/

extern uint8_t   aes_w[Nb*(Nr+1)*4]; // expanded key
//extern uint8_t *  aes_w; 
extern uint8_t   aes_in[16];
extern uint8_t   aes_out[AesBuf_Size];   

/************************* 外部函数 ***************************/
extern void key_expansion(uint8_t *key, uint8_t *w) ;
extern void cipher(uint8_t *in, uint8_t *out, uint8_t *w);
extern void inv_cipher(uint8_t *in, uint8_t *out, uint8_t *w) ;


#endif

