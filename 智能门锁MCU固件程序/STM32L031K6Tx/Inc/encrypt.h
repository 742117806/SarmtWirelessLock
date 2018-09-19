
#ifndef __ENCRYPT_H
#define __ENCRYPT_H
#include "stm32l0xx.h"



#define Encrypt_En            1           //0 不加密 1 内部加密 2 外部加密(加密芯片) -->带哪种加密功能

#if (Encrypt_En)
  #include "aes.h"
#endif



#define RoutPath_Size                  3               //路由的最大路径数
#define RoutSeries_Size                3               //路由的最大级数


void Rsa_Decode(uint8_t *p_minw);  
void EncryptTest(void); 
uint8_t Encrypt_Convert(uint8_t *p_source, uint8_t input_len, uint8_t *output_len, uint8_t Convert_Mode) ;

#endif

