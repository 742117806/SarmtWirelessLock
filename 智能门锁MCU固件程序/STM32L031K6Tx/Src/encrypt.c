#include "encrypt.h"
#include "uart.h"
#include "crc16.h"
#include "aes.h"
#include "protocol.h"

uint8_t const Mi_Yue_Tab[RsaByte_Size] = {
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X33, 0X99}; //02 7A D2 B3 82 97 C6 A6 86 14 A6 67 CB 74 CD 21 //RSA密钥直接存于程序的常量中

uint8_t const Mo_Zhi_Tab[RsaByte_Size] = {
    0XA4, 0XD9, 0XB2, 0XE8, 0X34, 0X7F, 0XBE, 0XF7, 0X14, 0XFA, 0X74, 0X32, 0XA3, 0X57, 0XC6, 0XAD}; //62 DA CF C7 85 2E E8 B1 A3 D4 0E 22 D1 A1 8D 17 //RSA模值直接存于程序的常量中

/*---------------------------------------------------------------------------------------
 功能: 利用aes_w的密文内容进行数据加密和解密
 参数: 
 @p_source 加密源数据，
 @input_len 源数据长度，
 @output_len 加密后数据长度  
 @Convert_Mode 加密模式：1加密，0解密
----------------------------------------------------------------------------------------*/
uint8_t Encrypt_Convert(uint8_t *p_source, uint8_t input_len, uint8_t *output_len, uint8_t Convert_Mode)
{

    uint8_t rout_region[3 + RoutSeries_Size];

    uint16_t crc16_val;
    uint8_t i, temp_len;
    uint8_t Encode_Round;
    uint8_t Encode_Mantissa;
    uint8_t Encode_Len = p_source[Region_DataLenNumber];

    Encode_Round = Encode_Len / 16;
    Encode_Mantissa = Encode_Len % 16;


    if ((p_source[Region_SeqNumber] & 0x80) && Convert_Mode) //有路由标志且要加密
    {
        //memmove(rout_region, &p_source[Region_DataAFNNumber + p_source[Region_DataLenNumber]], 3+RoutSeries_Size);
        memmove(rout_region, &p_source[input_len - (3 + 3 + RoutSeries_Size)], 3 + RoutSeries_Size);
    }

    for (i = 0; i < Encode_Round; i++)
    {
#if (Encrypt_En == 2)
        if (Encrypt_OnceConvert(&p_source[Region_DataAFNNumber + 16 * i], Convert_Mode))
            return 1;
#elif (Encrypt_En == 1)
        if (Convert_Mode)
            cipher(&p_source[Region_DataAFNNumber + 16 * i], &p_source[Region_DataAFNNumber + 16 * i], aes_w);
        else
            inv_cipher(&p_source[Region_DataAFNNumber + 16 * i], &p_source[Region_DataAFNNumber + 16 * i], aes_w);
#endif
    }
    if (Encode_Mantissa)
    {
        if (Convert_Mode)
        {
            for (i = 0; i < (16 - Encode_Mantissa); i++)
            {
                p_source[Region_DataAFNNumber + 16 * Encode_Round + Encode_Mantissa + i] = 0;
            }
#if (Encrypt_En == 2)
            if (Encrypt_OnceConvert(&p_source[Region_DataAFNNumber + 16 * Encode_Round], Convert_Mode))
                return 1;
#elif (Encrypt_En == 1)
            cipher(&p_source[Region_DataAFNNumber + 16 * Encode_Round], &p_source[Region_DataAFNNumber + 16 * Encode_Round], aes_w);
#endif
            temp_len = input_len + 16 - Encode_Mantissa;
        }
        else
        {
#if (Encrypt_En == 2)
            if (Encrypt_OnceConvert(&p_source[Region_DataAFNNumber + 16 * Encode_Round], Convert_Mode))
                return 1;
#elif (Encrypt_En == 1)
            inv_cipher(&p_source[Region_DataAFNNumber + 16 * Encode_Round], &p_source[Region_DataAFNNumber + 16 * Encode_Round], aes_w);
#endif
            temp_len = input_len - 16 + Encode_Mantissa;
        }
    }
    else
        temp_len = input_len;

    if ((p_source[Region_SeqNumber] & 0x80) && Convert_Mode)
    {
        memmove(&p_source[Region_DataAFNNumber + p_source[Region_DataLenNumber]], rout_region, 3 + RoutSeries_Size);
        temp_len += (3 + RoutSeries_Size);
    }

    crc16_val = CRC16_2(p_source, temp_len - 3);
    p_source[temp_len - 3] = crc16_val >> 8;
    p_source[temp_len - 2] = crc16_val & 0xff;
    p_source[temp_len - 1] = HKFreamEnd;

    *output_len = temp_len;
    return 0;
}

//解出AES密钥
//@p_minw 密钥存储地址
void Rsa_Decode(uint8_t *p_minw)
{
    uint8_t *Mi_Yue; //02 7A D2 B3 82 97 C6 A6 86 14 A6 67 CB 74 CD 21
    uint8_t *Mo_Zhi; //62 DA CF C7 85 2E E8 B1 A3 D4 0E 22 D1 A1 8D 17
    uint8_t *Mi_Wen; //30 34 47 F9 6D 76 75 A7 8F 47 26 8E 70 4C BF 21
    uint8_t *RSA_TempBuf;

    Mi_Yue = aes_w;
    Mo_Zhi = &aes_w[RsaByte_Size];
    Mi_Wen = &aes_w[RsaByte_Size + RsaByte_SizeMax];
    RSA_TempBuf = &aes_w[RsaByte_Size * 2 + RsaByte_SizeMax + 3];

    memset(Mi_Wen, 0, RsaByte_Size);
    memset(Mi_Yue, 0, RsaByte_Size);
    memset(Mo_Zhi, 0, RsaByte_SizeMax);
    memset(RSA_TempBuf, 0, RsaByte_SizeMax);

    memmove(Mi_Yue, Mi_Yue_Tab, RsaByte_Size);
    memmove(&Mo_Zhi[1], Mo_Zhi_Tab, RsaByte_Size);

    //Eeprom_ByteRead(Mi_Wen_EAddr, Mi_Wen, RsaByte_Size+3);
    memmove(Mi_Wen, &aes_out[RsaByte_Size * 2], RsaByte_Size + 3);

    LargeNumber_Power_Mod(p_minw, Mi_Wen, Mi_Yue, RSA_TempBuf, Mo_Zhi);

    p_minw[3] = Mi_Wen[RsaByte_Size + 0];
    p_minw[7] = Mi_Wen[RsaByte_Size + 1];
    p_minw[11] = Mi_Wen[RsaByte_Size + 2];
}



/*
加解密测试
*/
void EncryptTest(void)
{
    uint8_t out_len;
    uint8_t frame_len;
    uint8_t test[64] = {0, 0, 0, 0, 0, 0, 0, 7, 0, 1, 2, 3, 4, 5, 6};

    frame_len = Frame_Compose(test);
    Encrypt_Convert(test, frame_len, &out_len, 1);
    //HAL_UART_Transmit(&hlpuart1, test, out_len, 200);
    //Rsa_Decode(test);
    Encrypt_Convert(test, out_len, &frame_len, 0);
}
