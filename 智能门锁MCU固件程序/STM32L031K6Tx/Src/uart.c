#include "uart.h"
#include "stm32l0xx.h"
#include <string.h>
#include "protocol.h"

//UartFrame_TX_TypDef DownCom_TxBuf;
//uint8_t Secret_GKey_Flag;  //密钥公钥存在标志
//uint8_t Secret_KeyOk_Flag; //密钥OK标志
//UartFrame_TX_TypDef UpCom_TxBuf;

extern UART_HandleTypeDef hlpuart1;	



uint8_t LP_UartRec;         //串口1中断接收一个字符
uint8_t uart2_rec;			//串口2中断接收一个字符


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
int fputc(int ch, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART2 and Loop until the end of transmission */

    DEBUT_UART->TDR = ch;
    while ((DEBUT_UART->ISR & 0X40) == 0);
    return ch;
}


void UartSendData(USART_TypeDef *UARTx, uint8_t byte)
{
    UARTx->TDR = byte;

    while ((UARTx->ISR & 0X40) == 0)
        ;
}
void UartSendBytes(USART_TypeDef *UARTx, uint8_t *buf, uint16_t len)
{
    uint8_t i = 0;
    for (i = 0; i < len; i++)
    {
        UartSendData(UARTx,*buf++);
    }
}

