#include "postcodes.h"

uint16_t read_postcode() {
    return (slave_reg_read(0x20) & 0x0F)
        | ((slave_reg_read(0x21) & 0x0F) << 4)
        | ((slave_reg_read(0x22) & 0x0F) << 8)
        | ((slave_reg_read(0x23) & 0x0F) << 12);
}

uint8_t read_segment() {
    return slave_reg_read(0x24);
}

const char *get_segment_name(uint8_t seg) {
  switch (seg & 0xF0) {
    case 0x70:
      return "OS";
    case 0x30:
      return "SP";
    case 0x10:
      return "SMC";
    default:
      return "???";
  }
}

char get_segment_index(uint8_t seg) {
    return seg & 0x0F;
}