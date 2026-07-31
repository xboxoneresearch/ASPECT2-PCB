#ifndef _SLAVE_H
#define _SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define REG_MAP_SIZE      0x25

// I2C1 main slave address (unshifted)
#define MAX6958_SLAVE_I2C_ADDR 0x38
// Secondary I2C1 own-address (7-bit, unshifted): writing BOOTLOADER_TRIGGER_MAGIC
// here reboots straight into the STM32 system bootloader (DFU), letting the
// board be reflashed without physical button/SWD access.
#define BOOTLOADER_TRIGGER_I2C_ADDR   0x39
// Magic bytes, MSB first: 0xB0 0x07 0xB0 0x07 ("B007 B007" / "BOOT BOOT")
#define BOOTLOADER_TRIGGER_MAGIC      0xB007B007UL

void Slave_Init(void);
void Slave_Poll(void);

uint8_t Slave_RegRead(uint8_t addr);
void Slave_RegWrite(uint8_t addr, uint8_t val);

uint8_t Slave_IsNewSegmentAvailable(void);
uint8_t Slave_HardError(void);

#ifdef __cplusplus
}
#endif

#endif