#include <stdbool.h>
#include "stm32g031xx.h"
#include "gpio.h"
#include "bootloader.h"

#define DEBOUNCE_DELAY  200
#define CLOCK_FREQ_HSI 16000000 // 16 MHz HSI

/* size and crc are being replaced by scripts/fix_size_crc.py */
__attribute__((section(".tombstone_iapl")))
__attribute__((used))
const tombstone_t tombstone = {
    .magic = IAPL_MAGIC,
    .ver_major = 1,
    .ver_minor = 1,
    .size = 0x5A53, // ASCII: SZ
    .crc = 0x00435243, // ASCII: CRC + \0
    .reserved = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF },
};

/* Delay ms using SysTick */
static void delayMs(uint32_t ms) {
    SysTick->LOAD = (CLOCK_FREQ_HSI/1000 - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    for(uint32_t i=0;i<ms;i++) {
        while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    }
    SysTick->CTRL = 0;
}

void gpioConfig(GPIO_TypeDef *port, uint8_t pin,
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

    handleSystemBootLoader();

    // Enable GPIOA
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Enable CRC clock
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
    

    // Configure PA13 as Input, Pull-up
    gpioConfig(GPIOA, PIN_13,
                    GPIO_MODE_INPUT,
                    GPIO_OTYPE_PP,     // doesn’t matter for input
                    GPIO_SPEED_LOW,    // doesn’t matter for input
                    GPIO_PULLUP);


    // 2 second wait, debounced 
    if (!(GPIOA->IDR & (1 << PIN_13))) {
        delayMs(DEBOUNCE_DELAY);
        if (!(GPIOA->IDR & (1 << PIN_13))) {  // check again
            // button pressed -> system bootloader
            resetToSystemBootLoader();
        }
    }

    bool uapp_valid = validateUappTombstone();
    if (uapp_valid) {
        /* jump to application */
        jump(APP_ENTRY_POINT);
    }

    // As last resort, drop to system bootloader
    resetToSystemBootLoader();
    return 0;
}
