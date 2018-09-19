#include "74.h"
#include "uart.h"
#include <string.h>



uint16_t code16_74bit(uint8_t InData);
uint8_t decode16_74bit(uint16_t InData);
static uint8_t decode74(uint8_t InData);
static uint8_t code74(uint8_t Indata);



uint8_t code74(uint8_t InData)
{
  uint8_t temp;      
  
  temp = InData & 0xf0;               //输入的数据高4位有效
  temp |= 0x01;                       //最后不用的1位固定为1
  if(((temp >> 4) + (temp >> 5) + (temp >> 6)) & 0x01)  temp |= 0x08;   //P3
  if(((temp >> 4) + (temp >> 5) + (temp >> 7)) & 0x01)  temp |= 0x04;   //P2
  if(((temp >> 4) + (temp >> 6) + (temp >> 7)) & 0x01)  temp |= 0x02;   //P1
  return temp;
}




uint16_t code16_74bit(uint8_t InData)
{
  uint8_t temp_val;
  uint16_t verify_val = 0;
  
  temp_val = InData;           //默认取高4位
  verify_val = (uint16_t)(code74(temp_val) << 8);   //高字节
  temp_val = InData << 4;     //取低4位
  verify_val |= (uint16_t)code74(temp_val);   //或上低字节
  return verify_val;
}



uint8_t decode74(uint8_t InData)
{
  uint8_t temp;
  
  temp = InData;
  if(((temp >> 4) + (temp >> 5) + (temp >> 6) + (temp >> 3)) & 0x01)    //D0,D1,D2,P3有一个错
  {
    if(((temp >> 4) + (temp >> 5) + (temp >> 7) + (temp >> 2)) & 0x01)    //D0,D1有一个错
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //D0错
      {
        temp ^= 0x10;   //将D0取反
      }
      else    //D1错
      {
        temp ^= 0x20;   //将D1取反
      }
    }
    else  //D2,P3有一个错
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //D2错
      {
        temp ^= 0x40;   //将D2取反
      }
      else    //P3错
      {
        temp ^= 0x08;   //将P3取反
      }
    }
  }
  else  //D3,P2,P1错 或 全部正确
  {
    if(((temp >> 4) + (temp >> 5) + (temp >> 7) + (temp >> 2)) & 0x01)    //D3,P2有一个错
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //D3错
      {
        temp ^= 0x80;   //将D3取反
      }
      else    //P2错
      {
        temp ^= 0x04;   //将P2取反
      }
    }
    else  //P1错或全部正确
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //P1错
      {
        temp ^= 0x02;   //将P1取反
      }
    }
  }  
  return temp;
}


uint8_t decode16_74bit(uint16_t InData)
{
  uint8_t temp_val;
  uint8_t right_val;
  
  temp_val = (uint8_t)(InData >> 8);     //取出高字节
  right_val = decode74(temp_val) & 0xf0;
  temp_val = (uint8_t)InData;          //取出低字节 
  right_val |= decode74(temp_val) >> 4;
  return right_val;
}




/*
typedef struct
{
    uint8_t Na:1;
    uint8_t D3:1;
    uint8_t D2:1;
    uint8_t D1:1;
    uint8_t D0:1;
    uint8_t P3:1;
    uint8_t P2:1;
    uint8_t P1:1;  
}Sa;
typedef union 
{
 uint8_t INdata;
 Sa       bit;
}U_bit74;
uint16_t code16_74bit(uint8_t InData)
{
  uint16_t tmp16bit = 0;
  uint8_t tmpbuf = (InData&0xf0)>>4;  
  tmp16bit = code74(tmpbuf)<<7;
  tmpbuf = InData&0x0f;
  tmp16bit |= code74(tmpbuf);
  return tmp16bit;
}
uint8_t decode16_74bit(uint16_t InData)
{
  uint8_t returnbuf = 0; 
  uint8_t tempbuf = InData>>7; 
  returnbuf = ((decode74(tempbuf)<<1)&0xf0);
  tempbuf = InData&0x7f; 
  returnbuf |= ((decode74(tempbuf)>>3)&0x0f);
  return returnbuf;
}

static uint8_t code74(uint8_t InData)
{
  U_bit74 sbit74;
  sbit74.INdata = (InData&0x0f)<<3;
  sbit74.bit.P3 = sbit74.bit.D2+sbit74.bit.D1+sbit74.bit.D0;
  sbit74.bit.P2 = sbit74.bit.D3+sbit74.bit.D1+sbit74.bit.D0;
  sbit74.bit.P1 = sbit74.bit.D3+sbit74.bit.D2+sbit74.bit.D0;
  return sbit74.INdata;
}

static uint8_t decode74(uint8_t InData)
{
  U_bit74 sbit74;
  sbit74.INdata = InData&0x7f;
  if(sbit74.bit.P1==sbit74.bit.D3^sbit74.bit.D2^sbit74.bit.D0)// D0 D2 D3        D1,P2,P3未知
  {
    if(sbit74.bit.P2==sbit74.bit.D3^sbit74.bit.D1^sbit74.bit.D0)//D0 D2 D3,D1,P2       P3未知
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0);//没有错，
      else //P3错，把P3取反。
      {
        sbit74.bit.P3=~sbit74.bit.P3;
      } 
    }
    else //D0 D2 D3,P1,P3       P2,D1有错
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0)
      {
        sbit74.bit.P2=~sbit74.bit.P2;
      }
      else //D1错
      {
        sbit74.bit.D1=~sbit74.bit.D1;
      } 
    }
  }
  else////P2，P3,D1       有错：D0，D2，D3，P1
  {
    if(sbit74.bit.P2==sbit74.bit.D3^sbit74.bit.D1^sbit74.bit.D0)//D2或P1有错
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0)//P1错了，
      {
       sbit74.bit.P1 = ~sbit74.bit.P1;
      }
      else //D2错，把D2取反。
      {
        sbit74.bit.D2=~sbit74.bit.D2;
      } 
    }
    else //D0或D3有错。
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0)//D3有错。
      {
        sbit74.bit.D3=~sbit74.bit.D3;
      }
      else //D0错
      {
        sbit74.bit.D0=~sbit74.bit.D0;
      } 
    }
  }
  return sbit74.INdata;
}
*/

/****************************************************************
**函数功能：进行74编码一组数据
**参   数：
        @src 要进行编码的源数据指针
        @len 源数据的长度（字节数）
        @des 编码后的数据
**返回值:编码后的数据长度
****************************************************************/
uint8_t _74CodeBytes(uint8_t *src,uint8_t *des,uint8_t len)
{
	uint8_t i = 0;

	uint16_t out_data;	//编码后的数据
    uint8_t out_len = 0;
    
    for(i=0;i<len;i++)
    {
        out_data = code16_74bit(src[i]);
		des[i*2] = out_data>>8;
		des[i*2+1] = out_data&0Xff; 
    }
    out_len = len*2;
    return out_len;
}

/****************************************************************
**功   能：进行74解码一组数据
**参   数：
        @src 要进行解码的源数据指针
        @len 源数据的长度（字节数）
        @des 解码后的数据
**返回值:解码后的数据长度
****************************************************************/
uint8_t _74DecodeBytes(uint8_t *src,uint8_t *des,uint8_t len)
{
	uint8_t i = 0;

	uint16_t in_data;
    uint8_t out_len = 0;
    
    for(i=0;i<len;i++)
    {
	    in_data = (src[i*2]<<8)&0xff00;
		in_data |= src[i*2+1];
		des[i] = decode16_74bit(in_data);
    }
    out_len = len/2;
    return out_len;
}


//74(汉明码)编码解码测试
void _74Code_Test(void)
{
	#if 0
	uint8_t i = 0;
	uint8_t temp_s[32]={"0123456789"};//源数据
	uint8_t temp_d[32]={0}; //编码后
	uint8_t temp_j[32]={0};	//解码后
	uint16_t out_data;	//编码后的数据
	uint16_t in_data;
	
	for(i=0;i<10;i++)
	{
		out_data = code16_74bit(temp_s[i]);
		temp_d[i*2] = out_data>>8;
		temp_d[i*2+1] = out_data&0Xff;
	}
	
    temp_d[10] &= ~(1<<0);
	for(i=0;i<10;i++)
	{
	    in_data = (temp_d[i*2]<<8)&0xff00;
		in_data |= temp_d[i*2+1];
		temp_j[i] = decode16_74bit(in_data);
	}
	#else
	uint8_t i = 0;
	uint8_t src_data[32]={"123456789"};
	uint8_t des_data[32]={0};
	uint8_t des_data1[32]={0};
	uint8_t out_len,out_len1;
	
	out_len = _74CodeBytes(src_data,des_data,9);
	for(i=0;i<out_len;i++)
	{
		DEBUG_Printf("%02X ",des_data[i]); 
	}
	
	out_len1 = _74DecodeBytes(des_data,des_data1,out_len);
	DEBUG_Printf("\r\n");
	for(i=0;i<out_len1;i++)
	{
		DEBUG_Printf("%02X ",des_data1[i]); 
	}
	#endif
	
}






