#ifndef __POSTCODES_H
#define __POSTCODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "slave.h"

#define SEGMENT_FLAVOR_MASK 0xF0
#define SEGMENT_INDEX_MASK  0x0F

// Indicated by Segments-register
enum CodeFlavor: uint8_t {
    CODE_FLAVOR_CPU = 0x10,
    CODE_FLAVOR_SP =  0x30,
    CODE_FLAVOR_SMC = 0x70,
    CODE_FLAVOR_OS =  0xF0,
};

typedef struct {
    uint64_t code;
    uint8_t flavor;
} PostCode;

enum MAX6958Registers {
    REG_NoOp = 0x00,
    REG_DecodeMode = 0x01,
    REG_Intensity = 0x02,
    REG_ScanLimit = 0x03,
    REG_Configuration = 0x04,
    REG_FactoryReserved = 0x05,
    REG_GpIo = 0x06,
    REG_DisplayTest = 0x07,
    REG_ReadKeyDebounced = 0x08,
    REG_ReadKeyPressed = 0x0C,
    REG_Digit0 = 0x20,
    REG_Digit1 = 0x21,
    REG_Digit2 = 0x22,
    REG_Digit3 = 0x23,
    REG_Segments = 0x24,
};

const char *POST_GetSegmentName(uint8_t flavor);

// Feeds the current Digit0-3/Segments registers into the code assembler.
// A full POST code is made up of up to 4 consecutive Segments-writes of the
// same flavor (MSB-first, terminated by index 1); returns 1 and fills *out
// once a full code has been assembled, 0 otherwise.
uint8_t POST_ProcessSegment(PostCode *out);

#ifdef __cplusplus
}
#endif

#endif