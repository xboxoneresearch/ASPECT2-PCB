#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ssd1306.h>
#include <ssd1306_fonts.h>
#include "ssd1306_conf.h"
#include "main.h"

void Display_Init(void);
void Display_Print(char *message);
void Display_Tick(void);

#ifdef __cplusplus
}
#endif

#endif