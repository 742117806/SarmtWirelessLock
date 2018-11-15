#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "stm32l0xx.h"



////////////////////////////////协议帧的宏定义/////////////////////////////////////////////////
#define HKFreamHeader                               0xAC
#define HKFreamEnd                                  0x53
//#define Transpond_Data_Main                         0x10
//#define Transpond_Data_Slave                        0x90


//HK各命令帧长度
#define LogicAddr_Len                               4
#define FrameCs_Len                                 2

#define HKData_LenMax                               244          //数据标识4+数据8MAX
#define HKFrame_LenMax                              (11+HKData_LenMax)     //头1+地址4+配置序号1+控制码1+数据长度1 +校验2+尾1





//帧各个域的访问定位
#define Region_HeaderNumber                 0          //帧头
#define Region_AddrNumber                   1          //帧地址
#define Region_SeqNumber                    5          //帧配置序号
#define Region_CmdNumber                    6          //帧命令
#define Region_DataLenNumber                7          //帧数据长度
#define Region_DataAFNNumber                8          //帧数据AFN
#define Region_DataIDNumber                 9          //帧数据标识
#define Region_DataValNumber                12          //信息字标识


//AES帧格式
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


//AES各命令帧长度
#define MAC_Data_Len                               8

#define GK_Data_Len                               (16*2)
#define GA_Data_Len                               3

#define AesData_LenMax                            16
#define AesFrame_LenMax                           (5+AesData_LenMax)  //头1+控制码1+数据长度1 +校验2

#define UART_RECV_BUFF_SIZE	256	//串口接收数据缓存大小
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




//串口烧录MAC命令
typedef struct WRITE_MAC_CMD_
{
	uint8_t FameHead;		// 帧头
	uint8_t CmdFunc;		//功能命令
	uint8_t DataLen;			//数据长度
	uint8_t mac[8];			//设备MAC
	uint8_t crc[2];
}WRITE_MAC_CMD_t;


//帧配置码
typedef struct FSQ_
{
	uint8_t frameNum:4;			//帧序号
	uint8_t ctrlField:1;		//控制域：0一个字节，1两个字节
	uint8_t encryptType:2;		//加密类型：0加密，1第一种加密，2第二种加密，3第三种加密
	uint8_t routeFlag:1;		//路由标志
}FSQ_t;

//帧控制码
typedef struct CTRL_
{
	uint8_t c_AFN:3;		//控制功能码，0从站应答使用，1读后续帧数据
	uint8_t eventFlag:1;	//事件标志，0普通帧，1事件帧
	uint8_t	relayFlag:1;	//中继标志，0本地帧，1转发帧
	uint8_t followUpFlag:1;	//后续帧标志，0无后续帧，1有后续帧
	uint8_t recAckFlag:1;	//接收站接收应答标志，0正确应答，1异常应答
	uint8_t dir:1;			//传送方向，0主站发出，1从站发出
}CTRL_t;

//协议帧数据区
typedef struct FRAME_DATA_
{
	uint8_t AFN;		//数据功能码
	uint8_t Index[3];	//数据标识
	uint8_t content[235];	//链路数据		
}FRAME_DATA_t;
//帧命令
typedef struct FRAME_CMD_
{
	uint8_t FameHead;		//帧头
	uint8_t addr_DA;		//逻辑地址
	uint8_t addr_GA[3];		//群众地址
	FSQ_t 	FSQ;			//帧配置码
	CTRL_t Ctrl;			//帧控制码
	uint8_t DataLen;		//数据长度
	FRAME_DATA_t userData;	//用户数据区
//帧命令			
	
}FRAME_CMD_t;
//设备入网链路数据区
typedef struct JOINE_NET_CMD_
{
	uint8_t mac[8];
	uint8_t aes[16];		//用在rsa函数来得出一个aes加密或解密的密钥
	
}JOINE_NET_CMD_t;


//接收协议帧数据状态
typedef enum
{
  UartRx_FrameHead = 0,      
  UartRx_AesCmd,           
  UartRx_Aeslen,           
  UartRx_AesData,             
  UartRx_AesFrameCs1,          
  UartRx_AesFrameCs2,          

  UartRx_Finished,          //接收完成
}UART_RxSta_TypDef;



extern UartRec_t lpuart1Rec;
extern UartRec_t MAC_UartRec;


void  UpUart_RX_INT_Process(uint8_t rx_data, UartRec_t *pu_buf);
uint8_t Frame_Compose(uint8_t *p);
void DeviceMAC_WriteProcess(uint8_t *recBuff,uint8_t len);
uint8_t Frame_Check(uint8_t *p, uint8_t Len);

#endif
