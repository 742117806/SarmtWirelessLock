#ifndef RSA_H_
#define RSA_H_

#include  "stm32l0xx.h"
#include <string.h>


#define RsaByte_Size          16
#define RsaByte_SizeMax       (RsaByte_Size + 1) 



/************************* �ⲿ���� ***************************/
void LargeNumber_Power_Mod(uint8_t model[RsaByte_Size], uint8_t radix[RsaByte_Size], uint8_t index[RsaByte_Size], uint8_t mul_Product[RsaByte_SizeMax], uint8_t divisor[RsaByte_SizeMax]);


//ͬλ������
void LargeNumber_SameMod(uint8_t *p_sourse, uint8_t *p_subtrahend, uint8_t bit_size);
//��������
void LargeNumber_Modulo(uint8_t dividend[RsaByte_SizeMax], uint8_t divisor[RsaByte_SizeMax]);
//��λ����һλ��
void LargeNumber_Mul(uint8_t mul_factor1[RsaByte_Size], uint8_t mul_factor2, uint8_t mul_Product[RsaByte_SizeMax], uint8_t flag);
//��ģ����
void LargeNumber_Mul_Mod(uint8_t mul_factor1[RsaByte_Size], uint8_t mul_factor2[RsaByte_Size], uint8_t mul_Product[RsaByte_SizeMax], uint8_t divisor[RsaByte_SizeMax]);






#endif

