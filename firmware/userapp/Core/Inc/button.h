#ifndef __BUTTON_H
#define __BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern bool button_pressed;

bool Button_IsPressed(uint32_t *duration_ms);
void Button_Tick();

#ifdef __cplusplus
}
#endif

#endif