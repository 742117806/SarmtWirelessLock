
#ifndef __ENCRYPT_H
#define __ENCRYPT_H
#include "stm32l0xx.h"



#define Encrypt_En            1           //0 ������ 1 �ڲ����� 2 �ⲿ����(����оƬ) -->�����ּ��ܹ���

#if (Encrypt_En)
  #include "aes.h"
#endif



#define RoutPath_Size                  3               //·�ɵ����·����
#define RoutSeries_Size                3               //·�ɵ������


void Rsa_Decode(uint8_t *p_minw);  
void EncryptTest(void); 
uint8_t Encrypt_Convert(uint8_t *p_source, uint8_t input_len, uint8_t *output_len, uint8_t Convert_Mode) ;

#endif

