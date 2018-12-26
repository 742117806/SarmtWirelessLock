
#include "appTask.h"
#include "wireless_app.h"
#include "wireless_drv.h"
#include "delay.h"
#include <string.h>
#include "aes.h"
#include "crc16.h"
#include "device.h"
#include "74.h"
#include "uart.h"
#include "encrypt.h"
#include "myStr.h"

extern uint16_t sleep_delay_cnt;
static uint8_t wireless_tx_frame_num = 0;
static uint8_t wireless_tx_retry = 0;

//判断无线数据是否属于自己的数据
uint8_t WirelessDeviceAddrJudge(FRAME_CMD_t *frame)
{
	if((frame->addr_DA == deviceInfo.addr_DA)&&
		(frame->addr_GA[0] == deviceInfo.addr_GA[0])&&
	(frame->addr_GA[1] == deviceInfo.addr_GA[1])&&
	(frame->addr_GA[2] == deviceInfo.addr_GA[2]))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/*
无线接收数据处理
*/
uint8_t WirelessRxProcess(uint8_t *dat,uint8_t len)
{
	uint16_t index;		//数据标识
	FRAME_CMD_t *frame_cmd =(FRAME_CMD_t*)dat;
	uint16_t crc=0;		//根据接收数据计算出来的CRC16
	uint16_t crc_1=0;	//接收到的CRC16
	uint8_t out_len;		//解密后的长度
	
    if(frame_cmd->Ctrl.c_AFN == 0)
	{
		DEBUG_Printf("\r\n74Code");
		FrameData_74Convert((FRAME_CMD_t*)dat,len,&out_len,0); //解码
		len = out_len;		//解码后长度
	}
	
	if((frame_cmd->FSQ.encryptType == 2)||(frame_cmd->FSQ.encryptType == 1))		//是加密的
	{
		DEBUG_Printf("\r\nencryptType");
		Encrypt_Convert((uint8_t*)frame_cmd, len, &out_len, 0);         //解密
	}
	else		//不加密
	{
		DEBUG_Printf("\r\nno encryptType");
	}
	
	crc = CRC16_2(dat,frame_cmd->DataLen+11-3);		//一帧数据总长度减去2个字节的CRC和一个结束符
	
	crc_1 = (dat[frame_cmd->DataLen+11-3]<<8)+dat[frame_cmd->DataLen+11-2];
	
	if(crc != crc_1)		//检验CRC
	{
		DEBUG_Printf("\r\nCRC16_ERROR!!");
		return 0;		//CRC16错误
	}
	
	if(frame_cmd->Ctrl.eventFlag == 0)			//普通帧
	{
		index = (uint16_t)(frame_cmd->userData.Index[1]<<8)+frame_cmd->userData.Index[2];
		switch(frame_cmd->userData.Index[0])
		{
			case 0xFF:	//初始化数据标识
				switch(index)
				{
					case 0xFFFF:			//设备入网
						DeviceJoinNet(frame_cmd);
						
						break;
					case 0xFFFE:
						DEBUG_Printf("\r\n0xFFFE");
						break;
					default:
						break;
				}
				break;
			case 0x00:	//状态量数据标识
				break;
			case 0x01:	//控制量数据标识
				break;
			case 0x02:	//电器功能控制标识
				break;
			case 0x03:	//传感器类数据标识
				switch(index)
				{
					case 0xFFFF:			//读取传感器数据
						DEBUG_Printf("\r\nsensor data read!");
						#ifdef DEVICE_TYPE_SENSOR
						SensorDataReadCmdSend();
						#endif
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
		if(frame_cmd->Ctrl.c_AFN)
		{
			DEBUG_Printf("123456");
		}
	}
	else		//事件帧
	{
		DEBUG_SendBytes((uint8_t*)frame_cmd,frame_cmd->DataLen+11);
		if((frame_cmd->addr_DA == deviceInfo.addr_DA)&&(frame_cmd->FSQ.frameNum == (wireless_tx_frame_num&0x0f)))
		{
			wireless_tx_retry = 0; 
		}
	}
	return 1;	
}

/*
无线数据处理任务
*/
void WirelessTask(void)
{
    if (WIRELESS_STATUS == Wireless_RX_Finish)
    {
	     sleep_delay_cnt = 1;		//复位延迟睡眠计算
        DEBUG_SendBytes(Wireless_Buf.Wireless_RxData, Wireless_Buf.Wireless_PacketLength);
        WirelessRxProcess(Wireless_Buf.Wireless_RxData,Wireless_Buf.Wireless_PacketLength);   //无线数据处理
		
		if(WIRELESS_STATUS == Wireless_RX_Finish)
        {
			Si4438_Receive_Start(Wireless_Channel[0]);
		}
     //Si4438_Transmit_Start(&Wireless_Buf,Wireless_Channel[0],"RX_OK\r\n",6);		//应答
    }
    if (WIRELESS_STATUS == Wireless_TX_Finish)
    {
        DEBUG_Printf("\r\nWireless_TX_Finish\r\n");
        Si4438_Receive_Start(Wireless_Channel[0]); //开始接收无线数据
    }
    else if (WIRELESS_STATUS == Wireless_RX_Failure)
    {
        delay_ms(200);
        Si4438_Receive_Start(Wireless_Channel[0]); //开始接收无线数据
    }		
}

/* 串口协议处理 */
uint16_t UartMsgProcess(uint8_t *msg)
{
	uint8_t xResult;
	uint16_t len_valid = 0;		//有效长度
	char* token;
	char hex_str[2]={0};
	uint16_t i = 0;
	uint16_t j = 0;
	int hex_int;

	xResult = strncmp((const char*)msg,"AT+NMGS=",strlen("AT+NMGS="));
	if(xResult == 0)		//字符比较后相同
	{
	    //DEBUG_SendStr("987654321");
		token = strtok((char*)msg,"=");
		//DEBUG_Printf("%s\r\n",token);        //AT+NMGS
		
		token = strtok(NULL,",");
		//DEBUG_Printf("%s\r\n",token);		//有效数据长度
		len_valid = atoi(token);
		//DEBUG_Printf("len_valid = %d\r\n",len_valid);

		token = strtok(NULL,",");
		//DEBUG_Printf("%s\r\n",token);		//有效数据
		
		for(i=0; i<len_valid; i++)
		{
		    hex_str[0]= *token++;
			hex_str[1] = *token++;
			hex_int = hex2dec(hex_str);
			msg[j++] = (uint8_t)hex_int;
			//DEBUG_Printf("%c",hex_int) ;
			
		}
		//DEBUG_SendBytes(msg,len_valid);
		
	}
	return len_valid;
}

/* 组合一帧数据*/
uint8_t Frame_Compose_ext(uint8_t *src,uint8_t len,FRAME_CMD_t *des)
{
	
	des->Ctrl.dir = 1;		//从站
	des->Ctrl.eventFlag = 1;	//事件
	des->Ctrl.relayFlag = 1;	//中继帧
	des->FSQ.encryptType = 0;
	des->DataLen = len+4;
	memcpy(des->userData.content,src,len);
	Frame_Compose((uint8_t*)des);
	
} 
/*
串口数据处理任务
*/
void UartTask(void)
{
    uint8_t i = 0;
	uint16_t delay = 800;
	DOOR_CMD_t *pDoor_cmd;
	FRAME_CMD_t frame;
	FRAME_CMD_t FrameData;
	
	
	
    if ((lpuart1Rec.timeOut > 30) && (lpuart1Rec.cnt > 0))
    {
		wireless_tx_frame_num ++;
		lpuart1Rec.timeOut = 0;
		sleep_delay_cnt = 1;		//复位延迟睡眠计算		
		lpuart1Rec.cnt = UartMsgProcess(lpuart1Rec.buff);
		FrameData.addr_DA = deviceInfo.addr_DA;
		
		memcpy(FrameData.addr_GA,deviceInfo.addr_GA,3); 
		FrameData.Ctrl.dir = 1;		//从站
		FrameData.Ctrl.eventFlag = 1;	//事件
		FrameData.Ctrl.relayFlag = 1;	//中继
		FrameData.Ctrl.c_AFN = 7;
		FrameData.FSQ.encryptType = 0;   //不加密
		FrameData.FSQ.frameNum = wireless_tx_frame_num&0x0f;
		FrameData.userData.AFN = 0;
		FrameData.userData.Index[0]= 0x03;
		FrameData.userData.Index[1]= 0xFE;
		FrameData.userData.Index[2]= 0x02;
		FrameData.DataLen = lpuart1Rec.cnt+4;
		memcpy(FrameData.userData.content,lpuart1Rec.buff,lpuart1Rec.cnt);

		lpuart1Rec.cnt = Frame_Compose((uint8_t*)&FrameData);

		DEBUG_SendBytes((uint8_t*)&FrameData,lpuart1Rec.cnt);
		
        wireless_tx_retry = 1;
		while(1)
		{

			sleep_delay_cnt = 1;
			delay ++;
			if(delay > 800) 
			{
				i++;
				delay = 0;
				
				if((wireless_tx_retry == 0)||(i>6))
				{
					break;
				}
				Si4438_Transmit_Start(&Wireless_Buf,Wireless_Channel[0],(uint8_t*)&FrameData,lpuart1Rec.cnt);
			}
			WirelessTask();   //无线数据处理
			HAL_Delay(1);
		}
		lpuart1Rec.cnt = 0;
		lpuart1Rec.timeOut = 0;
    }
	
	
    if (lpuart1Rec.state == LOCK_FRAME_FENISH)			//门锁协议数据接收完成
    {
	
		sleep_delay_cnt = 1;		//复位延迟睡眠计算
        lpuart1Rec.rec_ok = 0;
		lpuart1Rec.state = 0;
        DEBUG_SendBytes(lpuart1Rec.buff, lpuart1Rec.Len);
		//Si4438_Transmit_Start(&Wireless_Buf, Wireless_Channel[0], lpuart1Rec.buff, lpuart1Rec.Len);
		
		pDoor_cmd = (DOOR_CMD_t*)lpuart1Rec.buff;		
		DeviceEventSend(&frame,(uint8_t*)&pDoor_cmd->dataPath,pDoor_cmd->DataLen,wireless_tx_frame_num++);	//门锁命令上报事件数据（通过无线发送到上层）
    }
	else if(MAC_UartRec.state == UartRx_Finished)      //烧录MAC协议数据接收完成
	{
		sleep_delay_cnt = 1;		//复位延迟睡眠计算
		MAC_UartRec.state = UartRx_FrameHead;
		
		DeviceMAC_WriteProcess(MAC_UartRec.buff, MAC_UartRec.cnt);
		//DEBUG_SendBytes(MAC_UartRec.buff, MAC_UartRec.cnt);
	}
}


//等待设备MAC烧入
void Device_MAC_Init(void)
{
	uint32_t delay_cnt = 0;

	DeviceInfoInit();
	while(1)
	{
		if(deviceInfo.mac_exist == 0x01)
		{
			break;
		}
		else
		{
			if(MAC_UartRec.state == UartRx_Finished)      //烧录MAC协议数据接收完成
			{
				MAC_UartRec.state = UartRx_FrameHead;
				
				DeviceMAC_WriteProcess(MAC_UartRec.buff, MAC_UartRec.cnt);
				//DEBUG_SendBytes(lpuart1Rec.buff, lpuart1Rec.cnt);
			}
			delay_cnt ++;
			if(delay_cnt > 1500)
			{
                delay_cnt  = 0;
				UartSendData(LPUART1,0x0C);		//提示烧录MAC
			}
			delay_ms(1);
		}
	}
}

