#include "user_call_back.h"
#include "uart.h"
#include "device.h"
#include "wireless_app.h"

extern uint16_t sleep_delay_cnt;

/**
  * @brief  SYSTICK callback.
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
    //	WirelessRx_Timeout_Cnt ++;			//���߽��յ�ͬ���ֺ���պ������ݳ�ʱ����
	sleep_delay_cnt ++;
	//���ڳ�ʱ���ռ���
//    lpuart1Rec.timeOut++;
//    if ((lpuart1Rec.timeOut > 30) && (lpuart1Rec.cnt > 0))
//    {
//        lpuart1Rec.Len = lpuart1Rec.cnt;
//        lpuart1Rec.cnt = 0;
//        lpuart1Rec.state = LOCK_FRAME_FENISH;
//    }
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
����2������ɻص�����
*/
uint8_t frameLen = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    extern uint8_t LP_UartRec;
    if (huart->Instance == LPUART1) //����Ǵ���1
    {
#if 1    
			//��¼�豸MCAЭ�����
			//֡ͷ  ���� ����           MAC(8)           CRC16
			//  68  20   08   04  02 00 01 00 00 02 01    7C 6B
			UpUart_RX_INT_Process(LP_UartRec, &MAC_UartRec);   //��¼/��ȡ�豸MACЭ��
			DoorLockDataReadFromUart(LP_UartRec,&lpuart1Rec); //ͨ��Э��
#else

		//�����ڽ������ݳ�ʱ��������
        if (lpuart1Rec.rec_ok == 0)     
        {
            lpuart1Rec.timeOut = 0;  //��ʱ���ձ������
            lpuart1Rec.buff[lpuart1Rec.cnt++] = LP_UartRec;			
        }	
#endif
    }
}
