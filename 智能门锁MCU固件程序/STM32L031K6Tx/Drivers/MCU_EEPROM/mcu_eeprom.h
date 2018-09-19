
#ifndef __MCU_EEPROM_H
#define __MCU_EEPROM_H
#include "stm32l0xx.h"

#define EEPROM_MAC_EXSIT_ADDR	0                         //MAC�Ѿ���¼��־0x01
#define EEPROM_MAC_ADDR			EEPROM_MAC_EXSIT_ADDR+1   //MAC���ݣ�8���ֽ�
#define EEPROM_AES_ADDR			EEPROM_MAC_ADDR+8		//AES����16���ֽ�	
#define EEPROM_DA_ADDR			EEPROM_AES_ADDR+16		//�豸�߼���ַ��1���ֽ�
#define	EEPROM_GA_ADDR			EEPROM_DA_ADDR+1		//�豸��ͥ���ַ��3���ֽ�


void EEPROM_ReadBytes(uint16_t Addr,uint8_t *Buffer,uint16_t Length);
void EEPROM_ReadWords(uint16_t Addr,uint16_t *Buffer,uint16_t Length);
void EEPROM_WriteBytes(uint16_t Addr,uint8_t *Buffer,uint16_t Length);
void EEPROM_WriteWords(uint16_t Addr,uint16_t *Buffer,uint16_t Length);
void MCU_EEPRomTest(void);

#endif

