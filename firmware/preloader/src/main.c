#include "stm32g031xx.h"
#include "gpio.h"
#include "tombstone.h"

#define DEBOUNCE_DELAY  2000
#define CLOCK_FREQ_HSI 16000000 // 16 MHz HSI

__attribute__((section(".tombstone")))
__attribute__((used))
const tombstone_t tombstone = {
    .magic = TOMBSTONE_IAPL_MAGIC,
    .ver_major = 1,
    .ver_minor = 0,
    .size = 0xB00B,
    .crc = 0xAAAAAAAA,
    .reserved = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF },
};

/* Delay ms using SysTick */
static void delay_ms(uint32_t ms) {
    SysTick->LOAD = (CLOCK_FREQ_HSI/1000 - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    for(uint32_t i=0;i<ms;i++) {
        while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    }
    SysTick->CTRL = 0;
}

static void jump(uint32_t addr) {
    // Setup pointers to vector table start + reset handler
    uint32_t sp = *(uint32_t*)addr;
    uint32_t reset = *(uint32_t*)(addr + 4);

    // Disable all interrupts
    __disable_irq();

    // Disable Systick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Disable interrupts and clear pending ones
    for (unsigned int i = 0; i < sizeof(NVIC->ICER)/sizeof(NVIC->ICER[0]); i++) {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    // Re-enable interrupts
    __enable_irq();

    // Set vector table base offset
    SCB->VTOR = addr;

    // Switch to Main Stack Pointer (in case it was using the Process Stack Pointer)
    __set_CONTROL(0);
    
    // Instruction synchronization barrier
    __ISB();

    __set_MSP(sp);

    __DSB(); // Data synchronization barrier
    __ISB(); // Instruction synchronization barrier

    ((void(*)(void))reset)();
}

void gpio_config(GPIO_TypeDef *port, uint8_t pin,
                        GPIO_Mode mode,
                        GPIO_OutputType otype,
                        GPIO_Speed speed,
                        GPIO_Pull pull)
{
    // 1. Set mode
    port->MODER &= ~(0x3 << (pin * 2));
    port->MODER |=  (mode << (pin * 2));

    // 2. Set output type
    port->OTYPER &= ~(1 << pin);
    port->OTYPER |=  (otype << pin);

    // 3. Set speed
    port->OSPEEDR &= ~(0x3 << (pin * 2));
    port->OSPEEDR |=  (speed << (pin * 2));

    // 4. Set pull-up/pull-down
    port->PUPDR &= ~(0x3 << (pin * 2));
    port->PUPDR |=  (pull << (pin * 2));
}

int main(void) {
    /* Enable GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Configure PA13 as Input, Pull-up
    gpio_config(GPIOA, PIN_PA13,
                    GPIO_MODE_INPUT,
                    GPIO_OTYPE_PP,     // doesn’t matter for input
                    GPIO_SPEED_LOW,    // doesn’t matter for input
                    GPIO_PULLUP);

    /* 2 second wait, debounced */
    if (GPIOA->IDR & (1 << PIN_PA13)) {
        delay_ms(2000);
        if ((GPIOA->IDR & (1 << PIN_PA13)) == 0) {  // check again
            /* button pressed -> system bootloader */
            jump(SYS_MEM_BASE);
        }
    }

    // TODO: Read & verify user-app tombstone
    // if invalid -> jump to system bootloader

    /* else jump to application */
    jump(APP_BASE);

    // Should never reach here
    while(1);
}
