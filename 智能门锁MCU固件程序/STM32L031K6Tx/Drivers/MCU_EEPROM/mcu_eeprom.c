//实现STM32L031K6内部EEPROM的读写

#include "mcu_eeprom.h"
#include "stm32l0xx.h"
#include "uart.h"

#define EEPROM_BASE_ADDR 0x08080000
#define EEPROM_BYTE_SIZE 0x0FFF

#define EN_INT __enable_irq()   //系统开全局中断
#define DIS_INT __disable_irq() //系统关全局中断
/*------------------------------------------------------------
 Func: EEPROM数据按字节读出
 Note:
-------------------------------------------------------------*/
void EEPROM_ReadBytes(uint16_t Addr, uint8_t *Buffer, uint16_t Length)
{
    uint8_t *wAddr;
    wAddr = (uint8_t *)(EEPROM_BASE_ADDR + Addr);
    while (Length--)
    {
        *Buffer++ = *wAddr++;
    }
}

/*------------------------------------------------------------
 Func: EEPROM数据读出
 Note:
-------------------------------------------------------------*/
void EEPROM_ReadWords(uint16_t Addr, uint16_t *Buffer, uint16_t Length)
{
    uint32_t *wAddr;
    wAddr = (uint32_t *)(EEPROM_BASE_ADDR + Addr);

    while (Length--)
    {
        *Buffer++ = *wAddr++;
    }
}

#define PEKEY1 0x89ABCDEF //FLASH_PEKEYR
#define PEKEY2 0x02030405 //FLASH_PEKEYR

/*------------------------------------------------------------
 Func: EEPROM数据按字节写入
 Note:
-------------------------------------------------------------*/
void EEPROM_WriteBytes(uint16_t Addr, uint8_t *Buffer, uint16_t Length)
{
    uint8_t *wAddr;
    wAddr = (uint8_t *)(EEPROM_BASE_ADDR + Addr);
    DIS_INT;
    FLASH->PEKEYR = PEKEY1; //unlock
    FLASH->PEKEYR = PEKEY2;
    while (FLASH->PECR & FLASH_PECR_PELOCK)
        ;

    while (Length--)
    {
        *wAddr++ = *Buffer++;
        while (FLASH->SR & FLASH_SR_BSY)
            ;
    }
    FLASH->PECR |= FLASH_PECR_PELOCK;
    EN_INT;
}

/*------------------------------------------------------------
 Func: EEPROM数据按字写入
 Note: 字当半字用
-------------------------------------------------------------*/
void EEPROM_WriteWords(uint16_t Addr, uint16_t *Buffer, uint16_t Length)
{
    uint32_t *wAddr;
    wAddr = (uint32_t *)(EEPROM_BASE_ADDR + Addr);
    DIS_INT;
    FLASH->PEKEYR = PEKEY1; //unlock
    FLASH->PEKEYR = PEKEY2;
    while (FLASH->PECR & FLASH_PECR_PELOCK)
        ;

    while (Length--)
    {
        *wAddr++ = *Buffer++;
        while (FLASH->SR & FLASH_SR_BSY)
            ;
    }
    FLASH->PECR |= FLASH_PECR_PELOCK;
    EN_INT;
}

/*------------------------------------------------------------
	内部EEPROM测试函数
-------------------------------------------------------------*/
void MCU_EEPRomTest(void)
{
    uint8_t i = 0;
    uint8_t eeprom_read_buff[16] = {0};
    //uint8_t eeprom_write_buff[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    //EEPROM_WriteBytes(8,eeprom_read_buff,16);
    EEPROM_ReadBytes(0, eeprom_read_buff, 16);

    for (i = 0; i < 16; i++)
    {
	
        DEBUG_Printf("%02x ", eeprom_read_buff[i]);
		
    }
    DEBUG_Printf("\r\n");
}
