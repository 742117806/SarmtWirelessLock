//ʵ��������������в�Ʒ�Խ�Э��ӿ�
/*
ģ���ѯ���ݣ�
AC 00 00 00 00 00 00 03 03 FF FF 42 CA 53
��������ǻ�Ӧ���ݣ�
(������PM2.5Ũ����10.2ug/�����������¶�28.5�ȣ����ʪ��80%)
AC 00 00 00 00 00 00 12 03 00 01 00 10 20 03 01 02 00 28 50 03 01 01 00 80 00 3C 81 53
*/



#include "user_app1.h"
#include "uart.h"
#include "frameType.h"
#include "string.h"
#include "crc16.h"
#include "wireless_app.h"
#include "frame_process.h"

const uint8_t version[2]={01,00};		//�汾��

extern DeviceInfo_t deviecInfo;
SENSOR_UART_REV_t sensor_uart_rev={0,0,0,0,{0}};
uint8_t frameNum = 0;

//�Ӵ��ڶ�ȡ����������������
//rec ����������
void SensorDataReadFromUart(uint8_t rec)
{

	if(sensor_uart_rev.rev_ok == 0)
	{
		rec = rec;
		if(rec == SENSOR_FRAME_STAET)
		{
			sensor_uart_rev.rev_cnt = 0;
			sensor_uart_rev.buf[sensor_uart_rev.rev_cnt++] = rec;
			sensor_uart_rev.rev_start = 1;		//��ʼ����
		}
		else if(sensor_uart_rev.rev_start == 1)
		{

			sensor_uart_rev.buf[sensor_uart_rev.rev_cnt++] = rec;
			
			if(sensor_uart_rev.rev_cnt == 29)//��֡���ݳ���
			{
				
				if(rec == SENSOR_FRAME_END)
				{
					sensor_uart_rev.rev_ok = 1;
					sensor_uart_rev.rev_len = sensor_uart_rev.rev_cnt;
					sensor_uart_rev.rev_cnt = 0;
					sensor_uart_rev.rev_start = 0;
				}
				else		// ���½���
				{
					sensor_uart_rev.rev_start = 0;		//��ʼ����
					sensor_uart_rev.rev_cnt = 0;
				}

			}

			
		}
		
	}	
}
//�����������ϱ�����
//frame���ϱ�����֡����ָ��
//eventDat���ϱ���������ָ��
//eventDatLen���ϱ����ݳ���
//frameNum������֡��
void SensorDataUpLoad(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum)
{
	
	uint16_t crc;
	uint8_t out_len,frame_len;		//���ܺ�����ĳ���
	
	
	frame->FameHead = HKFreamHeader;
	frame->addr_DA = deviecInfo.addr_DA;
	memcpy(frame->addr_GA,deviecInfo.addr_GA,3);

	
	frame->Ctrl.c_AFN = 7;		//���ƹ����룬0��վӦ��ʹ�ã�1������֡����
	frame->Ctrl.eventFlag = 1;	//�¼���־��0��ͨ֡��1�¼�֡
	frame->Ctrl.relayFlag = 1;	//�м̱�־��0����֡��1ת��֡
	frame->Ctrl.followUpFlag =0;		//����֡��־��0�޺���֡��1�к���֡
	frame->Ctrl.recAckFlag = 0;	//����վ����Ӧ���־��0��ȷӦ��1�쳣Ӧ��
	frame->Ctrl.dir = 1;		//���ͷ���0��վ������1��վ����
	
	
	frame->FSQ.frameNum = frameNum&0x0F;	//֡��ţ�0-15��
	//frame->FSQ.encryptType = 0;		//������
	frame->FSQ.encryptType = 1;		//����
	frame->FSQ.routeFlag = 0;		//����·�ɹ���
	frame->FSQ.ctrlField=0;			//������
	frame->DataLen = eventDatLen+4;	//���ݳ�������1�����ݹ��ܺ�3�����ݱ�ʶ
	frame->userData.AFN = 0;				//���ݹ�����
	frame->userData.Index[0] = 0x03;		//���ݱ�ʶ��03 FF FF�����������ݿ��ʶ
	frame->userData.Index[1] = 0xFF;
	frame->userData.Index[2] = 0xFF;
	memcpy(frame->userData.content,eventDat,eventDatLen);
	crc = CRC16_2((uint8_t*)frame,frame->DataLen+11-3);//һ֡�����ܳ��ȼ�ȥ2���ֽڵ�CRC��һ��������
	frame->userData.content[eventDatLen] = (uint8_t)(crc>>8);
	frame->userData.content[eventDatLen+1] = (uint8_t)crc;
	frame->userData.content[eventDatLen+2] = HKFreamEnd;
	
	//memcpy(temp,(uint8_t*)frame,frame->DataLen+11);
	
	Encrypt_Convert((uint8_t*)frame, frame->DataLen+11, &out_len, 1);
	
	 
	
	//Si4438_Transmit_Start(&Wireless_Buf, Wireless_Channel[0],(uint8_t*)frame, frame->DataLen+11);
	
	Si4438_Transmit_Start(&Wireless_Buf, Wireless_Channel[0],(uint8_t*)frame, out_len);		//����
	//DEBUG_Uart1SendBytes((uint8_t*)frame,frame->DataLen+11);
}


//���������ݴ���
void SensorDataTask(void)
{
	uint8_t frame_cmd[256];
	SENSOR_UPLOAD_DATA_t sensor_upload_data;
	SENSOR_DATA_CMD_t *pSensor_data;
	if(sensor_uart_rev.rev_ok)		//�������ԽӴ��ڽ��յ���Ч����
	{
		pSensor_data = (SENSOR_DATA_CMD_t*)sensor_uart_rev.buf;
		//�Ѵ��������ݴ�����ϲ���Ҫ�ĸ�ʽ
		sensor_upload_data.num = 3;					//һ����3������������
		memcpy(sensor_upload_data.pm2_5_index,pSensor_data->pm2_5_index,6);		//6�����ݰ�������������ʶ������
		memcpy(sensor_upload_data.hum_index,pSensor_data->hum_index,6);			//6�����ݰ�������������ʶ������
		memcpy(sensor_upload_data.temp_index,pSensor_data->temp_index,6);		//6�����ݰ�������������ʶ������
		
		SensorDataUpLoad((FRAME_CMD_t*)frame_cmd,(uint8_t*)&sensor_upload_data,sizeof(sensor_upload_data),frameNum++);
		sensor_uart_rev.rev_ok = 0;
	}
}


//���Ͷ�ȡ���������ݴ�������
void SensorDataReadCmdSend(void)
{
	uint8_t cmd_buf[14]={0xAC,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0xFF,0xFF,0x42,0xCA,0x53};
	Uart1SendBytes(cmd_buf,14);
	
}


//��ʱ��ѯ����������
void SensorDataReadTimer(void)
{
	static uint32_t new_t,old_t;		//��ǰʱ�䣬�ϴ�ʱ��
	uint32_t v_t;						//ʱ����			
	new_t = HAL_GetTick();
	
	if(new_t>old_t)
	{
		v_t = new_t - old_t;
	}
	else
	{
		v_t = old_t - new_t;
	}
	if(v_t > 10000)			//10 ��
	{
		v_t = 0;
		old_t = new_t;
		SensorDataReadCmdSend();
	}
}





