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


extern DeviceInfo_t deviceInfo;


void DoorLockDataTask(void);
void DoorLockDataReadFromUart(uint8_t rec,UartRec_t *uartRecv);
void DeviceEventSend(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum);
void DeviceInfoInit(void);
void DeviceJoinNet(FRAME_CMD_t *frame_cmd);

#endif



