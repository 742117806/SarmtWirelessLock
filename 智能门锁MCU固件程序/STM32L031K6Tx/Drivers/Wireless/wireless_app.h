#ifndef __WIRELESS_APP_H__
#define __WIRELESS_APP_H__

#include "protocol.h"
#include "stm32l0xx.h"
#include "uart.h"



#define WLSent_TimeOut_Val            25
#define HKData_LenMax                               244          //数据标识4+数据8MAX
#define HKFrame_LenMax                              (11+HKData_LenMax)     //头1+地址4+配置序号1+控制码1+数据长度1 +校验2+尾1

/*****************  si4438 data receive  ********************/
typedef struct
{
  uint8_t Wireless_ChannelNumber;
  uint8_t Wireless_RSSI_Value;
  uint8_t Wireless_PacketLength;
  uint8_t Wireless_FrameHeader[4];
  uint8_t Wireless_RxData[HKFrame_LenMax];
  //uint8_t Wireless_TxHeader[4];
  uint8_t Wireless_TxData[HKFrame_LenMax];
  uint8_t Wireless_RealData[HKFrame_LenMax];
  uint8_t Sent_TimeOut_Cnt;
  uint8_t Channel_Busy_cnt;
}WLS;


//volatile extern uint8_t Wireless_PACKET_HANDLER_STATUS;
//volatile extern uint8_t CURRENT_CHANNEL_NUMBER;

//#define HANDLER_FINISH                       0x00
//#define HANDLER_PENDING                      0x01

/*******************  si4438 device status  **********************/



typedef enum
{
  Wireless_Free = 0,
  Wireless_RX_Receiving,
  Wireless_RX_Idle,
  Wireless_RX_Finish,
  Wireless_RX_Failure,
  Wireless_TX_Sending,
  Wireless_TX_Finish,
  Wireless_TX_Failure
}Wireless_Status_TypDef;

extern volatile Wireless_Status_TypDef WIRELESS_STATUS;







/***********************  error code ***************************/

typedef enum
{
  Wireless_NoError = 0,
  Wireless_InitError,
  Wireless_CmdResponseError,
  Wireless_Busy,
  Wireless_ExecuteError
}Wireless_ErrorType;



typedef enum
{
  RECEIVE_NO_DATA = 0,
  RECEIVE_SUCCEED,
  SEND_SUCCEED,
  RX_ERROR,
  SEND_ERROR,
  RX_LENGTH_ERROR,
  RX_PARA_ERROR,
  SEND_PARA_ERROR,
  RECEIVE_TIMEOUT,
  SEND_TIMEOUT,
  WIRELESS_NO_EXIST,
  
}Wireless_Handle_Status;




#define CHANNEL_BUSY                              0X00
#define CHANNEL_FREE                              0X01

#define WirelessRx_Timeout_Threshold              18
#define BUSY_TIMEOUT                              18    //150ms


//void Si4438_Delay_ms(u16 nms);
#define Use_Rx_Hop

#define  Default_Channel            10

/************************* 外部变量 ***************************/
extern volatile  Wireless_ErrorType WIRELESS_ERROR_CODE;
extern WLS Wireless_Buf;
extern volatile uint16_t WirelessRx_Timeout_Cnt;
extern uint8_t Wireless_Channel[2];

extern uint16_t test;
/************************* 外部函数 ***************************/
Wireless_ErrorType Wireless_Init(void);
void Si4438_Interrupt_Handler(WLS *pWL);
uint8_t RSSI_Get(void);
//void Si4438_PhysicalLayer_TransmitDataConvert(WLS *pWL, uint8_t channel_number,uint8_t *addr, uint8_t length);
//void Si4438_PhysicalLayer_ReceiveDataConvert(WLS *pWL);
Wireless_ErrorType Si4438_Transmit_Start(WLS *pWL, uint8_t channel_number, uint8_t *addr, uint8_t length);
Wireless_ErrorType Si4438_Transmit(WLS *pWL, uint8_t channel_number, uint8_t *addr, uint8_t length);
Wireless_ErrorType Si4438_Receive_Start(uint8_t channel_number);


void Wireless_RestartRX(uint8_t channel_number);
uint8_t PLME_ED_Request(void);
uint8_t PLME_CCA(void);

Wireless_Handle_Status Si4438_RX_Start(uint8_t channel_number);
Wireless_Handle_Status si4438_DataReceive(WLS *pWL);
Wireless_Handle_Status si4438_DataSend(WLS *pWL, uint8_t channel_number, uint8_t *addr, uint8_t length);











#endif



