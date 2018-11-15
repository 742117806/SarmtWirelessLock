#include "wireless_app.h"
#include "wireless_drv.h"
//#include "frame_process.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "spi.h"
#include "user_call_back.h"

#include "delay.h"
#include "device.h"
#include "74.h"
#include "delay.h"
//#include "gpio.h"
//#include "exit.h"



volatile uint16_t WirelessRx_Timeout_Cnt;   //无线接收超时计数
volatile Wireless_ErrorType WIRELESS_ERROR_CODE;
volatile Wireless_Status_TypDef WIRELESS_STATUS;
WLS Wireless_Buf;


uint8_t Wireless_Channel[2];  //存放一个信道组的两个通信频点
//uint8_t p[5] = {0XFF,0XFF,0XFF};

static uint8_t TRANSMIT_REMAIN = 0;
static uint8_t TRANSMIT_TIMES = 0;
static uint8_t RECEIVE_TIMES = 0;
static uint8_t RECEIVE_REMAIN = 0;

//通道转成物理频道的对应关系
uint8_t const Channel_Frequency_Index[66] = {24,66, 0,39, 1,40, 2,41, 3,42, 4,43, 5,44, 6,45, 7,46, 8,47, 9,48,10,49,11,50,12,51,13,52,14,53,15,
                                               54,16,55,17,56,18,57,19,58,20,59,21,60,22,61,23,62,25,63,26,64,27,65,28,67,29,68,30,69,31,70,32,71};

static uint8_t const Frequency_Channel_Index[72] = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,0,50,52,54,56,58,60,62,64,
                                               88,88,88,88,88,88,
                                               3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,1,57,59,61,63,65};

//extern volatile time_t time_dat;
////给time函数使用
//time_t __time32 (time_t * p) {
//	return time_dat;
//}
////a -- bms随机延时
//void Rand_delay(uint8_t a,uint8_t b)
//{
//  //srand(time((uint32_t *)NULL));
//  srand(HAL_GetTick());
//  uint32_t Time = ((uint32_t)rand() % (b - a + 1)) + 10;
//  delay_ms(Time);
//}

Wireless_ErrorType Wireless_Init(void)
{
  uint8_t i = 3;
  
  //SDN_Init();         //只能放在前面？
  //WIRELESS_SPI_Init();
  //SDN_Init();
  //NIRQ_Int_Init();
  while((SI4438_PWUP_Init() != Wireless_NoError) && i)  i--;
  if(i == 0)
  {
    WIRELESS_ERROR_CODE = Wireless_InitError;
    SDN_H();                    //power_off
    return Wireless_InitError;
  }
	Si4438_Property_Init();
  return Wireless_NoError;
}



void Si4438_Interrupt_Handler(WLS *pWL)
{
  uint8_t int_status[8] = {0};
  uint8_t temp_int; 
  uint8_t temp_int2;
  
  Get_INTState(int_status, 0,0,0);
  temp_int = int_status[2];             //对接收到的中断状态作判断
  temp_int2 = int_status[4];
  
  if((temp_int2 & 0x01) == 0x01)    //检测到前导码
  {
    WirelessRx_Timeout_Cnt = 1;
  }
  if((temp_int2 & 0X08) == 0x08)        //RSSI interrupt-->Channel Busy
  {
    WIRELESS_ERROR_CODE = Wireless_Busy;
  }
  if((temp_int & 0x08) == 0x08)        //receive failure
  {
    WIRELESS_STATUS = Wireless_RX_Failure;
    //Set_Property(Interrupt_Close);
    WirelessRx_Timeout_Cnt = 0;
  }
  else if((temp_int & 0x10) == 0x10)         //receive accomplish
  {
    if(RECEIVE_TIMES == 0)
    {
      Read_RxFifo(&pWL->Wireless_FrameHeader[0], 4);
      if(pWL->Wireless_FrameHeader[0] > 3)
        Read_RxFifo(&pWL->Wireless_RxData[0], pWL->Wireless_FrameHeader[0] - 3);
    }
    else if(RECEIVE_REMAIN > 0)
    {
      Read_RxFifo(&pWL->Wireless_RxData[RX_THRESHOLD * RECEIVE_TIMES - 4], RECEIVE_REMAIN);
    }
    
    Get_ModemStatus();
    pWL->Wireless_RSSI_Value = Si4438Cmd_Reply.GET_MODEM_STATUS.LATCH_RSSI;
    //Set_Property(Interrupt_Close);
    
    if(Si4438Cmd_Reply.REQUEST_DEVICE_STATE.CURRENT_CHANNEL < 33 || (Si4438Cmd_Reply.REQUEST_DEVICE_STATE.CURRENT_CHANNEL > 38 && Si4438Cmd_Reply.REQUEST_DEVICE_STATE.CURRENT_CHANNEL < 72))
    {
      pWL->Wireless_ChannelNumber = Frequency_Channel_Index[Si4438Cmd_Reply.REQUEST_DEVICE_STATE.CURRENT_CHANNEL];
    }
    
    pWL->Wireless_PacketLength = pWL->Wireless_FrameHeader[0] - 3;
    pWL->Wireless_ChannelNumber = pWL->Wireless_FrameHeader[1];
    WIRELESS_STATUS = Wireless_RX_Finish;
    WirelessRx_Timeout_Cnt = 0;
  }
  
  else if((temp_int & 0x01) == 0x01)    //receive start
  {
    if(RECEIVE_TIMES == 0)
    {      
      Read_RxFifo(&pWL->Wireless_FrameHeader[0], 4);
      Read_RxFifo(&pWL->Wireless_RxData[0], RX_THRESHOLD - 4);
      RECEIVE_REMAIN = (pWL->Wireless_FrameHeader[0] + 1) - RX_THRESHOLD;
      RECEIVE_TIMES = 1; 
    }
    else
    {
      Read_RxFifo(&pWL->Wireless_RxData[RX_THRESHOLD * RECEIVE_TIMES - 4], RX_THRESHOLD);
      RECEIVE_REMAIN -= RX_THRESHOLD;
      RECEIVE_TIMES ++;
    }
  }
  
  else if((temp_int & 0x20) == 0x20)        //send finish
  {
    //Set_Property(Interrupt_Close);
    TRANSMIT_REMAIN = 0;
    WIRELESS_STATUS = Wireless_TX_Finish;
    pWL->Sent_TimeOut_Cnt = 0;
  }
  
  else if((temp_int & 0x02) == 0x02)        //send threshold
  {
    if(TRANSMIT_REMAIN > TX_THRESHOLD)
    {
      Write_TXfifo(&pWL->Wireless_TxData[60 + TX_THRESHOLD * TRANSMIT_TIMES], TX_THRESHOLD);
      TRANSMIT_TIMES ++;
      TRANSMIT_REMAIN -= TX_THRESHOLD;
    }
    else if(TRANSMIT_REMAIN > 0)
    {
      Write_TXfifo(&pWL->Wireless_TxData[60 + TX_THRESHOLD * TRANSMIT_TIMES], TRANSMIT_REMAIN);
      TRANSMIT_REMAIN = 0;
    }
  }
}
/*
void Channel_Change(void)
{
  static uint8_t Channel_cnt = 0;
  
  if(WIRELESS_STATUS == Wireless_Free)   //信道无数据
  {
    if(Channel_cnt == 0)
    {
      //Channel_cnt = 1;
      Si4438_Receive_Start(Default_Channel);   //Start Receive
    }
    else if(Channel_cnt == 1)
    {
      //Channel_cnt = 2;
      Channel_cnt = 0;
      Si4438_Receive_Start(Wireless_Channel[1]);   //Start Receive
    }
    else
    {
      Channel_cnt = 0;
      Si4438_Receive_Start(Wireless_Channel[0]);   //Start Receive
    }
      
  }
}
*/
uint8_t RSSI_Get(void)
{
  uint8_t rssi_val = 0;
  
  Read_FRR();
  //rssi_val = (255 - Si4438Cmd_Reply.FRR_READ.FRR_D_VALUE) / 2;
  rssi_val = Si4438Cmd_Reply.FRR_READ.FRR_D_VALUE;
  
  if(rssi_val > 120) return 120;
  else if(rssi_val < 40) return 40;
  else return rssi_val;
}




Wireless_ErrorType Si4438_Receive_Start(uint8_t channel_number)
{
  uint8_t int_status[8] = {0};
  Get_INTState(int_status, 0,0,0);
  
  if(WIRELESS_ERROR_CODE == Wireless_InitError)  return Wireless_InitError;
  if(channel_number > 65) channel_number = 65;
  
  Res_FifoInfo(RESET_RECEIVE);
  //Set_Property(Receive_Interrupt_set);
  Start_RX(Channel_Frequency_Index[channel_number], 0, 0, NO_CHANGE, READY_STATE, RX_STATE);
  RECEIVE_TIMES = 0;
  RECEIVE_REMAIN = 0;
  WIRELESS_STATUS = Wireless_RX_Receiving;
  return Wireless_NoError;
}



//////////////////////////////////////////////////////////////////////////////////////////////////

Wireless_ErrorType Si4438_Transmit(WLS *pWL, uint8_t channel_number, uint8_t *addr, uint8_t length)
{
  uint8_t i;
  uint8_t int_status[8] = {0};/*
  Get_INTState(int_status, 0,0,0);
  */
  if(length > 251) length = 251;
  if(channel_number > 65) channel_number = 65;
  
  if(WIRELESS_ERROR_CODE == Wireless_Busy)
  {
    pWL->Channel_Busy_cnt = 0;                //clear busy time
    WIRELESS_STATUS = Wireless_Free;
    return Wireless_ExecuteError;
  }
  
  for(i = 0; i < length; i++)
  {
    pWL->Wireless_TxData[i] = addr[i];  //get data that ready to send
  }
  pWL->Channel_Busy_cnt = 1;
  
  pWL->Wireless_FrameHeader[0] = length + 3;
  pWL->Wireless_FrameHeader[1] = channel_number;
  pWL->Wireless_FrameHeader[2] = 0x01;
  pWL->Wireless_FrameHeader[3] = (pWL->Wireless_FrameHeader[0]^pWL->Wireless_FrameHeader[1])^pWL->Wireless_FrameHeader[2];
  pWL->Channel_Busy_cnt = 0;                //clear busy time
  
  Change_State(READY_STATE);
  
  Get_INTState(int_status, 0,0,0);
  
  Res_FifoInfo(RESET_TRANSMIT);
  if(length > 60) 
  {
    TRANSMIT_REMAIN = length - 60;
    Write_TXfifo(&pWL->Wireless_FrameHeader[0], 4);
    Write_TXfifo(&pWL->Wireless_TxData[0], 60);
  }
  else
  {
    TRANSMIT_REMAIN = 0;
    Write_TXfifo(&pWL->Wireless_FrameHeader[0], 4);
    Write_TXfifo(&pWL->Wireless_TxData[0], length);
  }
  WIRELESS_STATUS = Wireless_TX_Sending;
  //Set_Property(Transmit_Interrupt_set);
  
  Start_TX(Channel_Frequency_Index[channel_number], 0x30, length + 4);
  TRANSMIT_TIMES = 0;
  return Wireless_NoError;
}


Wireless_ErrorType Si4438_Transmit_Start(WLS *pWL, uint8_t channel_number, uint8_t *addr, uint8_t length)
{
  uint8_t i,cnt = 0;
  uint8_t int_status[8] = {0};
  volatile uint16_t dest = 0;
  uint8_t temp[2] = {0};
  
  
  if(WIRELESS_ERROR_CODE == Wireless_InitError)  return Wireless_InitError;
  if(length > 251) length = 251;
  if(channel_number > 65) channel_number = 65;
  
//  Si4438_Receive_Start(channel_number);     //听信道
//  Rand_delay(20,40);
//  
//  if(WIRELESS_ERROR_CODE == Wireless_Busy)    //First application
//  {
//    pWL->Channel_Busy_cnt = 0;                //clear busy time
//    WIRELESS_STATUS = Wireless_Free;
//    return Wireless_ExecuteError;
//  }
  
  for(i = 0; i < length; i++)
  {
    pWL->Wireless_TxData[i] = addr[i];  //get data that ready to send
  }
  
  pWL->Channel_Busy_cnt = 1;            //Cnt busy time
  
#ifdef Use_74dcode 
  if(((pWL->Wireless_TxData[Region_CmdNumber] & 0X07) != 0X07) && ((pWL->Wireless_TxData[Region_CmdNumber] & 0X07) != 0X02))       //带编码功能
  {
    if(pWL->Wireless_TxData[Region_DataLenNumber] < UpDate_Len)      //长包编码后过长，不编码
    {
      if(!(pWL->Wireless_TxData[Region_SeqNumber] & 0X80))  //有路由标志，直接转发出去
      {
        for(i = Region_DataAFNNumber;i<length-1;i++)             //把数据和CRC16校验码进行74编码
        {
          dest = code16_74bit(pWL->Wireless_TxData[i]);
          temp[0] = dest >> 8;                                    //把16位分解
          temp[1] = dest & 0Xff;
          pWL->Wireless_RealData[cnt] = temp[0];
          pWL->Wireless_RealData[cnt+1] = temp[1];
          cnt+=2;                                                 //编码后的数据长度
        }
        memset(&pWL->Wireless_TxData[Region_DataAFNNumber],(uint8_t)0,length);   //把要发送的原帧清除，否则会有无用数据在帧的后缀出现
        memmove(&pWL->Wireless_TxData[Region_DataAFNNumber],pWL->Wireless_RealData,cnt);
        memset(pWL->Wireless_RealData,(uint8_t)0,cnt);                     //清除编码buf，否则影响下次的发送编码处理
        length = cnt+8;
        uint8_t *p = pWL->Wireless_TxData;
        length = Frame_Check(p,length);                           //重新对编码帧校验发送
      }
    }
  }
#endif 
  
  pWL->Wireless_FrameHeader[0] = length + 3;
  pWL->Wireless_FrameHeader[1] = channel_number;
  pWL->Wireless_FrameHeader[2] = 0x01;
  pWL->Wireless_FrameHeader[3] = (pWL->Wireless_FrameHeader[0]^pWL->Wireless_FrameHeader[1])^pWL->Wireless_FrameHeader[2];
  pWL->Channel_Busy_cnt = 0;                //clear busy time
  
  Change_State(READY_STATE);
  
  Get_INTState(int_status, 0,0,0);
  
  Res_FifoInfo(RESET_TRANSMIT);
  if(length > 60) 
  {
    TRANSMIT_REMAIN = length - 60;
    Write_TXfifo(&pWL->Wireless_FrameHeader[0], 4);
    Write_TXfifo(&pWL->Wireless_TxData[0], 60);
  }
  else
  {
    TRANSMIT_REMAIN = 0;
    Write_TXfifo(&pWL->Wireless_FrameHeader[0], 4);
    Write_TXfifo(&pWL->Wireless_TxData[0], length);
  }
  WIRELESS_STATUS = Wireless_TX_Sending;
  pWL->Sent_TimeOut_Cnt = 1;
  //Set_Property(Transmit_Interrupt_set);
  Start_TX(Channel_Frequency_Index[channel_number], 0x30, length + 4);
  TRANSMIT_TIMES = 0;
  return Wireless_NoError;
}








uint8_t PLME_ED_Request(void)
{
  uint8_t i = 0;
  uint8_t max = 0;
  uint8_t min = 0xff;
  uint16_t sum = 0;
  Change_State(RX_STATE);
  for(i = 0; i < 12; i++)
  {
//    delay_us(400);
    Get_ModemStatus();
    max = max > Si4438Cmd_Reply.GET_MODEM_STATUS.CURR_RSSI ? max : Si4438Cmd_Reply.GET_MODEM_STATUS.CURR_RSSI;
    min = min < Si4438Cmd_Reply.GET_MODEM_STATUS.CURR_RSSI ? min : Si4438Cmd_Reply.GET_MODEM_STATUS.CURR_RSSI;
    sum += Si4438Cmd_Reply.GET_MODEM_STATUS.CURR_RSSI;
  }
  return (uint8_t)((sum - max - min) / 10);
}



uint8_t PLME_CCA(void)
{
  uint8_t i;
  //Change_State(RX_STATE);
  Get_ModemStatus();
  for(i = 0; i < 15; i++)
  {
    Read_FRR();
    if((Si4438Cmd_Reply.FRR_READ.FRR_B_VALUE & 0x08) == 0x08) return CHANNEL_BUSY;
  }
  return CHANNEL_FREE;
}




































////////////////////////////////////////////////////////////////////////////
/******************* 不使用中断时使用的接收发送函数 ***********************/
Wireless_Handle_Status Si4438_RX_Start(uint8_t channel_number)
{
  uint8_t ph_status[2];
  //uint8_t aaaa[] = {0x11,0x12,0x02,0x0d,0x00,0x01};
  
  if(WIRELESS_ERROR_CODE == Wireless_InitError)  return WIRELESS_NO_EXIST;
  if(channel_number > 65) return RX_PARA_ERROR;
	//Set_Property(aaaa);
  Res_FifoInfo(RESET_RECEIVE);
  Get_PHstate(ph_status);
  Start_RX(Channel_Frequency_Index[channel_number], 0, 0, 0, 3, 0);
   
  return RECEIVE_NO_DATA;
}



Wireless_Handle_Status si4438_DataReceive(WLS *pWL)
{
  uint8_t ph_state[2];
  uint8_t RX_TIMES = 0;
  uint8_t RX_REMAIN = 0;
  uint16_t RX_timeout;
  uint8_t ph_status;
  
  if(WIRELESS_ERROR_CODE == Wireless_InitError)  return WIRELESS_NO_EXIST;
  /*
  if(WIRELESS_ERROR_CODE == Wireless_CmdResponseError)
  {
    if(SI4438_PWUP_Init() != Wireless_NoError) 
    {
      WIRELESS_ERROR_CODE = Wireless_InitError;
      SDN_H;                    //power_off
      return WIRELESS_NO_EXIST;
    }
    Si4438_Property_Init();
  }
  */
  Get_PHstate(ph_state);      //获取状态，同时清除标志
  ph_status = ph_state[0];
  if(!(ph_status & 0x19))  return RECEIVE_NO_DATA;
  if(((ph_status & 0x08) == 0x08) || ((ph_status & 0x11) == 0x11)) return RX_ERROR;
  Read_RxFifo(&pWL->Wireless_FrameHeader[0], 4);        //读出帧头，以便取出长度字节 
  RX_REMAIN = pWL->Wireless_FrameHeader[0] + 1;    //连自己本身一共需要接收的字节数
  if(RX_REMAIN < 5) return RX_LENGTH_ERROR;
  
  while(((ph_status & 0x01) == 0x01) && (RX_REMAIN > RX_THRESHOLD))    //剩余数大于门槛
  {  
    if(RX_TIMES == 0)
      Read_RxFifo(&pWL->Wireless_RxData[0], RX_THRESHOLD - 4);    //开始已读了4个
    else
      Read_RxFifo(&pWL->Wireless_RxData[RX_THRESHOLD * RX_TIMES - 4], RX_THRESHOLD);
    RX_REMAIN -= RX_THRESHOLD;
    RX_TIMES ++;
    RX_timeout = 0;         //每一次循环超时计数清0
    do
    {
      if(++RX_timeout > (uint8_t)(RX_THRESHOLD * 8 / 10 + 10)) return RECEIVE_TIMEOUT;  //超时检测,由于有误差,附加10ms
      Si4438_Delay_ms(1);
      Get_PHstate(ph_state);      
      ph_status = ph_state[0];
    }
    while(!(ph_status & 0x19));      //有数据收到 
  }
  if((ph_status & 0x10) == 0x10)
  {
    if(RX_TIMES == 0)
      Read_RxFifo(&pWL->Wireless_RxData[0], RX_REMAIN - 4);    //开始已读了4个
    else
      Read_RxFifo(&pWL->Wireless_RxData[RX_THRESHOLD * RX_TIMES - 4], RX_REMAIN);
    pWL->Wireless_RSSI_Value = RSSI_Get();
    pWL->Wireless_ChannelNumber = pWL->Wireless_FrameHeader[1];
    pWL->Wireless_PacketLength = pWL->Wireless_FrameHeader[0] - 3;
    return RECEIVE_SUCCEED;
  }
  return RX_ERROR;
}
  
  
Wireless_Handle_Status si4438_DataSend(WLS *pWL, uint8_t channel_number, uint8_t *addr, uint8_t length)
{
  uint8_t ph_state[2];
  uint8_t i;
  uint8_t ph_status;
  uint16_t TX_timeout = 0;
  uint8_t TX_REMAIN = 0;
  uint8_t TX_TIMES = 0;
  
  if(WIRELESS_ERROR_CODE == Wireless_InitError)  return WIRELESS_NO_EXIST;

  if((length > 251) || (channel_number > 65)) return SEND_PARA_ERROR;

  Change_State(READY_STATE);
  pWL->Wireless_FrameHeader[0] = length + 3;
  pWL->Wireless_FrameHeader[1] = channel_number;
  pWL->Wireless_FrameHeader[2] = 0x01;
  pWL->Wireless_FrameHeader[3] = (pWL->Wireless_FrameHeader[0]^pWL->Wireless_FrameHeader[1])^pWL->Wireless_FrameHeader[2];
  for(i = 0; i < length; i++ )
  {
    pWL->Wireless_RxData[i] = addr[i];
  }
  Res_FifoInfo(RESET_TRANSMIT);
  if(length > 60) 
  {
    TX_REMAIN = length - 60;
    Write_TXfifo(&pWL->Wireless_FrameHeader[0], 4);
    Write_TXfifo(&pWL->Wireless_RxData[0], 60);
  }
  else
  {
    TX_REMAIN = 0;
    Write_TXfifo(&pWL->Wireless_FrameHeader[0], 4);
    Write_TXfifo(&pWL->Wireless_RxData[0], length);
  }
  Get_PHstate(ph_state);
	Start_TX(Channel_Frequency_Index[channel_number], 0x30, length + 4);
  Si4438_Delay_ms(PREAMBLE_LENGTH * 8 / 10);           //跳过引导码的发送时间(字节数*位数*每位的ms时间) 
  do
  {
    if(++TX_timeout > (uint8_t)(TX_THRESHOLD * 8 / 10 + 10)) return SEND_TIMEOUT;    //超时检测,由于有误差,附加10ms
    Si4438_Delay_ms(1);           
    Get_PHstate(ph_state);      
    ph_status = ph_state[0];
    //Read_FRR();
    //ph_status = Si4438Cmd_Reply.FRR_READ.FRR_C_VALUE;
  }
  while(!(ph_status & 0x22));      //等待状态
  //Get_PHstate();
  
  while((ph_status & 0x02) == 0x02)
  {
    if(TX_REMAIN > TX_THRESHOLD)
    {
      Write_TXfifo(&pWL->Wireless_RxData[60 + TX_THRESHOLD * TX_TIMES], TX_THRESHOLD);
      TX_TIMES ++;
      TX_REMAIN -= TX_THRESHOLD;
    }
    else if(TX_REMAIN > 0)
    {
      Write_TXfifo(&pWL->Wireless_RxData[60 + TX_THRESHOLD * TX_TIMES], TX_REMAIN);
    }
    TX_timeout = 0;     //每一次循环超时计数清0 
    
    do
    {
      if(++TX_timeout > (uint8_t)(TX_THRESHOLD * 8 / 10 + 10)) return SEND_TIMEOUT;    //超时检测,由于有误差,附加10ms
      Si4438_Delay_ms(1);           
      Get_PHstate(ph_state);      
      ph_status = ph_state[0];
      //Read_FRR();
      //ph_status = Si4438Cmd_Reply.FRR_READ.FRR_C_VALUE;
    }
    while(!(ph_status & 0x22));      //等待状态
    //Get_PHstate();
    if((ph_status & 0x20) == 0x20)  return SEND_SUCCEED;
  }
  return SEND_SUCCEED;
}  
/*************************************** END **********************************************/
////////////////////////////////////////////////////////////////////////////////////////////



