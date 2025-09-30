#ifndef _GPIO_H
#define _GPIO_H

#define PIN_PA13    13

typedef enum {
    GPIO_MODE_INPUT     = 0x0,
    GPIO_MODE_OUTPUT    = 0x1,
    GPIO_MODE_AF        = 0x2,
    GPIO_MODE_ANALOG    = 0x3
} GPIO_Mode;

typedef enum {
    GPIO_NOPULL         = 0x0,
    GPIO_PULLUP         = 0x1,
    GPIO_PULLDOWN       = 0x2
} GPIO_Pull;

typedef enum {
    GPIO_OTYPE_PP       = 0x0,   // Push-Pull
    GPIO_OTYPE_OD       = 0x1    // Open-Drain
} GPIO_OutputType;

typedef enum {
    GPIO_SPEED_LOW      = 0x0,
    GPIO_SPEED_MEDIUM   = 0x1,
    GPIO_SPEED_HIGH     = 0x2,
    GPIO_SPEED_VHIGH    = 0x3    // very high (depends on MCU series)
} GPIO_Speed;

#endif // _GPIO_H