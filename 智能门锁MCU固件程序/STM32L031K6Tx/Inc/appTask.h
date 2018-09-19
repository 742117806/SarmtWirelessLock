#ifndef __APPTASK_H
#define __APPTASK_H

#include "stm32l0xx.h"

void WirelessTask(void);
void UartTask(void);
void AES_Init(void);
void _74Code_Test(void);
void Device_MAC_Init(void);

#endif

