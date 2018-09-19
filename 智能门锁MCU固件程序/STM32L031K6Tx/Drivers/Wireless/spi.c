#include "spi.h"
#include "stm32l0xx.h"
//#include "gpio.h"
extern SPI_HandleTypeDef hspi1;

uint8_t spi_rec_data;
uint8_t SPI_RWbyte(uint8_t sdata)
{
    HAL_SPI_TransmitReceive(&hspi1, &sdata, &spi_rec_data, 1, 1000);
    return spi_rec_data;
}
