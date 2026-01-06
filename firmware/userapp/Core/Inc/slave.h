#ifndef _SLAVE_H
#define _SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define REG_MAP_SIZE      0x25

void Slave_Init(void);

uint8_t Slave_RegRead(uint8_t addr);
void Slave_RegWrite(uint8_t addr, uint8_t val);

uint8_t Slave_IsNewSegmentAvailable(void);
uint8_t Slave_HardError(void);

#ifdef __cplusplus
}
#endif

#endif