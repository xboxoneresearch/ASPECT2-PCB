#include "jmp_bl.h"

void JumpToBootloader(void) {
    // Deinit HAL and Clocks
    HAL_DeInit();
    HAL_RCC_DeInit();
    
    // Disable all interrupts
    __disable_irq();

    // Disable Systick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Disable interrupts and clear pending ones
    for (size_t i = 0; i < sizeof(NVIC->ICER)/sizeof(NVIC->ICER[0]); i++) {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    // Re-enable interrupts
    __enable_irq();

    // Map Bootloader (system flash) memory to 0x00000000. This is STM32 family dependant.
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
    
    // Set embedded bootloader vector table base offset
    WRITE_REG(SCB->VTOR, SCB_VTOR_TBLOFF_Msk & 0x00000000);

    // Switch to Main Stack Pointer (in case it was using the Process Stack Pointer)
    __set_CONTROL(0);
    
    // Instruction synchronization barrier
    __ISB();

    // Set Main Stack Pointer to the Bootloader defined value.
    __set_MSP(BOOTLOADER_VECTOR_TABLE->stack_pointer);

    __DSB(); // Data synchronization barrier
    __ISB(); // Instruction synchronization barrier

    // Jump to Bootloader Reset Handler
    BOOTLOADER_VECTOR_TABLE->reset_handler();
    
    // The next instructions will not be reached
    while (1){}
}