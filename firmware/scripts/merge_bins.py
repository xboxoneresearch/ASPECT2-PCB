#!/usr/bin/env python3
"""Merge preloader and userapp binaries and patch tombstone size/crc fields.

This script will:
- concatenate the preloader (fixed 0x800 bytes) and the userapp
- compute CRC32 and size for the preloader tombstone at 0x7E0
  (within the preloader) and for the userapp tombstone at 0x800
  (start of the userapp in the merged image)
"""

import argparse
import struct
import zlib

ALIGN_BYTES = 4
PRELOADER_SIZE = 0x800
TOMBSTONE_SIZE = 0x20
PRELOADER_TOMBSTONE_OFF = 0x7E0
UAPP_TOMBSTONE_OFF = 0x800  # offset in merged image
# entry point offset within the userapp (APP_ENTRY_POINT - APP_BASE)
UAPP_ENTRY_OFFSET = 0x200


def write_le_u16(buf: bytearray, offset: int, value: int):
    buf[offset:offset+2] = struct.pack('<H', value & 0xFFFF)


def write_le_u32(buf: bytearray, offset: int, value: int):
    buf[offset:offset+4] = struct.pack('<I', value & 0xFFFFFFFF)


def crc32_final(data: bytes) -> int:
    return zlib.crc32(data)


def main():
    p = argparse.ArgumentParser(description='Merge preloader and userapp binaries into one image')
    p.add_argument('preloader', help='preloader binary path')
    p.add_argument('userapp', help='userapp binary path')
    p.add_argument('out', help='output merged binary path')
    args = p.parse_args()

    with open(args.preloader, 'rb') as f:
        pre = f.read()
    assert len(pre) == PRELOADER_SIZE, f"Fixed preloader size mismatch (expected {PRELOADER_SIZE:#x})"

    with open(args.userapp, 'rb') as f:
        app = f.read()

    out = bytearray(pre)
    out.extend(app)

    # ensure the userapp is byte-aligned
    app_len = len(app)
    pad_len = (ALIGN_BYTES - (app_len % ALIGN_BYTES)) % ALIGN_BYTES
    if pad_len:
        out.extend(b'\xFF' * pad_len)

    # Patch preloader tombstone: write size (u16 at +0x08) and crc (u32 at +0x0A)
    # Use preloader size and compute CRC over preloader excluding the tombstone area.
    pre_crc_region = pre[:PRELOADER_TOMBSTONE_OFF]
    pre_crc = crc32_final(pre_crc_region)
    pre_size = len(pre_crc_region)  # keep fixed preloader size
    write_le_u16(out, PRELOADER_TOMBSTONE_OFF + 0x08, pre_size)
    write_le_u32(out, PRELOADER_TOMBSTONE_OFF + 0x0A, pre_crc)

    # Patch userapp tombstone: size is number of bytes from entry offset to end
    if len(app) < UAPP_ENTRY_OFFSET:
        raise SystemExit("userapp is smaller than expected entry offset; cannot compute tombstone")
    uapp_data = app[UAPP_ENTRY_OFFSET:]
    uapp_size = len(uapp_data)
    uapp_crc = crc32_final(uapp_data)
    # uapp tombstone is at offset UAPP_TOMBSTONE_OFF within merged out
    write_le_u16(out, UAPP_TOMBSTONE_OFF + 0x08, uapp_size)
    write_le_u32(out, UAPP_TOMBSTONE_OFF + 0x0A, uapp_crc)

    with open(args.out, 'wb') as f:
        f.write(out)

    print(f"Wrote {args.out}, {len(out)} bytes (preloader: {len(pre)} bytes, userapp: {len(app)} bytes, pad-len: {pad_len} bytes)")
    print(f"Preloader tombstone: size={pre_size} crc=0x{pre_crc:08X} at 0x{PRELOADER_TOMBSTONE_OFF:04X}")
    print(f"Userapp tombstone: size={uapp_size} crc=0x{uapp_crc:08X} at 0x{UAPP_TOMBSTONE_OFF:04X}")


if __name__ == '__main__':
    main()
