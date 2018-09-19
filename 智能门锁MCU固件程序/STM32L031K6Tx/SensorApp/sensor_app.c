//实现与空气质量传感产品对接协议接口
/*
模块查询数据：
AC 00 00 00 00 00 00 03 03 FF FF 42 CA 53
环境监测仪回应数据：
(例如有PM2.5浓度是10.2ug/立方，环境温度28.5度，相对湿度80%)
AC 00 00 00 00 00 00 12 03 00 01 00 10 20 03 01 02 00 28 50 03 01 01 00 80 00 3C 81 53
*/



#include "user_app1.h"
#include "uart.h"
#include "frameType.h"
#include "string.h"
#include "crc16.h"
#include "wireless_app.h"
#include "frame_process.h"

const uint8_t version[2]={01,00};		//版本号

extern DeviceInfo_t deviecInfo;
SENSOR_UART_REV_t sensor_uart_rev={0,0,0,0,{0}};
uint8_t frameNum = 0;

//从串口读取传感器传来的数据
//rec ：串口数据
void SensorDataReadFromUart(uint8_t rec)
{

	if(sensor_uart_rev.rev_ok == 0)
	{
		rec = rec;
		if(rec == SENSOR_FRAME_STAET)
		{
			sensor_uart_rev.rev_cnt = 0;
			sensor_uart_rev.buf[sensor_uart_rev.rev_cnt++] = rec;
			sensor_uart_rev.rev_start = 1;		//开始接收
		}
		else if(sensor_uart_rev.rev_start == 1)
		{

			sensor_uart_rev.buf[sensor_uart_rev.rev_cnt++] = rec;
			
			if(sensor_uart_rev.rev_cnt == 29)//整帧数据长度
			{
				
				if(rec == SENSOR_FRAME_END)
				{
					sensor_uart_rev.rev_ok = 1;
					sensor_uart_rev.rev_len = sensor_uart_rev.rev_cnt;
					sensor_uart_rev.rev_cnt = 0;
					sensor_uart_rev.rev_start = 0;
				}
				else		// 重新接收
				{
					sensor_uart_rev.rev_start = 0;		//开始接收
					sensor_uart_rev.rev_cnt = 0;
				}

			}

			
		}
		
	}	
}
//传感器被动上报数据
//frame：上报命令帧数组指针
//eventDat：上报内容数组指针
//eventDatLen：上报内容长度
//frameNum：发送帧号
void SensorDataUpLoad(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum)
{
	
	uint16_t crc;
	uint8_t out_len,frame_len;		//加密后输出的长度
	
	
	frame->FameHead = HKFreamHeader;
	frame->addr_DA = deviecInfo.addr_DA;
	memcpy(frame->addr_GA,deviecInfo.addr_GA,3);

	
	frame->Ctrl.c_AFN = 7;		//控制功能码，0从站应答使用，1读后续帧数据
	frame->Ctrl.eventFlag = 1;	//事件标志，0普通帧，1事件帧
	frame->Ctrl.relayFlag = 1;	//中继标志，0本地帧，1转发帧
	frame->Ctrl.followUpFlag =0;		//后续帧标志，0无后续帧，1有后续帧
	frame->Ctrl.recAckFlag = 0;	//接收站接收应答标志，0正确应答，1异常应答
	frame->Ctrl.dir = 1;		//传送方向，0主站发出，1从站发出
	
	
	frame->FSQ.frameNum = frameNum&0x0F;	//帧序号（0-15）
	//frame->FSQ.encryptType = 0;		//不加密
	frame->FSQ.encryptType = 1;		//加密
	frame->FSQ.routeFlag = 0;		//不是路由功能
	frame->FSQ.ctrlField=0;			//控制域
	frame->DataLen = eventDatLen+4;	//数据长度中有1个数据功能和3个数据标识
	frame->userData.AFN = 0;				//数据功能码
	frame->userData.Index[0] = 0x03;		//数据标识（03 FF FF）传感器数据块标识
	frame->userData.Index[1] = 0xFF;
	frame->userData.Index[2] = 0xFF;
	memcpy(frame->userData.content,eventDat,eventDatLen);
	crc = CRC16_2((uint8_t*)frame,frame->DataLen+11-3);//一帧数据总长度减去2个字节的CRC和一个结束符
	frame->userData.content[eventDatLen] = (uint8_t)(crc>>8);
	frame->userData.content[eventDatLen+1] = (uint8_t)crc;
	frame->userData.content[eventDatLen+2] = HKFreamEnd;
	
	//memcpy(temp,(uint8_t*)frame,frame->DataLen+11);
	
	Encrypt_Convert((uint8_t*)frame, frame->DataLen+11, &out_len, 1);
	
	 
	
	//Si4438_Transmit_Start(&Wireless_Buf, Wireless_Channel[0],(uint8_t*)frame, frame->DataLen+11);
	
	Si4438_Transmit_Start(&Wireless_Buf, Wireless_Channel[0],(uint8_t*)frame, out_len);		//加密
	//DEBUG_Uart1SendBytes((uint8_t*)frame,frame->DataLen+11);
}


//传感器数据处理
void SensorDataTask(void)
{
	uint8_t frame_cmd[256];
	SENSOR_UPLOAD_DATA_t sensor_upload_data;
	SENSOR_DATA_CMD_t *pSensor_data;
	if(sensor_uart_rev.rev_ok)		//传感器对接串口接收到有效数据
	{
		pSensor_data = (SENSOR_DATA_CMD_t*)sensor_uart_rev.buf;
		//把传感器数据打包成上层需要的格式
		sensor_upload_data.num = 3;					//一共有3个传感器数据
		memcpy(sensor_upload_data.pm2_5_index,pSensor_data->pm2_5_index,6);		//6个数据包括传感器数标识和数据
		memcpy(sensor_upload_data.hum_index,pSensor_data->hum_index,6);			//6个数据包括传感器数标识和数据
		memcpy(sensor_upload_data.temp_index,pSensor_data->temp_index,6);		//6个数据包括传感器数标识和数据
		
		SensorDataUpLoad((FRAME_CMD_t*)frame_cmd,(uint8_t*)&sensor_upload_data,sizeof(sensor_upload_data),frameNum++);
		sensor_uart_rev.rev_ok = 0;
	}
}


//发送读取传感器数据串口命令
void SensorDataReadCmdSend(void)
{
	uint8_t cmd_buf[14]={0xAC,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0xFF,0xFF,0x42,0xCA,0x53};
	Uart1SendBytes(cmd_buf,14);
	
}


//定时查询传感器数据
void SensorDataReadTimer(void)
{
	static uint32_t new_t,old_t;		//当前时间，上次时间
	uint32_t v_t;						//时间间隔			
	new_t = HAL_GetTick();
	
	if(new_t>old_t)
	{
		v_t = new_t - old_t;
	}
	else
	{
		v_t = old_t - new_t;
	}
	if(v_t > 10000)			//10 秒
	{
		v_t = 0;
		old_t = new_t;
		SensorDataReadCmdSend();
	}
}





