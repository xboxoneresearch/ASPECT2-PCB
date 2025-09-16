#ifndef _SLAVE_H
#define _SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define REG_MAP_SIZE      0x25

void Slave_Init(void);

/* Example accessors */
uint8_t slave_reg_read(uint8_t addr);
void slave_reg_write(uint8_t addr, uint8_t val);

uint8_t slave_is_new_segment_available(void);
uint8_t slave_hard_error(void);

#ifdef __cplusplus
}
#endif

#endif