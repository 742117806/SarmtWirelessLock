#ifndef __DEVICE_H
#define __DEVICE_H
#include "stm32l0xx.h"

#include "stm32l0xx.h"
#include "protocol.h"


#define DOORLOCK_FRAME_STAET	0x55
#define DOORLOCK_FRAME_END		0xAA

/*
设备信息
*/
typedef struct DeviceInfo_
{
	uint8_t mac_exist;		//标识设备MAC是否已经烧录
	uint8_t mac[8];			//设备MAC地址
	uint8_t aes[16];		//密钥
	uint8_t addr_DA;		//逻辑地址
	uint8_t addr_GA[3];		//群众地址
}DeviceInfo_t;



//门锁串口接收状态枚举
typedef enum Lock_uart_recv_state_
{
    LOCK_FRAME_HEAD,	//帧头
	LOCK_FRAME_LEN,     //数据长度
	LOCK_FRAME_DATA,	//数据
	LOCK_FRAME_FENISH,	//接收完成
	
}Lock_uart_recv_state_e;


//门锁数据区结构
typedef struct DOOR_DATA_
{
	uint8_t ctrlType;		//操作类别：01 增加，02 删除 03 验证 04 保留
	uint8_t userType;       //用户类别：01 指纹 02 密码 03 卡 04 手机 05 保留
    uint8_t door_ID[8];     //锁 ID：0000000000000000~9999999999999999
    uint8_t userNum;        //用户编号：00~99 和锁 ID 相关。HEX 码
    uint8_t state;          //状态：00 失败，01 成功
}DOOR_DATA_t;
//门锁协议帧结构
typedef struct DOOR_CMD_
{
	uint8_t FrameHead;
	uint8_t DataLen;		//数据区的字节数
	uint8_t CmdFunc;		//功能命令(目前是固定0x31)
	DOOR_DATA_t dataPath;		//数据区
	uint8_t crc_sum;			//crc 是起始到crc前面的校验和
	uint8_t FrameEnd;           
}DOOR_CMD_t;


//门锁注册入网时数据区结构(数据标识后面的内容)
typedef struct LOCK_JOINNET_DATA_
{
	uint8_t version[2];		//版本号
	
}LOCK_JOINNET_DATA__t;


extern DeviceInfo_t deviceInfo;


void DoorLockDataTask(void);
void DoorLockDataReadFromUart(uint8_t rec,UartRec_t *uartRecv);
void DeviceEventSend(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum);
void DeviceInfoInit(void);
void DeviceJoinNet(FRAME_CMD_t *frame_cmd);

#endif



