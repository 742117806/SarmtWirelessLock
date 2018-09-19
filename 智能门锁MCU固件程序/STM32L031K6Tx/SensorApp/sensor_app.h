#ifndef __USER_APP1_H
#define __USER_APP1_H
#include "stm32l0xx_hal.h"
#include "frameType.h"

#define SENSOR_FRAME_STAET		0xAC
#define SENSOR_FRAME_END		0x53

//���������ͻ�����֡�������ݽṹ
typedef struct SENSOR_DATA_CMD_
{
    uint8_t frame_start;		//����ͷ
	uint8_t NC_6[6];			//6���޹�����
	uint8_t len;				//����
	uint8_t pm2_5_index[3];		//pm2_5��ʶ
	uint8_t pm2_5_data[3];		//pm2_5����
	uint8_t temp_index[3];		//�¶�
	uint8_t temp_data[3];
	uint8_t	hum_index[3];		//���ʪ��
	uint8_t hum_data[3];
	uint8_t crc16[2];
	uint8_t frame_end;			//���ݽ���		
}SENSOR_DATA_CMD_t;


//�������ڴ��������ݽṹ��
typedef struct SENSOR_UART_REV_
{
	uint8_t rev_len;			//���ճ���
	uint8_t rev_cnt:6;			//���ռ���
	uint8_t rev_start:1;		//��ʼ���ձ�־
	uint8_t rev_ok:1;			//������ɱ�־
	uint8_t buf[32];			//���ջ���
}SENSOR_UART_REV_t;

//�������ϱ����������ݽṹ
typedef struct SENSOR_UPLOAD_DATA_
{
	uint8_t num;			//����������
	uint8_t pm2_5_index[3];		//pm2_5��ʶ
	uint8_t pm2_5_data[3];		//pm2_5����
	uint8_t	hum_index[3];		//���ʪ��
	uint8_t hum_data[3];
	uint8_t temp_index[3];		//�¶�
	uint8_t temp_data[3];
}SENSOR_UPLOAD_DATA_t;


//������ע������ʱ�������ṹ(���ݱ�ʶ���������)
typedef struct SENSOR_JOINNET_DATA_
{
	uint8_t version[2];		//�汾��
	uint8_t no_2;
	uint8_t infrared:1;		//�����Ӧ	
	uint8_t no_1:7;			//Ԥ��
	uint8_t PM2_5:1;		//PM2.5
	uint8_t HCHO:1;			//��ȩ
	uint8_t CO2:1;			//������̼
	uint8_t TVOC:1;			//�ܻӷ��л���
	uint8_t humidity:1;		//ʪ��
	uint8_t temperature:1;	//�¶�
	uint8_t wind_power:1; 	//����
	uint8_t rainfall:1;		//����
	
}SENSOR_JOINNET_DATA_t;


extern const uint8_t version[2];

void SensorDataReadFromUart(uint8_t rec);
void SensorDataTask(void);
void SensorDataUpLoad(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum);
void SensorDataReadCmdSend(void);
void SensorDataReadTimer(void);
#endif


