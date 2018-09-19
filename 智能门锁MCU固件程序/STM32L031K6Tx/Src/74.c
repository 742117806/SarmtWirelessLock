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
  
  temp = InData & 0xf0;               //��������ݸ�4λ��Ч
  temp |= 0x01;                       //����õ�1λ�̶�Ϊ1
  if(((temp >> 4) + (temp >> 5) + (temp >> 6)) & 0x01)  temp |= 0x08;   //P3
  if(((temp >> 4) + (temp >> 5) + (temp >> 7)) & 0x01)  temp |= 0x04;   //P2
  if(((temp >> 4) + (temp >> 6) + (temp >> 7)) & 0x01)  temp |= 0x02;   //P1
  return temp;
}




uint16_t code16_74bit(uint8_t InData)
{
  uint8_t temp_val;
  uint16_t verify_val = 0;
  
  temp_val = InData;           //Ĭ��ȡ��4λ
  verify_val = (uint16_t)(code74(temp_val) << 8);   //���ֽ�
  temp_val = InData << 4;     //ȡ��4λ
  verify_val |= (uint16_t)code74(temp_val);   //���ϵ��ֽ�
  return verify_val;
}



uint8_t decode74(uint8_t InData)
{
  uint8_t temp;
  
  temp = InData;
  if(((temp >> 4) + (temp >> 5) + (temp >> 6) + (temp >> 3)) & 0x01)    //D0,D1,D2,P3��һ����
  {
    if(((temp >> 4) + (temp >> 5) + (temp >> 7) + (temp >> 2)) & 0x01)    //D0,D1��һ����
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //D0��
      {
        temp ^= 0x10;   //��D0ȡ��
      }
      else    //D1��
      {
        temp ^= 0x20;   //��D1ȡ��
      }
    }
    else  //D2,P3��һ����
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //D2��
      {
        temp ^= 0x40;   //��D2ȡ��
      }
      else    //P3��
      {
        temp ^= 0x08;   //��P3ȡ��
      }
    }
  }
  else  //D3,P2,P1�� �� ȫ����ȷ
  {
    if(((temp >> 4) + (temp >> 5) + (temp >> 7) + (temp >> 2)) & 0x01)    //D3,P2��һ����
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //D3��
      {
        temp ^= 0x80;   //��D3ȡ��
      }
      else    //P2��
      {
        temp ^= 0x04;   //��P2ȡ��
      }
    }
    else  //P1���ȫ����ȷ
    {
      if(((temp >> 4) + (temp >> 6) + (temp >> 7) + (temp >> 1)) & 0x01)    //P1��
      {
        temp ^= 0x02;   //��P1ȡ��
      }
    }
  }  
  return temp;
}


uint8_t decode16_74bit(uint16_t InData)
{
  uint8_t temp_val;
  uint8_t right_val;
  
  temp_val = (uint8_t)(InData >> 8);     //ȡ�����ֽ�
  right_val = decode74(temp_val) & 0xf0;
  temp_val = (uint8_t)InData;          //ȡ�����ֽ� 
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
  if(sbit74.bit.P1==sbit74.bit.D3^sbit74.bit.D2^sbit74.bit.D0)// D0 D2 D3        D1,P2,P3δ֪
  {
    if(sbit74.bit.P2==sbit74.bit.D3^sbit74.bit.D1^sbit74.bit.D0)//D0 D2 D3,D1,P2       P3δ֪
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0);//û�д�
      else //P3����P3ȡ����
      {
        sbit74.bit.P3=~sbit74.bit.P3;
      } 
    }
    else //D0 D2 D3,P1,P3       P2,D1�д�
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0)
      {
        sbit74.bit.P2=~sbit74.bit.P2;
      }
      else //D1��
      {
        sbit74.bit.D1=~sbit74.bit.D1;
      } 
    }
  }
  else////P2��P3,D1       �д�D0��D2��D3��P1
  {
    if(sbit74.bit.P2==sbit74.bit.D3^sbit74.bit.D1^sbit74.bit.D0)//D2��P1�д�
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0)//P1���ˣ�
      {
       sbit74.bit.P1 = ~sbit74.bit.P1;
      }
      else //D2����D2ȡ����
      {
        sbit74.bit.D2=~sbit74.bit.D2;
      } 
    }
    else //D0��D3�д�
    {
      if(sbit74.bit.P3==sbit74.bit.D2^sbit74.bit.D1^sbit74.bit.D0)//D3�д�
      {
        sbit74.bit.D3=~sbit74.bit.D3;
      }
      else //D0��
      {
        sbit74.bit.D0=~sbit74.bit.D0;
      } 
    }
  }
  return sbit74.INdata;
}
*/

/****************************************************************
**�������ܣ�����74����һ������
**��   ����
        @src Ҫ���б����Դ����ָ��
        @len Դ���ݵĳ��ȣ��ֽ�����
        @des ����������
**����ֵ:���������ݳ���
****************************************************************/
uint8_t _74CodeBytes(uint8_t *src,uint8_t *des,uint8_t len)
{
	uint8_t i = 0;

	uint16_t out_data;	//����������
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
**��   �ܣ�����74����һ������
**��   ����
        @src Ҫ���н����Դ����ָ��
        @len Դ���ݵĳ��ȣ��ֽ�����
        @des ����������
**����ֵ:���������ݳ���
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


//74(������)����������
void _74Code_Test(void)
{
	#if 0
	uint8_t i = 0;
	uint8_t temp_s[32]={"0123456789"};//Դ����
	uint8_t temp_d[32]={0}; //�����
	uint8_t temp_j[32]={0};	//�����
	uint16_t out_data;	//����������
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






