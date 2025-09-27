#ifndef __JMP_BL_H
#define __JMP_BL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define BOOTLOADER_ADDR 0x1FFF0000 // Bootloader start address (refer to AN2606). STM32 family-dependent.

struct bootloader_vectable__t {
    uint32_t stack_pointer;
    void (*reset_handler)(void);
};
#define BOOTLOADER_VECTOR_TABLE	((struct bootloader_vectable__t *)BOOTLOADER_ADDR)

void JumpToBootloader(void);

#ifdef __cplusplus
}
#endif

#endif