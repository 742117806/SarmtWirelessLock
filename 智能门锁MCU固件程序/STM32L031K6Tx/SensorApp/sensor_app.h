#ifndef __USER_APP1_H
#define __USER_APP1_H
#include "stm32l0xx_hal.h"
#include "frameType.h"

#define SENSOR_FRAME_STAET		0xAC
#define SENSOR_FRAME_END		0x53

//传感器发送回来的帧命令数据结构
typedef struct SENSOR_DATA_CMD_
{
    uint8_t frame_start;		//数据头
	uint8_t NC_6[6];			//6个无关数据
	uint8_t len;				//长度
	uint8_t pm2_5_index[3];		//pm2_5标识
	uint8_t pm2_5_data[3];		//pm2_5数据
	uint8_t temp_index[3];		//温度
	uint8_t temp_data[3];
	uint8_t	hum_index[3];		//相对湿度
	uint8_t hum_data[3];
	uint8_t crc16[2];
	uint8_t frame_end;			//数据结束		
}SENSOR_DATA_CMD_t;


//结束串口传感器数据结构体
typedef struct SENSOR_UART_REV_
{
	uint8_t rev_len;			//接收长度
	uint8_t rev_cnt:6;			//接收计算
	uint8_t rev_start:1;		//开始接收标志
	uint8_t rev_ok:1;			//接收完成标志
	uint8_t buf[32];			//接收缓存
}SENSOR_UART_REV_t;

//传感器上报数据区数据结构
typedef struct SENSOR_UPLOAD_DATA_
{
	uint8_t num;			//传感器个数
	uint8_t pm2_5_index[3];		//pm2_5标识
	uint8_t pm2_5_data[3];		//pm2_5数据
	uint8_t	hum_index[3];		//相对湿度
	uint8_t hum_data[3];
	uint8_t temp_index[3];		//温度
	uint8_t temp_data[3];
}SENSOR_UPLOAD_DATA_t;


//传感器注册入网时数据区结构(数据标识后面的内容)
typedef struct SENSOR_JOINNET_DATA_
{
	uint8_t version[2];		//版本号
	uint8_t no_2;
	uint8_t infrared:1;		//红外感应	
	uint8_t no_1:7;			//预留
	uint8_t PM2_5:1;		//PM2.5
	uint8_t HCHO:1;			//甲醛
	uint8_t CO2:1;			//二氧化碳
	uint8_t TVOC:1;			//总挥发有机物
	uint8_t humidity:1;		//湿度
	uint8_t temperature:1;	//温度
	uint8_t wind_power:1; 	//风力
	uint8_t rainfall:1;		//雨量
	
}SENSOR_JOINNET_DATA_t;


extern const uint8_t version[2];

void SensorDataReadFromUart(uint8_t rec);
void SensorDataTask(void);
void SensorDataUpLoad(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum);
void SensorDataReadCmdSend(void);
void SensorDataReadTimer(void);
#endif


