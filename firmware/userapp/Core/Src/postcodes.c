#include "postcodes.h"

uint16_t POST_ReadCode() {
    return (Slave_RegRead(REG_Digit0) & 0x0F)
        | ((Slave_RegRead(REG_Digit1) & 0x0F) << 4)
        | ((Slave_RegRead(REG_Digit2) & 0x0F) << 8)
        | ((Slave_RegRead(REG_Digit3) & 0x0F) << 12);
}

uint8_t POST_ReadSegment() {
    return Slave_RegRead(REG_Segments);
}

const char *POST_GetSegmentName(uint8_t seg) {
    switch (seg & 0xF0) {
        case CODE_FLAVOR_CPU:
            return "CPU";
        case CODE_FLAVOR_SP:
            return "SP ";
        case CODE_FLAVOR_SMC:
            return "SMC";
        case CODE_FLAVOR_OS:
            return "OS ";
        default:
            return "???";
    }
}

char POST_GetSegmentIndex(uint8_t seg) {
    return seg & 0x0F;
}