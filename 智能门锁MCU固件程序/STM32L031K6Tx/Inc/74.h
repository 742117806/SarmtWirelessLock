#ifndef __CECODEC74_H
#define __CECODEC74_H
#include "stdint.h"
#include "device.h"

uint16_t code16_74bit(uint8_t InData);
uint8_t decode16_74bit(uint16_t InData);
uint8_t _74CodeBytes(uint8_t *src,uint8_t *des,uint8_t len);
uint8_t _74DecodeBytes(uint8_t *src,uint8_t *des,uint8_t len);
void FrameData_74Convert(FRAME_CMD_t *srcData,uint8_t srcLen,uint8_t *outLen,uint8_t mode);

#endif
