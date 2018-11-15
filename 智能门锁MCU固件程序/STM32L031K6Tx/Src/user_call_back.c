#include "user_call_back.h"
#include "uart.h"
#include "device.h"
#include "wireless_app.h"

extern uint16_t sleep_delay_cnt;
extern uint16_t up_power_delay_cnt;
/**
  * @brief  SYSTICK callback.
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
    //	WirelessRx_Timeout_Cnt ++;			//无线接收到同步字后接收后面数据超时计数
    if(sleep_delay_cnt > 0) sleep_delay_cnt++;
	up_power_delay_cnt ++;
	if(lpuart1Rec.timeOut > 0)  lpuart1Rec.timeOut ++;	//串口接收超时计数

}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected to the EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        Si4438_Interrupt_Handler(&Wireless_Buf);
    }
}

/*
串口2接收完成回调函数
*/
uint8_t frameLen = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    extern uint8_t LP_UartRec;
    if (huart->Instance == LPUART1) //如果是串口1
    {
        //#if 1
        //			//烧录设备MCA协议接收
        //			//帧头  命令 长度           MAC(8)           CRC16
        //			//  68  20   08   04  02 00 01 00 00 02 01    7C 6B
        //			UpUart_RX_INT_Process(LP_UartRec, &MAC_UartRec);   //烧录/读取设备MAC协议
        //			DoorLockDataReadFromUart(LP_UartRec,&lpuart1Rec); //通信协议
        //#else

        //		//按串口接收数据超时接收数据
        //        if (lpuart1Rec.rec_ok == 0)
        //        {
        //            lpuart1Rec.timeOut = 0;  //超时接收变量清空
        //            lpuart1Rec.buff[lpuart1Rec.cnt++] = LP_UartRec;
        //        }
        //#endif


        UpUart_RX_INT_Process(LP_UartRec, &MAC_UartRec); //烧录/读取设备MAC协议
        
        if (lpuart1Rec.rec_ok == 0)   			//按串口接收数据超时接收数据
        {
            lpuart1Rec.timeOut = 1; 			//超时接收变量清空

            lpuart1Rec.buff[lpuart1Rec.cnt++] = LP_UartRec;
            if (lpuart1Rec.cnt > UART_RECV_BUFF_SIZE)
            {
                lpuart1Rec.cnt = UART_RECV_BUFF_SIZE;
            }
        }
    }
}
