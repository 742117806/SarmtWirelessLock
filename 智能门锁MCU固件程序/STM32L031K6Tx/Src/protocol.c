#include "protocol.h"
#include "crc16.h"
#include "uart.h"
#include "mcu_eeprom.h"
#include "device.h"
#include "string.h"
#include "74.h"

uint8_t lock_uart_buff[UART_RECV_BUFF_SIZE + 1];   //+1��Ϊ�����жϽ��ճ���С��UART_RECV_BUFF_SIZEʱ�򲻲�����ֵԽ��

//Ӧ��ͨ��Э�黺��ṹ�����
UartRec_t lpuart1Rec=
{
	.buff = lock_uart_buff,
	.state = LOCK_FRAME_HEAD, 
};


uint8_t mac_uart_buff[MAC_UART_RECV_BUFF_SIZE + 1];
//��¼�豸MACЭ�黺��ṹ�����
UartRec_t MAC_UartRec=
{
   .buff = mac_uart_buff,
   .state = UartRx_FrameHead, 
};



//����һ֡����Э���֡����

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
//�����߱���ʹ�õ�У�麯��
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
**��   �ܣ�����Ӧ��Э�����74����(��һ֡���ݵ��û���������ʼ��֡������ǰ�������)
**��   ����
        @srcData Դ����
        @srcLen Դ���ݵĳ��ȣ��ֽ�����
		@outLen	�����һ֡���ݵĳ���
        @mode 1���룬0����
**����ֵ:��
****************************************************************/
void FrameData_74Convert(FRAME_CMD_t *srcData,uint8_t srcLen,uint8_t *outLen,uint8_t mode)
{
	uint8_t frame_len;
	uint8_t temp[256]={0};
	uint16_t crc16;
	uint8_t *p_frame_data;
	
	if(mode==0)	//����
	{
       	frame_len = _74DecodeBytes((uint8_t*)&srcData->userData,temp,srcLen-11);       //�ѱ������ӵ�CRC16(2���ֽ�)ȥ��
	    temp[frame_len] = HKFreamEnd;
		memcpy((uint8_t*)&srcData->userData,temp,frame_len+1);
		*outLen = frame_len+9;		
	}
	else        //����
	{
		frame_len = _74CodeBytes((uint8_t*)&srcData->userData,temp,srcLen-9);    //74����
		
		memcpy((uint8_t*)&srcData->userData,temp,frame_len);					//�ѱ���õ����ݸ��ƻ�ԭ�����ݵĻ�������
		crc16 = CRC16_2((uint8_t*)srcData,frame_len+8);			//����󳤶�+Э��֡ǰ��û�����8���ֽ�֡����
		p_frame_data =  (uint8_t*)&srcData->userData;
		p_frame_data[frame_len]= (crc16 >> 8);
		p_frame_data[frame_len+1]= (crc16 & 0x00ff);
		p_frame_data[frame_len+2] = HKFreamEnd;				                            //��������֡����0x53
		*outLen = frame_len+8+3;
	}
}




/*********************************************************************
**��  �ܣ����յ��Դ��ڷ�����Э������
**��  ����
        @ rx_data ���ڽ��յ���һ���ֽ����� 
        @ pu_buf Э��֡���ݴ洢����ָ��
**����ֵ:��
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
ͨ��PC�������豸��MAC��ַ
recBuff:���Է��͹���������
���磺
��¼MAC��ַ
68 20 08 05 02 00 00 00 00 01 01 80 97
*/
void DeviceMAC_WriteProcess(uint8_t *recBuff,uint8_t len)
{
	uint8_t ack_temp[6]={0x68,0x21,0x01,0xFF,0x9E,0x0D};
	uint16_t crc=0;	
	WRITE_MAC_CMD_t *mac_cmd;
	
	mac_cmd = (WRITE_MAC_CMD_t*)recBuff;
	crc = CRC16_2(recBuff,len-2);		//һ֡�����ܳ��ȼ�ȥ2���ֽڵ�CRC
	if(crc == (recBuff[len-2]<<8)+recBuff[len-1])		//����CRC
	{
		DeviceInfoInit();		//��ȡ�豸��Ϣ
		switch(mac_cmd->CmdFunc)
		{
			case 0x20://дmac
				//if(deviceInfo.mac_exist != 0x01)	//��û��MAC��ַ
				{
					deviceInfo.mac_exist = 0x01;				
					EEPROM_WriteBytes(EEPROM_MAC_ADDR,mac_cmd->mac,8);
					EEPROM_WriteBytes(EEPROM_MAC_EXSIT_ADDR,&deviceInfo.mac_exist,1);   //�ѱ�־λд������������
					
					DeviceInfoInit();		//��ȡ�豸��Ϣ
					if(memcmp(mac_cmd->mac,deviceInfo.mac,8)==0)
					{
						UartSendBytes(LPUART1,ack_temp,6);		//��¼�ɹ�Ӧ��
					}
					else
					{
						ack_temp[1]=0x22;   			//ʧ��
						ack_temp[2]=0x01;
						ack_temp[3]=0x00;
						ack_temp[4]=0xDE;
						ack_temp[5]=0xBD;
						 
						UartSendBytes(LPUART1,ack_temp,6);		//��¼ʧ��Ӧ��
					}
					
				}
				break;
			case 0x30://��mac
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




