#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "slave.h"
#include <u8x8.h>

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

void Display_Init(void);
void Display_Print(const char *message);
void Display_ScrollText(const char *text);
void Display_ShowCode(uint16_t code, uint8_t segment);
void Display_Tick(void);

#ifdef __cplusplus
}
#endif

#endif