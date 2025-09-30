#ifndef _TOMBSTONE_H
#define _TOMBSTONE_H

#include <assert.h>
#include <stdint.h>

#define PRELOADER_BASE (FLASH_BASE)
#define PRELOADER_SIZE (2 * 1024) // 2KB
#define APP_BASE       (PRELOADER_BASE + PRELOADER_SIZE)
#define SYS_MEM_BASE   (0x1FFF0000)

#define TOMBSTONE_SIZE (0x20)
#define TOMBSTONE_IAPL_MAGIC {'I', 'A', 'P', 'L'}
#define TOMBSTONE_UAPP_MAGIC {'U', 'A', 'P', 'P'}

/* Tombstone */
typedef struct {
    /* 0x00 */ uint8_t magic[4];
    /* 0x04 */ uint16_t ver_major;
    /* 0x06 */ uint16_t ver_minor;
    /* 0x08 */ uint16_t size;
    /* 0x0A */ uint32_t crc;
    /* 0x0E */ uint8_t  reserved[0x12];
    /* 0x20 total */
}
__attribute__((packed, aligned(1)))
tombstone_t;

static_assert (TOMBSTONE_SIZE == (sizeof(tombstone_t)),
    "TOMBSTONE vs sizeof(tombstone_t) mismatch");

#endif // _TOMBSTONE_H