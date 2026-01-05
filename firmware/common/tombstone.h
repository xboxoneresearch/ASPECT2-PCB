#ifndef _TOMBSTONE_H
#define _TOMBSTONE_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define PRELOADER_BASE (FLASH_BASE)
#define PRELOADER_SIZE (2 * 1024) // 2KB
#define APP_BASE       (PRELOADER_BASE + PRELOADER_SIZE)
#define APP_ENTRY_POINT (0x08000A00U)
#define SYSTEM_BOOTLOADER_OFFSET   (0x1FFF0000U)

#define TOMBSTONE_SIZE (0x20)
#define IAPL_MAGIC {'I', 'A', 'P', 'L'}
#define UAPP_MAGIC {'U', 'A', 'P', 'P'}

static const uint8_t TOMBSTONE_IAPL_MAGIC[4] = IAPL_MAGIC;
static const uint8_t TOMBSTONE_UAPP_MAGIC[4] = UAPP_MAGIC;

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

uint32_t calculate_crc32(uint8_t *data, uint32_t len);
void resetToBootLoader();
void handleBootLoader();
void jump(uint32_t addr);
bool validate_tombstone(uint32_t tombstone_address, uint32_t data_offset, const uint8_t magic[4]);
bool validate_iapl_tombstone();
bool validate_uapp_tombstone();

#endif // _TOMBSTONE_H