#include "wireless_drv.h"
#include "spi.h"

extern void HAL_Delay(__IO uint32_t Delay);

volatile static uint16_t tocnt;

volatile union si4438_cmd_reply_union Si4438Cmd_Reply;

//#pragma optimize = low

#if (0)
uint8_t Int_Pend;
uint8_t Int_Status;
uint8_t Ph_Pend;
uint8_t Ph_Status;
uint8_t Modem_Pend;
uint8_t Modem_Status;
uint8_t Chip_Pend;
uint8_t Chip_Status;

#endif

#if (1)
uint8_t FrrA_Val; //包处理状态，
uint8_t FrrB_Val; //模式状态，
uint8_t FrrC_Val; //未使用，
uint8_t FrrD_Val; //RSSI值
#endif

void Si4438_Delay_ms(uint16_t nms)
{
    HAL_Delay(nms);
}

void SI4438_ReadCmdBuff(uint8_t rw_mode) //READ_CMD_BUFF 0:w 1:r
{
    //uint16_t count = 0x06c0;
    uint16_t count = tocnt;
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x44);
    while ((SPI_RWbyte(0xc0) != 0xff) && count)
    {
        WIRELESS_NSEL_H();
        WIRELESS_NSEL_L();
        SPI_RWbyte(0x44);
        count--;
    }
    if (count == 0)
    {
        WIRELESS_NSEL_H();
        WIRELESS_ERROR_CODE = Wireless_CmdResponseError;
        return;
    }
    if (rw_mode == 0)
    {
        WIRELESS_NSEL_H();
    }
}
#pragma optimize = speed

void Get_INTState(uint8_t int_status[8], uint8_t ph_clr, uint8_t mod_clr, uint8_t chip_clr)
{
    uint8_t loop;

    SI4438_ReadCmdBuff(0);
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x20); //GET_INT_STATUS Parameters
    SPI_RWbyte(ph_clr);
    SPI_RWbyte(mod_clr);
    SPI_RWbyte(chip_clr);
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //GET_INT_STATUS Response
    for (loop = 0; loop < 8; loop++)
    {
        int_status[loop] = SPI_RWbyte(0xaa);
    }
    //Si4438Cmd_Reply.GET_INT_STATUS.INT_PEND = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_INT_STATUS.INT_STATUS = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_INT_STATUS.PH_PEND = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_INT_STATUS.PH_STATUS = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_INT_STATUS.MODEM_PEND = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_INT_STATUS.MODEM_STATUS = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_INT_STATUS.CHIP_PEND = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_INT_STATUS.CHIP_STATUS = SPI_RWbyte(0xaa);
    WIRELESS_NSEL_H();
#if (0)
    Int_Pend = int_status[0];     //中断挂起
    Int_Status = int_status[1];   //中断状态
    Ph_Pend = int_status[2];      //包挂起
    Ph_Status = int_status[3];    //包状态
    Modem_Pend = int_status[4];   //模式挂起
    Modem_Status = int_status[5]; //模式状态
    Chip_Pend = int_status[6];    //芯片挂起
    Chip_Status = int_status[7];  //芯片状态
#endif
}

void Set_Property(unsigned char const pdata[])
{
    uint8_t i = 0;
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF

    WIRELESS_NSEL_L();
    for (i = 0; i < pdata[2] + 4; i++) //pdata[2] is property_number
    {
        SPI_RWbyte(pdata[i]);
    }
    WIRELESS_NSEL_H();
}

void Get_Property(uint8_t group_ID, uint8_t start_ID, uint8_t lenth, uint8_t *pdata)
{
    uint8_t i = 0;
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF

    WIRELESS_NSEL_L();
    SPI_RWbyte(0x12); //GET_PROPERTY Parameters
    SPI_RWbyte(group_ID);
    SPI_RWbyte(lenth);
    SPI_RWbyte(start_ID);
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //GET_PROPERTY Response
    for (i = 0; i < lenth; i++)
    {
        pdata[i] = SPI_RWbyte(0xaa);
    }
    WIRELESS_NSEL_H();
}

void Get_PartInfo(void)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF

    WIRELESS_NSEL_L();
    SPI_RWbyte(0x01); //PART_INFO Parameters
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //PART_INFO Response

    Si4438Cmd_Reply.PART_INFO.CHIPREV = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.PART_INFO.PART = (uint16_t)SPI_RWbyte(0xaa) << 8;
    Si4438Cmd_Reply.PART_INFO.PART |= (uint16_t)SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.PART_INFO.PBUILD = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.PART_INFO.ID = (uint16_t)SPI_RWbyte(0xaa) << 8;
    Si4438Cmd_Reply.PART_INFO.ID |= (uint16_t)SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.PART_INFO.CUSTOMER = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.PART_INFO.ROMID = SPI_RWbyte(0xaa);
    WIRELESS_NSEL_H();
}

void Get_ModemStatus(void)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF

    WIRELESS_NSEL_L();
    SPI_RWbyte(0x22); //GET_MODEM_STATUS Parameters
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //GET_MODEM_STATUS Response
    Si4438Cmd_Reply.GET_MODEM_STATUS.MODEM_PEND = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_MODEM_STATUS.MODEM_STATUS = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_MODEM_STATUS.CURR_RSSI = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_MODEM_STATUS.LATCH_RSSI = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_MODEM_STATUS.ANT1_RSSI = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_MODEM_STATUS.ANT2_RSSI = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_MODEM_STATUS.AFC_FREQ_OFFSET = (int16_t)SPI_RWbyte(0xaa) << 8;
    Si4438Cmd_Reply.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (int16_t)SPI_RWbyte(0xaa);
    WIRELESS_NSEL_H();
}

void Get_FuncInfo(uint8_t *spr)
{
    uint8_t i;

    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF

    WIRELESS_NSEL_L();
    SPI_RWbyte(0x10); //FUNC_INFO Parameters
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //FUNC_INFO Response

    for (i = 0; i < 6; i++)
    {
        spr[i] = SPI_RWbyte(0xaa);
    }
    WIRELESS_NSEL_H();
}

void Get_PHstate(uint8_t ph_status[2])
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x21); //GET_PH_STATUS
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //GET_PH_STATUS Response
    ph_status[0] = SPI_RWbyte(0xaa);
    ph_status[1] = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_PH_STATUS.PH_PEND = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.GET_PH_STATUS.PH_STATUS = SPI_RWbyte(0xaa);
    WIRELESS_NSEL_H();
}

uint8_t Request_DeviceState(void)
{
    uint8_t res_val;

    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x33); //REQUEST_DEVICE_STATE
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //REQUEST_DEVICE_STATE Response
    //Si4438Cmd_Reply.REQUEST_DEVICE_STATE.CURRENT_STATE = SPI_RWbyte(0xaa);
    //Si4438Cmd_Reply.REQUEST_DEVICE_STATE.CURRENT_CHANNEL = SPI_RWbyte(0xaa);
    res_val = SPI_RWbyte(0xaa);
    SPI_RWbyte(0xaa);
    WIRELESS_NSEL_H();

    return res_val;
}

void Change_State(uint8_t state)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x34); //CHANGE_STATE
    SPI_RWbyte(state);
    WIRELESS_NSEL_H();
}

void Start_TX(uint8_t channel, uint8_t condition, uint16_t lenth)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x31); //START_TX
    SPI_RWbyte(channel);
    SPI_RWbyte(condition);
    SPI_RWbyte((uint8_t)lenth >> 8);
    SPI_RWbyte((uint8_t)lenth);
    WIRELESS_NSEL_H();
}

void Start_RX(uint8_t channel, uint8_t condition, uint16_t lenth, uint8_t timeout_state, uint8_t valid_state, uint8_t invalid_state)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x32); //START_RX
    SPI_RWbyte(channel);
    SPI_RWbyte(condition);
    SPI_RWbyte((uint8_t)lenth >> 8);
    SPI_RWbyte((uint8_t)lenth);
    SPI_RWbyte(timeout_state);
    SPI_RWbyte(valid_state);
    SPI_RWbyte(invalid_state);
    WIRELESS_NSEL_H();
}

void GPIO_PIN_CFG(uint8_t *parameter)
{
    uint8_t i;
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    //SPI_RWbyte(0x13);                 //GPIO_PIN_CFG
    for (i = 0; i < 8; i++)
    {
        SPI_RWbyte(parameter[i]);
    }
    WIRELESS_NSEL_H();
}

void Write_TXfifo(uint8_t *pdata, uint8_t lenth)
{
    uint8_t i;
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x66); //WRITE_TX_FIFO
    for (i = 0; i < lenth; i++)
    {
        SPI_RWbyte(pdata[i]);
    }
    WIRELESS_NSEL_H();
}

void Read_RxFifo(uint8_t *pdata, uint8_t lenth)
{
    uint8_t i;
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x77); //READ_TX_FIFO
    for (i = 0; i < lenth; i++)
    {
        pdata[i] = SPI_RWbyte(0xaa);
    }
    WIRELESS_NSEL_H();
}

void Get_ADC_Value(void)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x14); //GET_ADC_READING Parameters
    SPI_RWbyte(0x18);
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //GET_ADC_READING Response
    Si4438Cmd_Reply.GET_ADC_READING.GPIO_ADC = (uint16_t)SPI_RWbyte(0xaa) << 8;
    Si4438Cmd_Reply.GET_ADC_READING.GPIO_ADC |= SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_ADC_READING.BATTERY_ADC = (uint16_t)SPI_RWbyte(0xaa) << 8;
    Si4438Cmd_Reply.GET_ADC_READING.BATTERY_ADC |= SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_ADC_READING.TEMP_ADC = (uint16_t)SPI_RWbyte(0xaa) << 8;
    Si4438Cmd_Reply.GET_ADC_READING.TEMP_ADC |= SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_ADC_READING.TEMP_SLOPE = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.GET_ADC_READING.TEMP_INTERCEPT = SPI_RWbyte(0xaa);
    WIRELESS_NSEL_H();
}

void RX_Hop(uint8_t inte, uint32_t frac, uint8_t vco_cnt1, uint8_t vco_cnt0)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x36); //RX_HOP
    SPI_RWbyte(inte);
    SPI_RWbyte((uint8_t)frac >> 16);
    SPI_RWbyte((uint8_t)frac >> 8);
    SPI_RWbyte((uint8_t)frac);
    SPI_RWbyte(vco_cnt1);
    SPI_RWbyte(vco_cnt1);
    WIRELESS_NSEL_H();
}

void Read_FRR(void)
{
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x50); //FRR_A_READ

#if (1)
    Si4438Cmd_Reply.FRR_READ.FRR_A_VALUE = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.FRR_READ.FRR_B_VALUE = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.FRR_READ.FRR_C_VALUE = SPI_RWbyte(0xaa);
    Si4438Cmd_Reply.FRR_READ.FRR_D_VALUE = SPI_RWbyte(0xaa);
#else
    FrrA_Val = SPI_RWbyte(0xaa);
    FrrB_Val = SPI_RWbyte(0xaa);
    FrrC_Val = SPI_RWbyte(0xaa);
    FrrD_Val = SPI_RWbyte(0xaa);
#endif
    WIRELESS_NSEL_H();
}

void Res_FifoInfo(uint8_t parameter)
{
    uint8_t i;
    uint8_t save[2] = {0};
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x15); //FIFO_INFO Parameters
    SPI_RWbyte(parameter);
    WIRELESS_NSEL_H();

    SI4438_ReadCmdBuff(1); //FIFO_INFO Response
    for (i = 0; i < 2; i++)
    {
        save[i] = SPI_RWbyte(0xaa);
        save[0] = save[1];
    }
    WIRELESS_NSEL_H();
}

void Rx_Hop(uint8_t const pchannel[6])
{
    uint8_t i;
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
    WIRELESS_NSEL_L();
    SPI_RWbyte(0x36); //Fast RX hopping Parameters
    for (i = 0; i < 6; i++)
    {
        SPI_RWbyte(pchannel[i]);
    }
    WIRELESS_NSEL_H();
    SI4438_ReadCmdBuff(0); //READ_CMD_BUFF
}

Wireless_ErrorType SI4438_PWUP_Init(void)
{
    uint8_t int_status[8];
    tocnt = 0x1000;
    SDN_H();
    Si4438_Delay_ms(2);
    SDN_L();

    Si4438_Delay_ms(10);

    WIRELESS_NSEL_L();
    SPI_RWbyte(0x02); //POWER_UP
    SPI_RWbyte(0x01);
    SPI_RWbyte(0x00);
    SPI_RWbyte((uint8_t)(30000000 >> 24));
    SPI_RWbyte((uint8_t)(30000000 >> 16));
    SPI_RWbyte((uint8_t)(30000000 >> 8));
    SPI_RWbyte((uint8_t)(30000000 >> 0));
    WIRELESS_NSEL_H();
    Si4438_Delay_ms(2);
    //WIRELESS_ERROR_CODE = Wireless_NoError;
    Get_INTState(int_status, 0, 0, 0);

    Get_PartInfo();

    tocnt = 0x0020;
    if (Si4438Cmd_Reply.PART_INFO.PART != 0x4438)
        return Wireless_InitError;
    else
        return Wireless_NoError;
}

/*********************************** si4438 的参数设置 ************************************/
//晶振调整
unsigned char const RF_GLOBAL_XO_TUNE_2[] = {0x11, 0x00, 0x02, 0x00, 0x52, 0x00};

//全局配置
unsigned char const RF_GLOBAL_CONFIG_1[] = {0x11, 0x00, 0x01, 0x03, 0x60};

//中断配置
unsigned char const RF_INT_CTL_ENABLE_2[] = {0x11, 0x01, 0x02, 0x00, 0x00, 0x00};

//快速寄存器配置
unsigned char const RF_FRR_CTL_A_MODE_4[] = {0x11, 0x02, 0x04, 0x00, 0x03, 0x05, 0x00, 0x0a}; //包处理状态，模式状态，未使用，RSSI值

//报头设置                                                                               前导有效时间/信道停留时间
unsigned char const RF_PREAMBLE_TX_LENGTH_1[] = {0x11, 0x10, 0x05, 0x00, PREAMBLE_LENGTH, 0x0A, 0x00, 0x0A, 0x12};

//同步字段设置
unsigned char const RF_SYNC_CONFIG_3[] = {0x11, 0x11, 0x03, 0x00, 0x01, SYNC_31_24, SYNC_23_16};

//包的整体配置
unsigned char const RF_PKT_CONFIG1_1[] = {0x11, 0x12, 0x01, 0x06, 0x81}; //域分开

//波特率配置
//unsigned char const RF_MODEM_MOD_TYPE_12[] = {0x11, 0x20, 0x0A, 0x00, 0x03, 0x00, 0x07, 0x06, 0x1A, 0x80, 0x05, 0xC9, 0xC3, 0x80};
unsigned char const RF_MODEM_MOD_TYPE_12[] = {0x11, 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x0C, 0x35, 0x00, 0x09, 0xC9, 0xC3, 0x80, 0x00, 0x05};
//频偏配置
unsigned char const RF_MODEM_FREQ_DEV_0_1[] = {0x11, 0x20, 0x03, 0x0A, 0x00, 0x06, 0x76};

//RSSI配置
unsigned char const RF_MODEM_RSSI_CONTROL_3[] = {0x11, 0x20, 0x03, 0x4C, 0x31, 0x00, 0x20};

//基准频率和步进配置
unsigned char const RF_FREQ_CONTROL_INTE_8[] = {0x11, 0x40, 0x08, 0x00, 0x3D, 0x0E, 0xEE, 0xEE, 0x36, 0x9D, 0x20, 0xFE};

//PA设置
unsigned char const RF_PA_TC_1[] = {0x11, 0x22, 0x03, 0x01, 0x7f, 0x00, 0x1D};

/****************************************** 其它配置 **********************************************/
//unsigned char const RF_MODEM_TX_RAMP_DELAY_12[] = {0x11, 0x20, 0x0C, 0x18, 0x01, 0x80, 0x08, 0x03, 0x80, 0x00, 0x20, 0x20, 0x00, 0xE8, 0x01, 0x77};
unsigned char const RF_MODEM_TX_RAMP_DELAY_12[] = {0x11, 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0x80, 0x00, 0x20, 0x20, 0x00, 0xE8, 0x00, 0x5E};
//unsigned char const RF_MODEM_BCR_NCO_OFFSET_2_12[] = {0x11, 0x20, 0x0C, 0x24, 0x01, 0x5D, 0x86, 0x00, 0xAF, 0x02, 0xC2, 0x00, 0x04, 0x32, 0x80, 0x1D};
unsigned char const RF_MODEM_BCR_NCO_OFFSET_2_12[] = {0x11, 0x20, 0x0C, 0x24, 0x05, 0x76, 0x1A, 0x05, 0x72, 0x02, 0x00, 0x00, 0x00, 0x12, 0x81, 0x5E};
//unsigned char const RF_MODEM_AFC_LIMITER_1_3[] = {0x11, 0x20, 0x03, 0x30, 0x11, 0xEC, 0x80};
unsigned char const RF_MODEM_AFC_LIMITER_1_3[] = {0x11, 0x20, 0x03, 0x30, 0x01, 0xCD, 0xA0};
unsigned char const RF_MODEM_AGC_CONTROL_1[] = {0x11, 0x20, 0x01, 0x35, 0xE0};

//unsigned char const RF_MODEM_AGC_WINDOW_SIZE_3[] = {0x11, 0x20, 0x03, 0x38, 0x11, 0x52, 0x52};
unsigned char const RF_MODEM_AGC_WINDOW_SIZE_3[] = {0x11, 0x20, 0x03, 0x38, 0x11, 0x15, 0x15};
unsigned char const RF_MODEM_OOK_PDTC_4[] = {0x11, 0x20, 0x04, 0x40, 0x2A, 0x0C, 0xA4, 0x22};

//unsigned char const RF_MODEM_RAW_CONTROL_8[] = {0x11, 0x20, 0x05, 0x45, 0x83, 0x00, 0xDE, 0x02, 0x00, 0xFF, 0x06, 0x00};
unsigned char const RF_MODEM_RAW_CONTROL_8[] = {0x11, 0x20, 0x05, 0x45, 0x03, 0x00, 0xDE, 0x01, 0x00};
unsigned char const RF_MODEM_RAW_SEARCH2_2[] = {0x11, 0x20, 0x02, 0x50, 0x84, 0x0A};

unsigned char const RF_MODEM_SPIKE_DET_2[] = {0x11, 0x20, 0x02, 0x54, 0x03, 0x07};

unsigned char const RF_MODEM_RSSI_MUTE_1[] = {0x11, 0x20, 0x01, 0x57, 0x00};

//unsigned char const RF_MODEM_DSA_CTRL1_5[] = {0x11, 0x20, 0x05, 0x5B, 0x42, 0x04, 0x07, 0x78, 0x20};
unsigned char const RF_MODEM_DSA_CTRL1_5[] = {0x11, 0x20, 0x05, 0x5B, 0x40, 0x04, 0x06, 0x78, 0x20};
//unsigned char const RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12[] = {0x11, 0x21, 0x0C, 0x00, 0xCC, 0xA1, 0x30, 0xA0, 0x21, 0xD1, 0xB9, 0xC9, 0xEA, 0x05, 0x12, 0x11};
unsigned char const RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12[] = {0x11, 0x21, 0x0C, 0x00, 0xA2, 0x81, 0x26, 0xAF, 0x3F, 0xEE, 0xC8, 0xC7, 0xDB, 0xF2, 0x02, 0x08};
//unsigned char const RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12[] = {0x11, 0x21, 0x0C, 0x0C, 0x0A, 0x04, 0x15, 0xFC, 0x03, 0x00, 0xCC, 0xA1, 0x30, 0xA0, 0x21, 0xD1};
unsigned char const RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12[] = {0x11, 0x21, 0x0C, 0x0C, 0x07, 0x03, 0x15, 0xFC, 0x0F, 0x00, 0xA2, 0x81, 0x26, 0xAF, 0x3F, 0xEE};
//unsigned char const RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12[] = {0x11, 0x21, 0x0C, 0x18, 0xB9, 0xC9, 0xEA, 0x05, 0x12, 0x11, 0x0A, 0x04, 0x15, 0xFC, 0x03, 0x00};
unsigned char const RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12[] = {0x11, 0x21, 0x0C, 0x18, 0xC8, 0xC7, 0xDB, 0xF2, 0x02, 0x08, 0x07, 0x03, 0x15, 0xFC, 0x0F, 0x00};
unsigned char const RF_SYNTH_PFDCP_CPFF_7[] = {0x11, 0x23, 0x07, 0x00, 0x2C, 0x0E, 0x0B, 0x04, 0x0C, 0x73, 0x03};
/*******************************************************************************************************/
#ifdef Use_Rx_Hop

//跳频配置(频道不要太多，否则会影响接收)
unsigned char RF_RX_HOP_CONTROL_12[] = {0x11, 0x50, 0x0C, 0x00, 0x10, 0x03, 4, 4, 4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char const RF_RX_HOP_TABLE_ENTRY_10_12[] = {0x11, 0x50, 0x0C, 0x0C, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char const RF_RX_HOP_TABLE_ENTRY_22_12[] = {0x11, 0x50, 0x0C, 0x18, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char const RF_RX_HOP_TABLE_ENTRY_34_12[] = {0x11, 0x50, 0x0C, 0x24, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char const RF_RX_HOP_TABLE_ENTRY_46_12[] = {0x11, 0x50, 0x0C, 0x30, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char const RF_RX_HOP_TABLE_ENTRY_58_6[] = {0x11, 0x50, 0x06, 0x3C, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#else
//跳频配置(频道不要太多，否则会影响接收)
unsigned char RF_RX_HOP_CONTROL_12[] = {0x11, 0x50, 0x0C, 0x00, 0x04, 0x40, 0x00, 0x27, 0x01, 0x28, 0x02, 0x29, 0x03, 0x2A, 0x04, 0x2B};
unsigned char const RF_RX_HOP_TABLE_ENTRY_10_12[] = {0x11, 0x50, 0x0C, 0x0C, 0x05, 0x2C, 0x06, 0x2D, 0x07, 0x2E, 0x08, 0x2F, 0x09, 0x30, 0x0A, 0x31};
unsigned char const RF_RX_HOP_TABLE_ENTRY_22_12[] = {0x11, 0x50, 0x0C, 0x18, 0x0B, 0x32, 0x0C, 0x33, 0x0D, 0x34, 0x0E, 0x35, 0x0F, 0x36, 0x10, 0x37};
unsigned char const RF_RX_HOP_TABLE_ENTRY_34_12[] = {0x11, 0x50, 0x0C, 0x24, 0x11, 0x38, 0x12, 0x39, 0x13, 0x3A, 0x14, 0x3B, 0x15, 0x3C, 0x16, 0x3D};
unsigned char const RF_RX_HOP_TABLE_ENTRY_46_12[] = {0x11, 0x50, 0x0C, 0x30, 0x17, 0x3E, 0x19, 0x3F, 0x1A, 0x40, 0x1B, 0x41, 0x1C, 0x43, 0x1D, 0x44};
unsigned char const RF_RX_HOP_TABLE_ENTRY_58_6[] = {0x11, 0x50, 0x06, 0x3C, 0x1E, 0x45, 0x1F, 0x46, 0x20, 0x47};
#endif

//包配置
unsigned char const pk_set1[] = {0x11, 0x12, 0x03, 0x08, 0x0a, 0x01, 0x00}; //Set variable packet length
//unsigned char const pk_set2[] = {0x11,0x12,0x04,0x0d,0x00,0x01,0x06,0xa2};		//Field1 is fix 1byte length,used for PKT_LEN
//unsigned char const pk_set3[] = {0x11,0x12,0x04,0x11,0x01,0x00,0x02,0x0a};		//Field2 is variable length,contains the actual payload
//120D-1220 TX
uint8_t pk_set2[] = {0x11, 0x12, 0x04, 0x0d, 0x00, 0x01, 0x06, 0xa2};
uint8_t pk_set3[] = {0x11, 0x12, 0x04, 0x11, 0x01, 0x00, 0x02, 0x22};
//1221-1234 RX
uint8_t pk_set4[] = {0x11, 0x12, 0x04, 0x21, 0x00, 0x01, 0x06, 0x82};
uint8_t pk_set5[] = {0x11, 0x12, 0x04, 0x25, 0x01, 0x00, 0x02, 0x0a};
unsigned char const pk_threshold[] = {0x11, 0x12, 0x02, 0x0b, TX_THRESHOLD, RX_THRESHOLD};
unsigned char const rssi_threshold[] = {0x11, 0x20, 0x01, 0x4a, 0x3f}; //Config RSSI THRESHOLD--> 63
unsigned char const pkt_crc_config[] = {0x11, 0x12, 0x01, 0x00, 0x04}; //PKT_CRC_CONFIG

//中断设置
unsigned char const Receive_Interrupt_set[] = {0x11, 0x01, 0x03, 0x00, 0x03, 0x19, 0x09};
unsigned char const Transmit_Interrupt_set[] = {0x11, 0x01, 0x02, 0x00, 0x01, 0x22, 0x00};
unsigned char const Interrupt_Close[] = {0x11, 0x01, 0x03, 0x00, 0x03, 0x00, 0x00};

//跳频关闭
unsigned char const RxHop_Close[] = {0x11, 0x50, 0x01, 0x00, 0x04};

unsigned char RF_GPIO_PIN_CFG[] = {0x13, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00};

void Si4438_Property_Init(void)
{
    //tocnt = 0x0040;

    Set_Property(RF_GLOBAL_XO_TUNE_2);     //0000 Configure crystal oscillator frequency tuning bank
    Set_Property(RF_GLOBAL_CONFIG_1);      //0003 Global configuration settings
    Set_Property(RF_INT_CTL_ENABLE_2);     //0100 Interrupt enable property
    Set_Property(RF_FRR_CTL_A_MODE_4);     //0200 Fast Response Register A Configuration
    Set_Property(RF_PREAMBLE_TX_LENGTH_1); //1000 Preamble length

    Set_Property(RF_SYNC_CONFIG_3); //1100 Sync configuration bits
    Set_Property(RF_PKT_CONFIG1_1); //1206 General packet configuration bits

    Set_Property(RF_MODEM_MOD_TYPE_12); //2000 Modulation Type

    Set_Property(RF_MODEM_FREQ_DEV_0_1);        //200c Byte 0 of frequency deviation.
    Set_Property(RF_MODEM_TX_RAMP_DELAY_12);    //
    Set_Property(RF_MODEM_BCR_NCO_OFFSET_2_12); //
    Set_Property(RF_MODEM_AFC_LIMITER_1_3);     //
    Set_Property(RF_MODEM_AGC_CONTROL_1);       //
    Set_Property(RF_MODEM_AGC_WINDOW_SIZE_3);   //
    Set_Property(RF_MODEM_OOK_PDTC_4);          //
    Set_Property(RF_MODEM_RAW_CONTROL_8);       //
    Set_Property(RF_MODEM_RSSI_CONTROL_3);      //204b RSSI jumping detection threshold.
    Set_Property(RF_MODEM_RAW_SEARCH2_2);       //

    Set_Property(RF_MODEM_SPIKE_DET_2);
    Set_Property(RF_MODEM_RSSI_MUTE_1);
    Set_Property(RF_MODEM_DSA_CTRL1_5);
    Set_Property(RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12);
    Set_Property(RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12);
    Set_Property(RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12);
    Set_Property(RF_PA_TC_1);             //2200 PA operating mode and groups.
    Set_Property(RF_SYNTH_PFDCP_CPFF_7);  //
    Set_Property(RF_FREQ_CONTROL_INTE_8); //4000 Frac-N PLL integer number.

    Set_Property(RF_RX_HOP_CONTROL_12); //Setup RF_RX_HOP
    Set_Property(RF_RX_HOP_TABLE_ENTRY_10_12);
    Set_Property(RF_RX_HOP_TABLE_ENTRY_22_12);
    Set_Property(RF_RX_HOP_TABLE_ENTRY_34_12);
    Set_Property(RF_RX_HOP_TABLE_ENTRY_46_12);
    Set_Property(RF_RX_HOP_TABLE_ENTRY_58_6);

    Set_Property(pk_set1);
    Set_Property(pk_set2);
    Set_Property(pk_set3);
    Set_Property(pk_set4);
    Set_Property(pk_set5);
    Set_Property(pk_threshold);

    Set_Property(rssi_threshold); //设置RSSI门槛
    Set_Property(pkt_crc_config);

    GPIO_PIN_CFG(RF_GPIO_PIN_CFG);
    //tocnt = 0x0020;
}
