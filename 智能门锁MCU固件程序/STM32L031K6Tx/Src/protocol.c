#include "protocol.h"
#include "crc16.h"
#include "uart.h"
#include "mcu_eeprom.h"
#include "device.h"
#include "string.h"
#include "74.h"

uint8_t lock_uart_buff[UART_RECV_BUFF_SIZE + 1];   //+1是为了在判断接收长度小于UART_RECV_BUFF_SIZE时候不产生数值越界

//应用通信协议缓存结构体变量
UartRec_t lpuart1Rec=
{
	.buff = lock_uart_buff,
	.state = LOCK_FRAME_HEAD, 
};


uint8_t mac_uart_buff[MAC_UART_RECV_BUFF_SIZE + 1];
//烧录设备MAC协议缓存结构体变量
UartRec_t MAC_UartRec=
{
   .buff = mac_uart_buff,
   .state = UartRx_FrameHead, 
};



//整合一帧符合协议的帧数据

uint8_t Frame_Compose(uint8_t *p)
{
    uint8_t ComposeFrame_Len;
    uint16_t crc16_val;


    p[Region_HeaderNumber] = HKFreamHeader;

    p[Region_CmdNumber] &= ~(1 << 5);
    ComposeFrame_Len = p[Region_DataLenNumber] + (Region_DataLenNumber + 1);
    crc16_val = CRC16_2(p, ComposeFrame_Len);
    p[ComposeFrame_Len] = crc16_val >> 8;
    ComposeFrame_Len++;
    p[ComposeFrame_Len] = crc16_val & 0xff;
    ComposeFrame_Len++;
    p[ComposeFrame_Len] = HKFreamEnd;
    ComposeFrame_Len++;

    return ComposeFrame_Len;
}
//给无线编码使用的校验函数
uint8_t Frame_Check(uint8_t *p, uint8_t Len)
{
    uint8_t ComposeFrame_Len;
    uint16_t crc16_val;

    p[Region_HeaderNumber] = HKFreamHeader;

    ComposeFrame_Len = Len;
    crc16_val = CRC16_2(p, ComposeFrame_Len);
    p[ComposeFrame_Len] = crc16_val >> 8;
    ComposeFrame_Len++;
    p[ComposeFrame_Len] = crc16_val & 0xff;
    ComposeFrame_Len++;
    p[ComposeFrame_Len] = HKFreamEnd;
    ComposeFrame_Len++;

    return ComposeFrame_Len;
}

/****************************************************************
**功   能：按照应用协议进行74编码(从一帧数据的用户数据区开始到帧结束符前面的数据)
**参   数：
        @srcData 源数据
        @srcLen 源数据的长度（字节数）
		@outLen	编码后一帧数据的长度
        @mode 1编码，0解码
**返回值:无
****************************************************************/
void FrameData_74Convert(FRAME_CMD_t *srcData,uint8_t srcLen,uint8_t *outLen,uint8_t mode)
{
	uint8_t frame_len;
	uint8_t temp[256]={0};
	uint16_t crc16;
	uint8_t *p_frame_data;
	
	if(mode==0)	//解码
	{
       	frame_len = _74DecodeBytes((uint8_t*)&srcData->userData,temp,srcLen-11);       //把编码过后加的CRC16(2个字节)去掉
	    temp[frame_len] = HKFreamEnd;
		memcpy((uint8_t*)&srcData->userData,temp,frame_len+1);
		*outLen = frame_len+9;		
	}
	else        //编码
	{
		frame_len = _74CodeBytes((uint8_t*)&srcData->userData,temp,srcLen-9);    //74编码
		
		memcpy((uint8_t*)&srcData->userData,temp,frame_len);					//把编码好的数据复制回原来数据的缓存区，
		crc16 = CRC16_2((uint8_t*)srcData,frame_len+8);			//编码后长度+协议帧前面没编码的8个字节帧数据
		p_frame_data =  (uint8_t*)&srcData->userData;
		p_frame_data[frame_len]= (crc16 >> 8);
		p_frame_data[frame_len+1]= (crc16 & 0x00ff);
		p_frame_data[frame_len+2] = HKFreamEnd;				                            //编码后加上帧结束0x53
		*outLen = frame_len+8+3;
	}
}




/*********************************************************************
**功  能：接收电脑串口发来的协议数据
**参  数：
        @ rx_data 串口接收到的一个字节数据 
        @ pu_buf 协议帧数据存储缓存指针
**返回值:无
*********************************************************************/
void  UpUart_RX_INT_Process(uint8_t rx_data, UartRec_t *pu_buf)
{
 	if (pu_buf->rec_ok == 0)
	{
		switch (pu_buf->state)
		{
		case UartRx_FrameHead:
			if (rx_data == AESFreamHeader)
			{
				pu_buf->buff[0] = rx_data;
				pu_buf->state = UartRx_AesCmd;
			}
			break;
		case UartRx_AesCmd:
			pu_buf->buff[1] = rx_data;
			pu_buf->state = UartRx_Aeslen;
			break;

		case UartRx_Aeslen:
			if (rx_data == 0)
			{
				pu_buf->buff[2] = 0;
				pu_buf->state = UartRx_AesFrameCs1;
				pu_buf->cnt = 3;
			}
			else if (rx_data > GK_Data_Len)
			{
				pu_buf->state = UartRx_FrameHead;
			}
			else
			{
				pu_buf->buff[2] = rx_data;
				pu_buf->state = UartRx_AesData;
				pu_buf->cnt = 3;
			}
			break;

		case UartRx_AesData:
			pu_buf->buff[pu_buf->cnt] = rx_data;
			if (++pu_buf->cnt >= (pu_buf->buff[2] + 3))
			{
				pu_buf->state = UartRx_AesFrameCs1;
			}

			break;

		case UartRx_AesFrameCs1:
			pu_buf->buff[pu_buf->cnt] = rx_data;
			pu_buf->cnt++;
			pu_buf->state = UartRx_AesFrameCs2;
			break;

		case UartRx_AesFrameCs2:
			pu_buf->buff[pu_buf->cnt] = rx_data;
			pu_buf->cnt++;
			pu_buf->state = UartRx_Finished;
			break;
		default:
			break;
		}
	
	}
	
}


/*
通过PC串口烧设备的MAC地址
recBuff:电脑发送过来的数据
例如：
烧录MAC地址
68 20 08 05 02 00 00 00 00 01 01 80 97
*/
void DeviceMAC_WriteProcess(uint8_t *recBuff,uint8_t len)
{
	uint8_t ack_temp[6]={0x68,0x21,0x01,0xFF,0x9E,0x0D};
	uint16_t crc=0;	
	WRITE_MAC_CMD_t *mac_cmd;
	
	mac_cmd = (WRITE_MAC_CMD_t*)recBuff;
	crc = CRC16_2(recBuff,len-2);		//一帧数据总长度减去2个字节的CRC
	if(crc == (recBuff[len-2]<<8)+recBuff[len-1])		//检验CRC
	{
		DeviceInfoInit();		//读取设备信息
		switch(mac_cmd->CmdFunc)
		{
			case 0x20://写mac
				//if(deviceInfo.mac_exist != 0x01)	//还没有MAC地址
				{
					deviceInfo.mac_exist = 0x01;				
					EEPROM_WriteBytes(EEPROM_MAC_ADDR,mac_cmd->mac,8);
					EEPROM_WriteBytes(EEPROM_MAC_EXSIT_ADDR,&deviceInfo.mac_exist,1);   //把标志位写到保存数据区
					
					DeviceInfoInit();		//读取设备信息
					if(memcmp(mac_cmd->mac,deviceInfo.mac,8)==0)
					{
						UartSendBytes(LPUART1,ack_temp,6);		//烧录成功应答
					}
					else
					{
						ack_temp[1]=0x22;   			//失败
						ack_temp[2]=0x01;
						ack_temp[3]=0x00;
						ack_temp[4]=0xDE;
						ack_temp[5]=0xBD;
						 
						UartSendBytes(LPUART1,ack_temp,6);		//烧录失败应答
					}
					
				}
				break;
			case 0x30://读mac
				//68 31 08 04 02 00 01 00 00 02 01 2C 3B
				mac_cmd->CmdFunc = 0x31;
				mac_cmd->DataLen = 8;
				memcpy(mac_cmd->mac,deviceInfo.mac,8);
				crc = CRC16_2((uint8_t*)mac_cmd,mac_cmd->DataLen+3);		//
				mac_cmd->crc[0]  = (uint8_t)(crc>>8);
				mac_cmd->crc[1]  = (uint8_t)crc&0x00ff;
				UartSendBytes(LPUART1,(uint8_t*)mac_cmd,mac_cmd->DataLen+5);
				break;
		}
	}
}




