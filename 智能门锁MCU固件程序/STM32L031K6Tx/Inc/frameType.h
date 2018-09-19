#ifndef __FRAMETYPE_H
#define __FRAMETYPE_H
#include "stm32l0xx.h"

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



/*
设备信息
*/
typedef struct DeviceInfo_
{
	uint8_t mac[8];			//设备MAC地址
	uint8_t aes[16];		//密钥
	uint8_t addr_DA;		//逻辑地址
	uint8_t addr_GA[3];		//群众地址
}DeviceInfo_t;


#endif


