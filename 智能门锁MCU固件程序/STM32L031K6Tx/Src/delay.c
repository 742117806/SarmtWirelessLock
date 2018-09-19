#include "delay.h"
//#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////

static uint32_t fac_us = 0; //us��ʱ������

//��ʼ���ӳٺ���
//��ʹ��ucos��ʱ��,�˺������ʼ��ucos��ʱ�ӽ���
//SYSTICK��ʱ�ӹ̶�ΪAHBʱ��
//SYSCLK:ϵͳʱ��Ƶ��
void delay_init(uint8_t SYSCLK)
{

    //HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTickƵ��ΪHCLK
    fac_us = SYSCLK; //�����Ƿ�ʹ��OS,fac_us����Ҫʹ��
}

//��ʱnus
//nusΪҪ��ʱ��us��.
//nus:0~190887435(���ֵ��2^32/fac_us@fac_us=22.5)
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD; //LOAD��ֵ
    ticks = nus * fac_us;            //��Ҫ�Ľ�����
    told = SysTick->VAL;             //�ս���ʱ�ļ�����ֵ
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow; //����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break; //ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
        }
    };
}

//��ʱnms
//nms:Ҫ��ʱ��ms��
void delay_ms(uint16_t nms)
{
    uint32_t i;
    for (i = 0; i < nms; i++)
        delay_us(1000);
}
