#include "postcodes.h"

uint16_t read_postcode() {
    return (slave_reg_read(REG_Digit0) & 0x0F)
        | ((slave_reg_read(REG_Digit1) & 0x0F) << 4)
        | ((slave_reg_read(REG_Digit2) & 0x0F) << 8)
        | ((slave_reg_read(REG_Digit3) & 0x0F) << 12);
}

uint8_t read_segment() {
    return slave_reg_read(REG_Segments);
}

const char *get_segment_name(uint8_t seg) {
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

char get_segment_index(uint8_t seg) {
    return seg & 0x0F;
}