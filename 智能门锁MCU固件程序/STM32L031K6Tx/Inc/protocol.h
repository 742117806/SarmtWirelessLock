#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "stm32l0xx.h"



////////////////////////////////Э��֡�ĺ궨��/////////////////////////////////////////////////
#define HKFreamHeader                               0xAC
#define HKFreamEnd                                  0x53
//#define Transpond_Data_Main                         0x10
//#define Transpond_Data_Slave                        0x90


//HK������֡����
#define LogicAddr_Len                               4
#define FrameCs_Len                                 2

#define HKData_LenMax                               244          //���ݱ�ʶ4+����8MAX
#define HKFrame_LenMax                              (11+HKData_LenMax)     //ͷ1+��ַ4+�������1+������1+���ݳ���1 +У��2+β1





//֡������ķ��ʶ�λ
#define Region_HeaderNumber                 0          //֡ͷ
#define Region_AddrNumber                   1          //֡��ַ
#define Region_SeqNumber                    5          //֡�������
#define Region_CmdNumber                    6          //֡����
#define Region_DataLenNumber                7          //֡���ݳ���
#define Region_DataAFNNumber                8          //֡����AFN
#define Region_DataIDNumber                 9          //֡���ݱ�ʶ
#define Region_DataValNumber                12          //��Ϣ�ֱ�ʶ


//AES֡��ʽ
#define AESFreamHeader                            0x68

#define MACWrite_Cmd_Request                      0x20 
#define MACWrite_Yes_Response                     0x21
#define MACWrite_NO_Response                      0x22

#define MACRead_Cmd_Request                      0x30 
#define MACRead_Yes_Response                     0x31
#define MACRead_NO_Response                      0x32

#define Encode_Cmd_Request                        0xC0
#define Encode_Yes_Response                       0x41
#define Encode_No_Response                        0x42

#define Decode_Cmd_Request                        0xD0
#define Decode_Yes_Response                       0x51
#define Decode_No_Response                        0x52

#define GAWrite_Cmd_Request                       0xA0
#define GAWrite_Yes_Response                      0xA1
#define GAWrite_NO_Response                       0xA2

#define GKWrite_Cmd_Request                       0xB0
#define GKWrite_Yes_Response                      0xB1
#define GKWrite_NO_Response                       0xB2

#define Secret_Key_Yes           									0x66

#define None_GA_Key                               0xAE
#define None_GK_Key                               0xBE
#define None_GAK_Key                              0xCE


//AES������֡����
#define MAC_Data_Len                               8

#define GK_Data_Len                               (16*2)
#define GA_Data_Len                               3

#define AesData_LenMax                            16
#define AesFrame_LenMax                           (5+AesData_LenMax)  //ͷ1+������1+���ݳ���1 +У��2

#define UART_RECV_BUFF_SIZE	256	//���ڽ������ݻ����С
#define MAC_UART_RECV_BUFF_SIZE	48

typedef struct UartRec_
{
	uint8_t *buff;      
	uint16_t Len;
	uint16_t cnt;
	uint8_t rec_ok;
	uint8_t state;
	uint8_t timeOut;
}UartRec_t;




//������¼MAC����
typedef struct WRITE_MAC_CMD_
{
	uint8_t FameHead;		// ֡ͷ
	uint8_t CmdFunc;		//��������
	uint8_t DataLen;			//���ݳ���
	uint8_t mac[8];			//�豸MAC
	uint8_t crc[2];
}WRITE_MAC_CMD_t;


//֡������
typedef struct FSQ_
{
	uint8_t frameNum:4;			//֡���
	uint8_t ctrlField:1;		//������0һ���ֽڣ�1�����ֽ�
	uint8_t encryptType:2;		//�������ͣ�0���ܣ�1��һ�ּ��ܣ�2�ڶ��ּ��ܣ�3�����ּ���
	uint8_t routeFlag:1;		//·�ɱ�־
}FSQ_t;

//֡������
typedef struct CTRL_
{
	uint8_t c_AFN:3;		//���ƹ����룬0��վӦ��ʹ�ã�1������֡����
	uint8_t eventFlag:1;	//�¼���־��0��ͨ֡��1�¼�֡
	uint8_t	relayFlag:1;	//�м̱�־��0����֡��1ת��֡
	uint8_t followUpFlag:1;	//����֡��־��0�޺���֡��1�к���֡
	uint8_t recAckFlag:1;	//����վ����Ӧ���־��0��ȷӦ��1�쳣Ӧ��
	uint8_t dir:1;			//���ͷ���0��վ������1��վ����
}CTRL_t;

//Э��֡������
typedef struct FRAME_DATA_
{
	uint8_t AFN;		//���ݹ�����
	uint8_t Index[3];	//���ݱ�ʶ
	uint8_t content[235];	//��·����		
}FRAME_DATA_t;
//֡����
typedef struct FRAME_CMD_
{
	uint8_t FameHead;		//֡ͷ
	uint8_t addr_DA;		//�߼���ַ
	uint8_t addr_GA[3];		//Ⱥ�ڵ�ַ
	FSQ_t 	FSQ;			//֡������
	CTRL_t Ctrl;			//֡������
	uint8_t DataLen;		//���ݳ���
	FRAME_DATA_t userData;	//�û�������
//֡����			
	
}FRAME_CMD_t;
//�豸������·������
typedef struct JOINE_NET_CMD_
{
	uint8_t mac[8];
	uint8_t aes[16];		//����rsa�������ó�һ��aes���ܻ���ܵ���Կ
	
}JOINE_NET_CMD_t;


//����Э��֡����״̬
typedef enum
{
  UartRx_FrameHead = 0,      
  UartRx_AesCmd,           
  UartRx_Aeslen,           
  UartRx_AesData,             
  UartRx_AesFrameCs1,          
  UartRx_AesFrameCs2,          

  UartRx_Finished,          //�������
}UART_RxSta_TypDef;



extern UartRec_t lpuart1Rec;
extern UartRec_t MAC_UartRec;


void  UpUart_RX_INT_Process(uint8_t rx_data, UartRec_t *pu_buf);
uint8_t Frame_Compose(uint8_t *p);
void DeviceMAC_WriteProcess(uint8_t *recBuff,uint8_t len);
uint8_t Frame_Check(uint8_t *p, uint8_t Len);

#endif
