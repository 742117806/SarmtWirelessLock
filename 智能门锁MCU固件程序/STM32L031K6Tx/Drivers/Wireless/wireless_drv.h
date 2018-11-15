#ifndef WIRELESS_DRV_H
#define WIRELESS_DRV_H

#include "wireless_app.h"


#define PREAMBLE_LENGTH                     100
#define SYNC_31_24                          0x98
#define SYNC_23_16                          0xf3

#define TX_THRESHOLD                        0x30
#define RX_THRESHOLD                        0x30

#define RESET_TRANSMIT                      0x01
#define RESET_RECEIVE                       0x02


#define NO_CHANGE  									0
#define SLEEP_STATE  								1
#define SPI_ACTIVE_STATE 						2
#define READY_STATE 								3
#define ANOTHER_ENUMERATION_STATE 	4
#define TURE_TX_STATE 							5
#define TURE_RX_STATE 							6
#define TX_STATE 										7
#define RX_STATE 										8




struct si4438_GET_INT_STATUS_Reply 
{
    uint8_t  INT_PEND;
    uint8_t  INT_STATUS;
    uint8_t  PH_PEND;
    uint8_t  PH_STATUS;
    uint8_t  MODEM_PEND;
    uint8_t  MODEM_STATUS;
    uint8_t  CHIP_PEND;
    uint8_t  CHIP_STATUS;
};

struct si4438_REQUEST_DEVICE_STATE_Reply 
{
    uint8_t  CURRENT_STATE;
    uint8_t  CURRENT_CHANNEL;
};

struct si4438_GET_MODEM_STATUS_Reply 
{
    uint8_t  MODEM_PEND;
    uint8_t  MODEM_STATUS;
    uint8_t  CURR_RSSI;
    uint8_t  LATCH_RSSI;
    uint8_t  ANT1_RSSI;
    uint8_t  ANT2_RSSI;
    int16_t AFC_FREQ_OFFSET;
};


struct si4438_GET_ADC_READING_Reply 
{
    uint16_t GPIO_ADC;
    uint16_t BATTERY_ADC;
    uint16_t TEMP_ADC;
    uint8_t  TEMP_SLOPE;
    uint8_t  TEMP_INTERCEPT;
};


struct si4438_PART_INFO_Reply 
{
    uint8_t  CHIPREV;
    uint16_t PART;
    uint8_t  PBUILD;
    uint16_t ID;
    uint8_t  CUSTOMER;
    uint8_t  ROMID;
};

struct si4438_GET_PH_STATUS_Reply 
{
    uint8_t  PH_PEND;
    uint8_t  PH_STATUS;
};


struct si4438_FRR_READ_Reply
{
    uint8_t  FRR_A_VALUE;
    uint8_t  FRR_B_VALUE;
    uint8_t  FRR_C_VALUE;
    uint8_t  FRR_D_VALUE;
};

 union si4438_cmd_reply_union 
{
  struct si4438_GET_INT_STATUS_Reply              GET_INT_STATUS;
  struct si4438_REQUEST_DEVICE_STATE_Reply        REQUEST_DEVICE_STATE;
  struct si4438_GET_MODEM_STATUS_Reply            GET_MODEM_STATUS;
  struct si4438_GET_ADC_READING_Reply             GET_ADC_READING;
  struct si4438_PART_INFO_Reply                   PART_INFO;
  struct si4438_GET_PH_STATUS_Reply               GET_PH_STATUS;
  struct si4438_FRR_READ_Reply                    FRR_READ;
};


volatile extern union si4438_cmd_reply_union  Si4438Cmd_Reply;

#if(1)
extern uint8_t FrrA_Val;  //
extern uint8_t FrrB_Val;
extern uint8_t FrrC_Val;
extern uint8_t FrrD_Val;
#endif

extern void Get_WireLessChannel(uint8_t* wire_chnel);
extern uint8_t const Channel_Frequency_Index[66];
extern unsigned char  RF_RX_HOP_CONTROL_12[];

/////////////////////////////////////////////////////////
void Si4438_Delay_ms(uint16_t nms);



void SI4438_ReadCmdBuff(uint8_t rw_mode);
void Set_Property(unsigned char const pdata[]);
void Get_Property(uint8_t group_ID, uint8_t start_ID, uint8_t lenth, uint8_t *pdata);

//void Get_INTState(void);
void Get_INTState(uint8_t int_status[8], uint8_t ph_clr, uint8_t mod_clr, uint8_t chip_clr);
void Get_PartInfo(void);
void Get_FuncInfo(uint8_t * spr);
void Get_PHstate(uint8_t ph_status[2]);
void Get_ADC_Value(void);
//void Request_DeviceState(void);
uint8_t Request_DeviceState(void);
void Change_State(uint8_t state);
void Start_TX(uint8_t channel, uint8_t condition, uint16_t lenth);
void Start_RX(uint8_t channel, uint8_t condition, uint16_t lenth, uint8_t timeout_state, uint8_t valid_state, uint8_t invalid_state);
void Write_TXfifo(uint8_t *pdata, uint8_t lenth);
void Read_RxFifo(uint8_t *pdata, uint8_t lenth);
void RX_Hop(uint8_t inte, uint32_t frac, uint8_t vco_cnt1, uint8_t vco_cnt0);

void Read_FRR(void);
void Res_FifoInfo(uint8_t parameter);
void Get_ModemStatus(void);
void Change_RxThreshold(uint8_t value);
void Rx_Hop(uint8_t const pchannel[6]);
Wireless_ErrorType SI4438_PWUP_Init(void);


extern uint8_t const Receive_Interrupt_set[];
extern uint8_t const Transmit_Interrupt_set[];
extern uint8_t const Interrupt_Close[];
extern uint8_t const RxHop_Close[];

void Si4438_Property_Init(void);













#endif

