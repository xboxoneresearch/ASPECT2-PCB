#ifndef __POSTCODES_H
#define __POSTCODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "slave.h"

uint16_t read_postcode();
uint8_t read_segment();
const char *get_segment_name(uint8_t seg);
char get_segment_index(uint8_t seg);

#ifdef __cplusplus
}
#endif

#endif