#ifndef _CRC16_H
#define _CRC16_H
#include "stm32l0xx.h"

uint16_t CRC16_1(uint8_t* pchMsg, uint16_t wDataLen);
uint16_t CRC16_2(uint8_t* pchMsg, uint16_t wDataLen);
uint16_t CRC16_3(uint8_t* pchMsg, uint16_t wDataLen);
uint16_t CRC16_check(uint16_t *Data,uint16_t Data_length);

#endif


