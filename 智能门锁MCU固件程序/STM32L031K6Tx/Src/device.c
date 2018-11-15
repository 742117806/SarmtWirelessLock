
//实现与智能门锁产品对接协议接口
/*
门锁数据格式
开门验证失败
55 0C 31 03 01 00 00 FC E7 77 B0 9E CF 01 00 0E AA 
55 0C 31 03 01 00 00 FC E7 77 B0 9E CF 01 00 0E AA 
55 0C 31 03 01 00 00 FC E7 77 B0 9E CF 01 00 0E AA 
55 0C 31 03 01 00 00 FC E7 77 B0 9E CF 01 00 0E AA 
开门验证成功
55 0C 31 03 01 00 00 FC E7 77 B0 9E CF 04 01 12 AA
*/




#include "uart.h"
#include "string.h"
#include "crc16.h"
#include "wireless_app.h"
#include "device.h"
#include "mcu_eeprom.h"
#include "encrypt.h"
#include "wireless_app.h"
#include "wireless_drv.h"
#include "74.h"

const uint8_t version[2]={01,00};		//版本号


DeviceInfo_t deviceInfo=
{
	.aes = {0xA3,0xA6,0x89,0x26,0xAF,0xA7,0x13,0x29,0x33,0x0A,0xB1,0xA2,0x15,0xF8,0xFB,0xDB},
	.addr_GA={0x00,0x2A,0x5B},
};

//密文初始化
void AES_Init(void)
{
	//计算出密文，存放在aes_w，供加解密用
	memcpy(&aes_out[2*RsaByte_Size],deviceInfo.aes,16);
	memcpy(&aes_out[3*RsaByte_Size],deviceInfo.addr_GA,3);
	
	Rsa_Decode(aes_out);  
	key_expansion(aes_out, aes_w);  
}



/*
读设备信息
*/
void DeviceInfoInit(void)
{
	EEPROM_ReadBytes(EEPROM_MAC_EXSIT_ADDR,(uint8_t*)&deviceInfo,sizeof(deviceInfo));
//	deviceInfo.addr_GA[0]=0x00;
//	deviceInfo.addr_GA[1]=0x2A;
//	deviceInfo.addr_GA[2]=0x5B;
}

//根据家庭组切换到新的固定通讯频道上

void Get_WireLessChannel(uint8_t *wire_chnel)
{
    uint32_t temp_val = deviceInfo.addr_GA[0] + deviceInfo.addr_GA[1] + deviceInfo.addr_GA[2];
    //uint32_t temp_val = 0x00 + 0x2A + 0x5B;
    //uint32_t temp_val;
    if (temp_val == 0)
    {
        wire_chnel[0] = Default_Channel;
        wire_chnel[1] = Default_Channel;
    }
    else
    {
        wire_chnel[0] = (temp_val & 0x1f) << 1; //共32个信道组，每个信道组有两个信道
        wire_chnel[1] = wire_chnel[0] + 1;
		if(wire_chnel[0] == Default_Channel)
		{
			wire_chnel[0] = wire_chnel[0] + 2;
			wire_chnel[1] = wire_chnel[0] + 1;
		} 
    }

    RF_RX_HOP_CONTROL_12[7] = Channel_Frequency_Index[wire_chnel[0]]; //放入群组的主频道
    RF_RX_HOP_CONTROL_12[8] = Channel_Frequency_Index[wire_chnel[1]]; //放入群组的备用频道
}



//从串口接收门锁发来的事件数据接收
//rec ：串口数据
void DoorLockDataReadFromUart(uint8_t rec,UartRec_t *uartRecv)
{

	//if (uartRecv->rec_ok == 0)
	{
		switch ((Lock_uart_recv_state_e)uartRecv->state)
		{
		case LOCK_FRAME_HEAD: //等待帧头
			if (rec == DOORLOCK_FRAME_STAET)
			{
				uartRecv->state = LOCK_FRAME_LEN;
				uartRecv->cnt = 0;
				uartRecv->buff[uartRecv->cnt++] = rec;
			}
			break;
		case LOCK_FRAME_LEN:
			uartRecv->Len = rec + 5; //帧数据长度+5=整一帧长度
			uartRecv->state = LOCK_FRAME_DATA;
			uartRecv->buff[uartRecv->cnt++] = rec;
			if(uartRecv->cnt > UART_RECV_BUFF_SIZE)			
			{
				uartRecv->state = LOCK_FRAME_HEAD;
			}
			break;
		case LOCK_FRAME_DATA:

			uartRecv->buff[uartRecv->cnt++] = rec;
			if(uartRecv->cnt > UART_RECV_BUFF_SIZE)	//接收数据长度大于缓存区大小
			{
			   uartRecv->state = LOCK_FRAME_HEAD;                    //重新接收数据
			}
			if (uartRecv->cnt >= uartRecv->Len)
			{
				if (rec == DOORLOCK_FRAME_END)
				{
					//uartRecv->rec_ok = 1;
					uartRecv->state = LOCK_FRAME_FENISH;	//接收完成
				}
				else
				{
					uartRecv->state = LOCK_FRAME_HEAD;
				}
			}
			break;
		default:
			break;
		}
	}	
}

/*
通过无线发送门锁设备事件
*/
void DeviceEventSend(FRAME_CMD_t*frame,uint8_t* eventDat,uint8_t eventDatLen,uint8_t frameNum)
{
	uint16_t crc;
	uint8_t out_len;
    uint8_t frameLen; 

	 
	frame->FameHead = HKFreamHeader;
	frame->addr_DA = deviceInfo.addr_DA;
	memcpy(frame->addr_GA,deviceInfo.addr_GA,3);
	frame->Ctrl.dir = 1;				//从站发出
	frame->Ctrl.followUpFlag=0;		//无后续帧
	frame->Ctrl.recAckFlag = 0;	//应答结果
	frame->Ctrl.relayFlag = 1;		//无中继
	frame->Ctrl.eventFlag = 1;		//普通帧
	frame->Ctrl.c_AFN = 0;			//带74编码
	frame->FSQ.frameNum = frameNum&0x0f;	//帧序号（0-15）
	frame->FSQ.encryptType = 1;		//加密
	frame->FSQ.routeFlag = 0;		//不是路由功能
	frame->FSQ.ctrlField=0;			//控制域

	frame->DataLen = eventDatLen+4;	//数据长度中有1个数据功能和3个数据标识
	frame->userData.AFN = 0x80;				//数据功能码
	frame->userData.Index[0] = 0x04;
	frame->userData.Index[1] = 0x00;
	frame->userData.Index[2] = 0x01;
	memcpy(frame->userData.content,eventDat,eventDatLen);
	crc = CRC16_2((uint8_t*)frame,frame->DataLen+11-3);//一帧数据总长度减去2个字节的CRC和一个结束符
	frame->userData.content[eventDatLen] = (uint8_t)(crc>>8);
	frame->userData.content[eventDatLen+1] = (uint8_t)crc;
	frame->userData.content[eventDatLen+2] = HKFreamEnd;
	
	frameLen =  frame->DataLen+11;
	//Encrypt_Convert((uint8_t*)frame, frameLen, &out_len, 1);		//加密
	//frameLen = out_len;											//把加密后的数据长度赋值给原来数据的长度
	//FrameData_74Convert((FRAME_CMD_t*)frame,frameLen,&out_len,1); //编码
	//frameLen = out_len;		//编码后长度		
	
	Si4438_Transmit_Start(&Wireless_Buf, Wireless_Channel[0],(uint8_t*)frame,frameLen);
}




/*
无线应答
*/
void WirelessRespoint(FRAME_CMD_t*frame,FRAME_CMD_t* respoint,uint8_t result,uint8_t *datPath,uint8_t datPath_len)
{

    static uint8_t frameNum = 0;
	uint16_t crc;
	uint8_t out_len;
	uint8_t frameLen;

	frameNum ++;
	respoint->FameHead = HKFreamHeader;
	respoint->addr_DA = deviceInfo.addr_DA;
	memcpy(respoint->addr_GA,deviceInfo.addr_GA,3);
	respoint->Ctrl.dir = 1;				//从站发出
	respoint->Ctrl.followUpFlag=0;		//无后续帧
	respoint->Ctrl.recAckFlag = result;	//应答结果
	respoint->Ctrl.relayFlag = 1;		//无中继
	respoint->Ctrl.eventFlag = 0;		//普通帧
	respoint->Ctrl.c_AFN = 0;			//有74编码
	respoint->DataLen = datPath_len+4;	//数据长度中有1个数据功能和3个数据标识
	respoint->userData.AFN = frame->userData.AFN;
	respoint->FSQ.frameNum = frame->FSQ.frameNum;
	respoint->FSQ.encryptType = 1;
	respoint->FSQ.routeFlag = 0;
	respoint->FSQ.ctrlField=frame->FSQ.ctrlField;
	memcpy(respoint->userData.Index,frame->userData.Index,3);
	memcpy(respoint->userData.content,datPath,respoint->DataLen);
	crc = CRC16_2((uint8_t*)respoint,respoint->DataLen+11-3);//一帧数据总长度减去2个字节的CRC和一个结束符
	respoint->userData.content[datPath_len] = (uint8_t)(crc>>8);
	respoint->userData.content[datPath_len+1] = (uint8_t)crc;
	respoint->userData.content[datPath_len+2]=HKFreamEnd;
	
	frameLen =  respoint->DataLen+11;
	

	
	Encrypt_Convert((uint8_t*)respoint, frameLen, &out_len, 1);	//加密
	frameLen = out_len;											//把加密后的数据长度赋值给原来数据的长度
	
	FrameData_74Convert((FRAME_CMD_t*)respoint,frameLen,&out_len,1); //编码
	frameLen = out_len;		//编码后长度
	Si4438_Transmit_Start(&Wireless_Buf,Default_Channel, (uint8_t*)respoint, frameLen);	

	
	
	
}


/*
设备入网
dat:mac+aes
*/
void DeviceJoinNet(FRAME_CMD_t *frame_cmd)
{
	FRAME_CMD_t repoint;
	uint16_t i=0;
	uint8_t ret;
	JOINE_NET_CMD_t *joine_net_cmd;
	LOCK_JOINNET_DATA__t lock_joinnet_repoint_content;

	joine_net_cmd = (JOINE_NET_CMD_t*)frame_cmd->userData.content;
	ret = memcmp(joine_net_cmd->mac,deviceInfo.mac,8);			//比较自身MAC于接收到的MAC是否一样
	if(ret == 0)		//相等
	{
		ret = memcmp(deviceInfo.aes,joine_net_cmd->aes,16);
		if(ret != 0)		//设备的aes与核心板发过来的aes不相等
		{
			memcpy(deviceInfo.aes,joine_net_cmd->aes,16);			//获取设备的aes
			DEBUG_Printf("\r\n***********deviceInfo.aes******\r\n");
			for (i = 0; i < sizeof(deviceInfo.aes); i++)
			{
				DEBUG_Printf("%02X ", deviceInfo.aes[i]);
			}
			EEPROM_WriteBytes(EEPROM_AES_ADDR,(uint8_t*)deviceInfo.aes,16);		//保存设备信息 aes长度
		}
		else
		{
			DEBUG_Printf("\r\nDevice AES Not Cheng");
		}

		if((deviceInfo.addr_DA != frame_cmd->addr_DA)||
		(deviceInfo.addr_GA[0]!= frame_cmd->addr_GA[0])||
		(deviceInfo.addr_GA[1]!= frame_cmd->addr_GA[1])||
		(deviceInfo.addr_GA[2]!= frame_cmd->addr_GA[2]))
		{
			DEBUG_Printf("\r\nGroup Is Cheng");
			deviceInfo.addr_DA=frame_cmd->addr_DA;					//设备逻辑地址
			memcpy(deviceInfo.addr_GA,frame_cmd->addr_GA,3);		//设备群组地址
			EEPROM_WriteBytes(EEPROM_DA_ADDR,&deviceInfo.addr_DA,4);		//保存设备信息 addr_DA+addr_GA长度
			Get_WireLessChannel(Wireless_Channel);	//获取无线通道号
			
			Wireless_Init();
		}
		else
		{
			DEBUG_Printf("\r\nGroup Not Cheng");
		}
		
		

		
		//计算出密文，存放在aes_w，供加解密用
		//AES_Init(); 
		lock_joinnet_repoint_content.version[0]= version[0];
		lock_joinnet_repoint_content.version[1]= version[1];
		WirelessRespoint(frame_cmd,&repoint,0x00,(uint8_t*)&lock_joinnet_repoint_content,sizeof(lock_joinnet_repoint_content));
		
		

	}
	else
	{
		DEBUG_Printf("\r\nMAC Error");
		DEBUG_Printf("\r\n*********Recv MAC********\r\n");
		for (i = 0; i < 8; i++)
		{
		    DEBUG_Printf("%02X ", joine_net_cmd->mac[i]);
		}
		
		
		DEBUG_Printf("\r\n*********Device MAC********\r\n");
		for (i = 0; i <8; i++)
		{
		    DEBUG_Printf("%02X ", deviceInfo.mac[i]);
		}
		
		
	}
}

