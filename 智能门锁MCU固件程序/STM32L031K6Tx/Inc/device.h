#ifndef __DEVICE_H
#define __DEVICE_H
#include "stm32l0xx.h"

#include "stm32l0xx.h"
#include "protocol.h"


#define DOORLOCK_FRAME_STAET	0x55
#define DOORLOCK_FRAME_END		0xAA

/*
�豸��Ϣ
*/
typedef struct DeviceInfo_
{
	uint8_t mac_exist;		//��ʶ�豸MAC�Ƿ��Ѿ���¼
	uint8_t mac[8];			//�豸MAC��ַ
	uint8_t aes[16];		//��Կ
	uint8_t addr_DA;		//�߼���ַ
	uint8_t addr_GA[3];		//Ⱥ�ڵ�ַ
	uint8_t NA;				//2�ֽڶ��벹ȫ�ֽ�
}DeviceInfo_t;



//�������ڽ���״̬ö��
typedef enum Lock_uart_recv_state_
{
    LOCK_FRAME_HEAD,	//֡ͷ
	LOCK_FRAME_LEN,     //���ݳ���
	LOCK_FRAME_DATA,	//����
	LOCK_FRAME_FENISH,	//�������
	
}Lock_uart_recv_state_e;


//�����������ṹ
typedef struct DOOR_DATA_
{
	uint8_t ctrlType;		//�������01 ���ӣ�02 ɾ�� 03 ��֤ 04 ����
	uint8_t userType;       //�û����01 ָ�� 02 ���� 03 �� 04 �ֻ� 05 ����
    uint8_t door_ID[8];     //�� ID��0000000000000000~9999999999999999
    uint8_t userNum;        //�û���ţ�00~99 ���� ID ��ء�HEX ��
    uint8_t state;          //״̬��00 ʧ�ܣ�01 �ɹ�
}DOOR_DATA_t;
//����Э��֡�ṹ
typedef struct DOOR_CMD_
{
	uint8_t FrameHead;
	uint8_t DataLen;		//���������ֽ���
	uint8_t CmdFunc;		//��������(Ŀǰ�ǹ̶�0x31)
	DOOR_DATA_t dataPath;		//������
	uint8_t crc_sum;			//crc ����ʼ��crcǰ���У���
	uint8_t FrameEnd;           
}DOOR_CMD_t;


//����ע������ʱ�������ṹ(���ݱ�ʶ���������)
typedef struct LOCK_JOINNET_DATA_
{
	uint8_t version[2];		//�汾��
	
}LOCK_JOINNET_DATA__t;

//������ע������ʱ�������ṹ(���ݱ�ʶ���������)
typedef struct SENSOR_JOINNET_DATA_
{
//	uint8_t version[2];		//�汾��
//	uint8_t no_2;
//	uint8_t infrared:1;		//�����Ӧ	
//	uint8_t no_1:7;			//Ԥ��
//	uint8_t PM2_5:1;		//PM2.5
//	uint8_t HCHO:1;			//��ȩ
//	uint8_t CO2:1;			//������̼
//	uint8_t TVOC:1;			//�ܻӷ��л���
//	uint8_t humidity:1;		//ʪ��
//	uint8_t temperature:1;	//�¶�
//	uint8_t wind_power:1; 	//����
//	uint8_t rainfall:1;		//����

	uint8_t version[2];		//�汾��
	uint8_t byte2_0_3:4;	//Ԥ��
	uint8_t infrared:1;		//�����Ӧ	
	uint8_t doorLock:1;		//�Ŵ�
	uint8_t byte2_6_7:2;	//Ԥ��

	
	uint8_t byte1_0_1:2;			//Ԥ��
	uint8_t humidity:1;		//ʪ��
	uint8_t temperature:1;	//�¶�
	uint8_t wind_power:1; 	//����
	uint8_t rainfall:1;		//����
	uint8_t byte1_6_7:2;			//Ԥ��
	
	uint8_t PM2_5:1;		//PM2.5
	uint8_t HCHO:1;			//��ȩ
	uint8_t CO2:1;			//������̼
	uint8_t TVOC:1;			//�ܻӷ��л���
	uint8_t byte1_4_7:4;			//Ԥ��


	
	
}SENSOR_JOINNET_DATA_t;



extern DeviceInfo_t deviceInfo;


void DoorLockDataTask(void);
void DoorLockDataReadFromUart(uint8_t rec,UartRec_t *uartRecv);
void DeviceEventSend(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum);
void DeviceInfoInit(void);
void DeviceJoinNet(FRAME_CMD_t *frame_cmd);

#endif



