#ifndef __SPI_H
#define __SPI_H


#include "stm32l0xx.h"


/*****************************************************************/


//#define SPI_SSN_H()	(HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET))
//#define SPI_SSN_L()	(HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET))

#define WIRELESS_NSEL_H() 	     	(HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET))
#define WIRELESS_NSEL_L()	        (HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET))

#define SDN_H()		(HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET))
#define SDN_L()		(HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET))

uint8_t SPI_RWbyte(uint8_t sdata);

#endif


